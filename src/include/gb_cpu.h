#ifndef CPU_H

#define CPU_H

#define GB_BOOTROM_SIZE 0x100
#define SIZE_MEM 1000

#include <stdint.h>
#include <stdbool.h>

typedef struct Register{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint16_t pc;
    uint16_t sp;  
    uint16_t hl;
} Register;

typedef struct ArithmeticFlags{
    bool zero;
    bool h_carry;
    bool carry;
    bool subtraction;
} ArithmeticFlags;

typedef struct StateFlags {
    bool ime;
    bool ie;
    bool enable_interrupts;
    bool iflags;
    bool halted;
    bool stopped;
    bool halt_bug;
} StateFlags;

typedef struct CPU {
    int clock_speed;
    Register reg;
    ArithmeticFlags flags;
    StateFlags state;
    memory[SIZE_MEM]
} CPU;

CPU * newCPU();

unsigned char load_bootrom(char * file_path);

int run_cpu(CPU * CPU, unsigned char * memory);


/*
    MEMORY operations
*/

uint8_t cpu_read8(CPU * cpu, uint16_t addr);

void cpu_write8(CPU * cpu, uint16_t addr, uint8_t value);


/*
    Instruction set (ISA)
*/

void NOP(CPU * cpu);

void INC_8(CPU * cpu, uint8_t * reg);

void INC_16(CPU * cpu, uint16_t * reg);

void DEC_8(CPU * cpu, uint8_t * reg);

void INC_mem(CPU *cpu, uint16_t addr);

void DEC_mem(CPU *cpu, uint16_t addr);

void RLCA(CPU * cpu);

void RRCA(CPU * cpu);

void RLA(CPU * cpu);

void RRA(CPU * cpu);

void DAA(CPU * cpu);

void CPL(CPU * cpu);

void SCF(CPU * cpu);

void CCF(CPU * cpu);

void HALT(CPU * cpu);

void ADD_A(CPU * cpu, uint8_t value);

void ADC_A(CPU * cpu, uint8_t value);

void SUB_A(CPU * cpu, uint8_t value);

void SBC_A(CPU * cpu, uint8_t value);

void AND_A(CPU * cpu, uint8_t value);

void XOR_A(CPU * cpu, uint8_t value);

void OR_A(CPU * cpu, uint8_t value);

void CP_A(CPU * cpu, uint8_t value);

void RET(CPU *cpu);

void RET_Z(CPU *cpu);

void RET_NZ(CPU *cpu);

void RET_C(CPU *cpu);

void RET_NC(CPU *cpu);

void POP_16(CPU * cpu, uint16_t * reg);

void JP(CPU * cpu, uint16_t addr);

void CALL(CPU * cpu, uint16_t addr);

void PUSH_16(CPU * cpu, uint16_t * reg);

void ADD_HL(CPU * cpu, uint16_t value);

void RST_8(CPU * cpu, uint8_t value);

void PREFIX_CB(CPU * cpu);

void LDH_mem_A(CPU *cpu, uint8_t offset);

void LDH_A_mem(CPU *cpu, uint8_t offset);

void LD_8(uint8_t * buffer, uint8_t value); //REGISTER assignment, not memory (use cpu_write)

void JR_e8(CPU * cpu, int8_t offset);

void JR_Z_e8(CPU *cpu, int8_t offset);

void JR_NZ_e8(CPU *cpu, int8_t offset);

void JR_C_e8(CPU *cpu, int8_t offset);

void JR_NC_e8(CPU *cpu, int8_t offset);

void DI(CPU *cpu);

void EI(CPU *cpu);

void STOP(CPU *cpu);

void RETI(CPU *cpu);


/*
    Prefixed ($CB $xx)
*/

void RLC(CPU * cpu, uint8_t * reg);

void RRC(CPU * cpu, uint8_t * reg);

void RL(CPU * cpu, uint8_t * reg);

void RR(CPU * cpu, uint8_t * reg);

void SLA(CPU * cpu, uint8_t * reg);

void SRA(CPU * cpu, uint8_t * reg);

void SWAP(CPU * cpu, uint8_t * reg);

void BIT(CPU * cpu, uint8_t bit, uint8_t * reg);

void RES(CPU * cpu, uint8_t bit, uint8_t * reg);

void SET(CPU * cpu, uint8_t bit, uint8_t * reg);

#endif