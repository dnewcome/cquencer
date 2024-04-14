#include "clock.c"

int main(int argc, const char* argv[]) {
    init_array();
    init_midi();
    clk_main();
    return 0;
}   