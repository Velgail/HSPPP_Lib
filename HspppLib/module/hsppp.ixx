// hsppp.ixx : Main module interface

export module hsppp;

export void screen(int mode, int width, int height, const char* title);

export namespace hsppp {
namespace internal {
void init_system();
void close_system();
}
}