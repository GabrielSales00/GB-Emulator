#include <stdio.h>
#include <include/gb_cpu.h>


int main(int argc, char ** argv) {
    unsigned char bootrom = load_bootrom("src/bootrom/dmg_boot.bin");

    // the cpu is to read the byte at the memory address PC points to
    CPU * gba_cpu = newCPU();
    run_instructions(gba_cpu, &bootrom);
        
    return 0;
}