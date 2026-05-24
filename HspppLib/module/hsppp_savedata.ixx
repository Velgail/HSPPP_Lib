// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/module/hsppp_savedata.ixx
// ═══════════════════════════════════════════════════════════════════
// HSPPP セーブデータ - モジュールインターフェース
// ═══════════════════════════════════════════════════════════════════
//
// バイナリ独自形式のセーブ／ロード API。
//
// バイト構造（version 1, little-endian, x64 前提）:
//   [0..3]   magic         "HSPP"
//   [4..7]   uint32        version (= kSaveFormatVersion)
//   [8..11]  uint32        block_count
//   [12..]   blocks*
//     block:
//       uint16  key_length
//       bytes   key                (UTF-8)
//       uint16  type_tag_length
//       bytes   type_tag           (UTF-8, L::type_tag())
//       uint32  payload_length
//       bytes   payload
//
// version mismatch・type_tag mismatch・key 不在は HspError もしくは std::out_of_range。
//
// 設計根拠:
//   .github/agents/sprints/current/artifacts/design-TICKET-002.md
//   §7.3, §8.3, §12.3, §13(L5), §15(F5, C6), §17(Risk-5)

export module hsppp:savedata;

import :interrupt;   // HspError / ERR_*

import <array>;
import <concepts>;
import <cstddef>;
import <cstdint>;
import <cstring>;
import <format>;
import <optional>;
import <span>;
import <stdexcept>;
import <string>;
import <string_view>;
import <type_traits>;
import <unordered_map>;
import <utility>;
import <vector>;

export namespace hsppp {

// ═══════════════════════════════════════════════════════════════════
// バージョン定数（design §7.3 / §15 C6: 本 Sprint 中は固定）
// ═══════════════════════════════════════════════════════════════════

inline constexpr std::uint32_t kSaveFormatVersion = 1;
inline constexpr std::array<char, 4> kSaveMagic = { 'H', 'S', 'P', 'P' };

// ═══════════════════════════════════════════════════════════════════
// Serializable concept（design §7.3 / §18 Q-6）
// ═══════════════════════════════════════════════════════════════════

/// @brief セーブ／ロード可能型の要件
///
/// 対象型 L に以下の static メンバを要求する:
///   - static void   L::serialize(const L& v, std::vector<std::byte>& out)
///   - static std::size_t L::deserialize(std::span<const std::byte> in, L& dst)
///   - static std::string_view L::type_tag()  // 型タグ（version 別比較用）
///
/// design §18 Q-6 の判断: type_tag は string_view 固定（uint32 ハッシュは採用しない）。
template <typename L>
concept Serializable = requires(const L& v,
                                std::vector<std::byte>& out,
                                std::span<const std::byte> in,
                                L& dst)
{
    { L::serialize(v, out) } -> std::same_as<void>;
    { L::deserialize(in, dst) } -> std::same_as<std::size_t>;
    { L::type_tag() } -> std::convertible_to<std::string_view>;
};

// ═══════════════════════════════════════════════════════════════════
// SaveWriter（design §7.3）
// ═══════════════════════════════════════════════════════════════════

/// @brief セーブデータ書き出し器
///
/// finalize() でマジック / version / block 群を含むバイト列を返す。
/// 同一 key に対し write() を複数回呼ぶと最後の値で上書きされる。
class SaveWriter {
public:
    SaveWriter() = default;

    SaveWriter(const SaveWriter&) = delete;
    SaveWriter& operator=(const SaveWriter&) = delete;
    SaveWriter(SaveWriter&&) noexcept = default;
    SaveWriter& operator=(SaveWriter&&) noexcept = default;

    /// @brief 型安全な書き込み API（利用者向け）
    template <typename L>
        requires Serializable<L>
    void write(std::string_view key, const L& value)
    {
        std::vector<std::byte> payload;
        L::serialize(value, payload);
        write_raw(key, L::type_tag(), payload);
    }

    /// @brief 型消去された書き込み API（StateScope 内部用）
    /// @note key / type_tag は UTF-8 を仮定。length は uint16 にキャストできる範囲を要求。
    void write_raw(std::string_view key,
                   std::string_view type_tag,
                   std::span<const std::byte> payload)
    {
        if (key.size() > 0xFFFFu) {
            throw HspError(ERR_OUT_OF_RANGE,
                           std::format("SaveWriter: key length {} exceeds 65535", key.size()));
        }
        if (type_tag.size() > 0xFFFFu) {
            throw HspError(ERR_OUT_OF_RANGE,
                           std::format("SaveWriter: type_tag length {} exceeds 65535", type_tag.size()));
        }
        if (payload.size() > 0xFFFFFFFFu) {
            throw HspError(ERR_OUT_OF_RANGE,
                           std::format("SaveWriter: payload length {} exceeds 4GiB", payload.size()));
        }

        Block block;
        block.key = std::string(key);
        block.type_tag = std::string(type_tag);
        block.payload.assign(payload.begin(), payload.end());

        // 同一 key は上書き（後勝ち）
        if (auto it = key_index_.find(block.key); it != key_index_.end()) {
            blocks_[it->second] = std::move(block);
        }
        else {
            key_index_.emplace(block.key, blocks_.size());
            blocks_.push_back(std::move(block));
        }
    }

    /// @brief マジック + version + 全 block を 1 つのバイト列に出力
    [[nodiscard]] std::vector<std::byte> finalize() const
    {
        std::vector<std::byte> out;
        // header の概算予約（最低 12 バイト）
        out.reserve(12);

        // magic
        for (char c : kSaveMagic) {
            out.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
        }
        // version
        write_u32(out, kSaveFormatVersion);
        // block_count
        write_u32(out, static_cast<std::uint32_t>(blocks_.size()));

        for (const auto& b : blocks_) {
            write_u16(out, static_cast<std::uint16_t>(b.key.size()));
            write_bytes(out, std::string_view(b.key));
            write_u16(out, static_cast<std::uint16_t>(b.type_tag.size()));
            write_bytes(out, std::string_view(b.type_tag));
            write_u32(out, static_cast<std::uint32_t>(b.payload.size()));
            out.insert(out.end(), b.payload.begin(), b.payload.end());
        }

        return out;
    }

    /// @brief 現在保持しているブロック数
    [[nodiscard]] std::size_t block_count() const noexcept { return blocks_.size(); }

private:
    struct Block {
        std::string              key;
        std::string              type_tag;
        std::vector<std::byte>   payload;
    };

    static void write_u16(std::vector<std::byte>& out, std::uint16_t v)
    {
        out.push_back(static_cast<std::byte>(v & 0xFFu));
        out.push_back(static_cast<std::byte>((v >> 8) & 0xFFu));
    }

    static void write_u32(std::vector<std::byte>& out, std::uint32_t v)
    {
        out.push_back(static_cast<std::byte>(v & 0xFFu));
        out.push_back(static_cast<std::byte>((v >> 8) & 0xFFu));
        out.push_back(static_cast<std::byte>((v >> 16) & 0xFFu));
        out.push_back(static_cast<std::byte>((v >> 24) & 0xFFu));
    }

    static void write_bytes(std::vector<std::byte>& out, std::string_view sv)
    {
        for (char c : sv) {
            out.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
        }
    }

    std::vector<Block>                          blocks_;
    std::unordered_map<std::string, std::size_t> key_index_;
};

// ═══════════════════════════════════════════════════════════════════
// SaveReader（design §7.3）
// ═══════════════════════════════════════════════════════════════════

/// @brief セーブデータ読み出し器
///
/// コンストラクタでヘッダ検証（magic / version / block 解析）を行い、
/// 失敗時には HspError を throw する。read<L>() は型 tag 不一致時にも
/// HspError、key 不在時には std::out_of_range を投げる。
class SaveReader {
public:
    /// @brief セーブデータをパース
    /// @throws HspError magic / version / 構造不正
    explicit SaveReader(std::span<const std::byte> data)
        : data_(data.begin(), data.end())
    {
        parse_header_();
    }

    SaveReader(const SaveReader&) = delete;
    SaveReader& operator=(const SaveReader&) = delete;
    SaveReader(SaveReader&&) noexcept = default;
    SaveReader& operator=(SaveReader&&) noexcept = default;

    [[nodiscard]] std::uint32_t version() const noexcept { return version_; }

    [[nodiscard]] bool has(std::string_view key) const noexcept
    {
        return blocks_.find(std::string(key)) != blocks_.end();
    }

    /// @brief 型安全な読み出し API
    /// @throws HspError type_tag mismatch
    /// @throws std::out_of_range key 不在
    template <typename L>
        requires Serializable<L>
    [[nodiscard]] L read(std::string_view key) const
    {
        auto it = blocks_.find(std::string(key));
        if (it == blocks_.end()) {
            throw std::out_of_range(std::format("SaveReader: key '{}' not found", key));
        }
        const auto& blk = it->second;
        if (blk.type_tag != std::string_view(L::type_tag())) {
            throw HspError(ERR_TYPE_MISMATCH,
                           std::format("SaveReader: type_tag mismatch for key '{}': stored='{}', expected='{}'",
                                       key, blk.type_tag, L::type_tag()));
        }
        L dst{};
        std::span<const std::byte> payload(data_.data() + blk.payload_offset, blk.payload_length);
        (void)L::deserialize(payload, dst);
        return dst;
    }

    /// @brief 型消去された参照取得（StateScope 内部用）
    struct RawBlock {
        std::string_view              type_tag;
        std::span<const std::byte>    payload;
    };

    [[nodiscard]] std::optional<RawBlock> find_raw(std::string_view key) const noexcept
    {
        auto it = blocks_.find(std::string(key));
        if (it == blocks_.end()) return std::nullopt;
        const auto& blk = it->second;
        return RawBlock{
            std::string_view(blk.type_tag),
            std::span<const std::byte>(data_.data() + blk.payload_offset, blk.payload_length)
        };
    }

    [[nodiscard]] std::size_t block_count() const noexcept { return blocks_.size(); }

private:
    struct ParsedBlock {
        std::string   type_tag;
        std::size_t   payload_offset;
        std::size_t   payload_length;
    };

    void require_(std::size_t available, std::size_t need, std::string_view what) const
    {
        if (available < need) {
            throw HspError(ERR_OUT_OF_RANGE,
                           std::format("SaveReader: truncated data while reading {}", what));
        }
    }

    static std::uint16_t read_u16_(const std::byte* p)
    {
        return static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(std::to_integer<unsigned char>(p[0])) |
            (static_cast<std::uint16_t>(std::to_integer<unsigned char>(p[1])) << 8));
    }

    static std::uint32_t read_u32_(const std::byte* p)
    {
        return  static_cast<std::uint32_t>(std::to_integer<unsigned char>(p[0])) |
               (static_cast<std::uint32_t>(std::to_integer<unsigned char>(p[1])) << 8)  |
               (static_cast<std::uint32_t>(std::to_integer<unsigned char>(p[2])) << 16) |
               (static_cast<std::uint32_t>(std::to_integer<unsigned char>(p[3])) << 24);
    }

    void parse_header_()
    {
        const std::size_t total = data_.size();
        require_(total, 12, "header");

        // magic
        for (std::size_t i = 0; i < kSaveMagic.size(); ++i) {
            if (std::to_integer<unsigned char>(data_[i]) !=
                static_cast<unsigned char>(kSaveMagic[i]))
            {
                throw HspError(ERR_TYPE_MISMATCH,
                               std::format("SaveReader: bad magic at byte {}", i));
            }
        }
        version_ = read_u32_(data_.data() + 4);
        if (version_ != kSaveFormatVersion) {
            throw HspError(ERR_UNSUPPORTED,
                           std::format("SaveReader: unsupported save format version {} (expected {})",
                                       version_, kSaveFormatVersion));
        }
        const std::uint32_t block_count = read_u32_(data_.data() + 8);

        std::size_t cur = 12;
        for (std::uint32_t i = 0; i < block_count; ++i) {
            require_(total - cur, 2, "block.key_length");
            const std::uint16_t key_len = read_u16_(data_.data() + cur);
            cur += 2;
            require_(total - cur, key_len, "block.key");
            std::string key(reinterpret_cast<const char*>(data_.data() + cur), key_len);
            cur += key_len;

            require_(total - cur, 2, "block.type_tag_length");
            const std::uint16_t tag_len = read_u16_(data_.data() + cur);
            cur += 2;
            require_(total - cur, tag_len, "block.type_tag");
            std::string tag(reinterpret_cast<const char*>(data_.data() + cur), tag_len);
            cur += tag_len;

            require_(total - cur, 4, "block.payload_length");
            const std::uint32_t payload_len = read_u32_(data_.data() + cur);
            cur += 4;
            require_(total - cur, payload_len, "block.payload");

            ParsedBlock pb{ std::move(tag), cur, payload_len };
            cur += payload_len;

            // 重複 key は後勝ち（SaveWriter と整合）
            blocks_.insert_or_assign(std::move(key), std::move(pb));
        }

        if (cur != total) {
            // 余剰バイトは厳格に拒否（design §7.3 「fail-fast」精神）
            throw HspError(ERR_OUT_OF_RANGE,
                           std::format("SaveReader: trailing {} unread bytes after blocks",
                                       total - cur));
        }
    }

    std::vector<std::byte>                       data_;
    std::uint32_t                                version_ = 0;
    std::unordered_map<std::string, ParsedBlock> blocks_;
};

}  // namespace hsppp
