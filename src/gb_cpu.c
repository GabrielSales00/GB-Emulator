#include "include/gb_cpu.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

unsigned char load_rom(char * path) {
    unsigned char bootrom[0x100]; //bootrom's size is 256 bytes long from 0x0000 0x00ff

    FILE * f = fopen(path, "rb");

    if (!f) {
        perror("Be for real. Find the bootrom.");
        return;
    }

    size_t size_read = fread(bootrom, 1, sizeof(bootrom), f);

    if (size_read != sizeof(bootrom)) {
        fprintf("Size mismatch between the provided bootrom file and the expected size (256 bytes).\
            Check if the file is correct", stderr);
    }

    fclose(f);

    return bootrom;
}

uint8_t cpu_read8(CPU * cpu, uint8_t addr);

uint8_t cpu_write8(CPU * cpu, uint8_t reg, uint8_t addr);

void set_flags_false_all(CPU * cpu) {
    cpu->flags.zero = false;
    cpu->flags.subtraction = false;
    cpu->flags.h_carry = false;
    cpu->flags.carry=false;
}

/*
    Instruction set (ISA)
*/

void NOP(CPU * cpu) {
    // Nothing
}

void INC_8(CPU * cpu, uint8_t * reg) {
    uint8_t result = reg + 1;
    cpu->flags.subtraction = false;
    if (result == 0) {
        cpu->flags.zero = true;
    }
    if ((*reg & 0x0F) + 1 > 0x0F) {
        cpu->flags.h_carry = true;
    }
    *reg = result;
}

void INC_16(CPU * cpu, uint16_t * reg) {
    reg += 1;
}

void DEC_8(CPU * cpu, uint8_t * reg) {
    uint8_t result = reg - 1;
    cpu->flags.subtraction = true;
    if (result == 0) {
        cpu->flags.zero = true;
    }
    if ((*reg & 0x0F) == 0) {
        cpu->flags.h_carry = true;
    }
    *reg = result;
}

void INC_mem(CPU *cpu, uint16_t addr) {
    uint8_t value = cpu->memory[addr];
    uint8_t result = value + 1;
    cpu->flags.subtraction = false;
    if (result == 0) {
        cpu->flags.zero = true;
    }
    if ((value & 0x0F) + 1 > 0x0F) {
        cpu->flags.h_carry = true;
    }
    cpu->memory[addr] = result;

}

void DEC_mem(CPU *cpu, uint16_t addr) {
    uint8_t value = cpu->memory[addr];
    uint8_t result = value - 1;
    cpu->flags.subtraction = true;
    if (result == 0) {
        cpu->flags.zero = true;
    }
    if ((value & 0x0F) == 0) {
        cpu->flags.h_carry = true;
    }
    cpu->memory[addr] = result;
}

void RLCA(CPU * cpu) {
    uint8_t a = cpu->reg.a;
    uint8_t bit7 = (a >> 7) & 1;

    cpu->reg.a = (a << 1) | bit7;

    cpu->flags.zero = false;
    cpu->flags.subtraction = false;
    cpu->flags.h_carry = false;
    if (bit7 == 1) {
        cpu->flags.carry = true;
    }
    else {
        cpu->flags.carry = false;
    }

}

void RRCA(CPU * cpu) {
    uint8_t a = cpu->reg.a;
    uint8_t bit0 = (a << 7) & 1;
    
    cpu->reg.a = (a >> 1) | bit0;

    cpu->flags.zero = false;
    cpu->flags.subtraction = false;
    cpu->flags.h_carry = false;
    if (bit0 == 1) {
        cpu->flags.carry = true;
    }
    else {
        cpu->flags.carry = false;
    }
}

void RLA(CPU * cpu, uint8_t * reg) {
    uint8_t old_carry = cpu->flags.carry ? 1 : 0;
    uint8_t new_carry = (*reg >> 7) & 1;
    *reg = (*reg << 1) | old_carry;
    cpu->flags.carry = new_carry;
    cpu->flags.subtraction = false;
    cpu->flags.h_carry = false;
    if (*reg == 0) {
        cpu->flags.zero = true;
    }

}

void RRA(CPU * cpu) {
    uint8_t a = cpu->reg.a;
    uint8_t b0 = cpu->reg.a & 1;
    cpu->reg.a = (cpu->reg.a >> 1) | (cpu->flags.carry << 7);
    cpu->flags.carry = b0;
    cpu->flags.zero = false;
    cpu->flags.subtraction = false;
    cpu->flags.h_carry = false;
}

void DAA(CPU * cpu) {
    uint8_t adjustment = 0;
    if (cpu->flags.subtraction) {
        if (cpu->flags.h_carry) {
            adjustment += 0x06;
        }
        if (cpu->flags.carry) {
            adjustment += 0x60;
        }
        cpu->reg.a -= adjustment;
    } else {
        if (cpu->flags.h_carry || (cpu->reg.a & 0x0F > 0x09)) {
            adjustment += 0x06;
        }
        if (cpu->flags.carry || cpu->reg.a > 0x99) {
            adjustment += 0x60;
        }
        cpu->reg.a += adjustment;
    }
    cpu->flags.h_carry = false;
}

void CPL(CPU * cpu) {
    cpu->reg.a = ~cpu->reg.a;

    cpu->flags.subtraction = true;
    cpu->flags.h_carry = true;
}

void SCF(CPU * cpu) {
    cpu->flags.carry = true;
    cpu->flags.subtraction = false;
    cpu->flags.h_carry = false;
}

void CCF(CPU * cpu) {
    cpu->flags.carry = ~cpu->flags.carry;
    cpu->flags.subtraction = false;
    cpu->flags.h_carry = false;
}

void HALT(CPU * cpu) {
    if (cpu->state.ime) {
        cpu->state.halted = true;
    } else if ((cpu->state.ie & cpu->state.iflags) == false) {
        cpu->state.halted = true;
    } else {
        cpu->state.halt_bug = true;
    }
}

void ADD_A(CPU * cpu, uint8_t value) {
    uint16_t result = cpu->reg.a + value;
    
    clear_flags(cpu);

    if ((result & 0xFF) == 0) {
        cpu->flags.zero = true;
    }

    if((cpu->reg.a & 0x0F) + (value & 0x0F) > 0x0F) {
        cpu->flags.h_carry = true;
    }

    if ((result > 0xFF)) {
        cpu->flags.carry = true;
    }

    cpu->reg.a += (uint8_t) result;
}

void ADC_A(CPU * cpu, uint8_t value) {
    uint8_t carry = cpu->flags.carry;
    uint16_t result = cpu->reg.a + cpu->flags.carry + value;

    clear_flags(cpu);

    if ((result & 0xFF) == 0) {
        cpu->flags.zero = true;
    }

    if ((cpu->reg.a & 0x0F) + (value & 0x0F) + carry > 0x0F) {
        cpu -> flags.h_carry = true;
    }

    if (result > 0XFF) {
        cpu -> flags.carry = true;
    }

    cpu->reg.a += (uint8_t)result;

}


void SUB_A(CPU * cpu, uint8_t value) {
    uint16_t result = cpu->reg.a - value;
    cpu->flags.subtraction = true;
    if ((result & 0xFF) == 0) {
        cpu->flags.zero = true;
    }
    if ((value & 0x0F) > (cpu->reg.a & 0x0F)) {
        cpu->flags.h_carry = true;
    }
    if (value > (result & 0xFF)) {
        cpu->flags.carry = true;
    }
    cpu->reg.a = result;
}

void SBC_A(CPU * cpu, uint8_t value) {
    uint16_t result = cpu->reg.a - value - cpu->flags.carry;
    cpu->flags.subtraction = true;
    if ((result & 0xFF) == 0) {
        cpu->flags.zero = true;
    }
    if ((cpu->flags.carry & 0x0F) + (value & 0x0F) > (cpu->reg.a & 0x0F)) {
        cpu->flags.h_carry = true;
    }
    if (cpu->flags.carry + value > result & 0xFF) {
        cpu->flags.carry = true;
    }
    cpu->reg.a = result;
}

void AND_A(CPU * cpu, uint8_t value) {
    cpu->reg.a = (cpu->reg.a & value);
}

void XOR_A(CPU * cpu, uint8_t value) {
    cpu->reg.a = (cpu->reg.a ^ value);
}

void OR_A(CPU * cpu, uint8_t value) {
    cpu->reg.a = (cpu->reg.a | value);
}

void CP_A(CPU * cpu, uint8_t value) {
    uint16_t result = cpu->reg.a - value;
    cpu->flags.subtraction = true;
    if ((result & 0xFF) == 0) {
        cpu->flags.zero = true;
    }
    if ((value & 0x0F) > (cpu->reg.a & 0x0F)) {
        cpu->flags.h_carry = true;
    }
    if (value > (result & 0xFF)) {
        cpu->flags.carry = true;
    }
}

void RET(CPU *cpu) {
    uint8_t lo = cpu_read8(cpu, cpu->reg.sp);
    uint8_t hi = cpu_read8(cpu, cpu->reg.sp+1);
    cpu->reg.sp += 2;
    cpu->reg.pc = (hi << 8) | lo; //(hi << 8) is 16 bit
}

void RET_Z(CPU *cpu) {
    if (cpu->flags.zero) {
        RET(cpu);
    }
}

void RET_NZ(CPU *cpu) {
    if (!cpu->flags.zero) {
        RET(cpu);
    }
}

void RET_C(CPU *cpu) {
    if (cpu->flags.carry) {
        RET(cpu);
    }
}

void RET_NC(CPU *cpu) {
    if (!cpu->flags.carry) {
        RET(cpu);
    }
}

void POP_16(CPU * cpu, uint16_t * reg) {
    uint16_t low = cpu_read8(cpu, cpu->reg.sp);
    cpu->reg.sp += 1;
    uint16_t high = cpu_read8(cpu, cpu->reg.sp);
    cpu->reg.sp += 1;

    *reg = (high << 8) | low;

}

void JP(CPU * cpu) { //Jump to immediate next 16 bit
    uint8_t high = cpu_read8(cpu, cpu->reg.pc + 1);
    uint8_t low = cpu_read8(cpu, cpu->reg.pc+2);

    uint16_t addr = (high << 8) | low;

    cpu->reg.pc = addr;
}

void CALL(CPU * cpu, uint16_t addr) {
    uint8_t high = cpu_read16(cpu, cpu->reg.pc+1);
    uint8_t low = cpu_read16(cpu, cpu->reg.pc+2);

    uint16_t addr = (high << 8) | low;

    //return address of the next instruction and call is 3 bytes
    uint16_t ret = cpu->reg.pc+3;

    //push high
    cpu->reg.sp--;
    cpu_write8(cpu, cpu->reg.sp, (ret >> 8) & 0xFF);

    cpu->reg.sp--;
    cpu_write8(cpu, cpu->reg.sp, ret & 0xFF);

    cpu->reg.pc = addr;

}

void PUSH_16(CPU * cpu, uint16_t * reg) {
    cpu->reg.sp--;
    cpu_write8(cpu, cpu->reg.sp, (*reg >> 8) & 0xFF);

    cpu->reg.sp--;
    cpu_write8(cpu, cpu->reg.sp, *reg & 0xFF);
}

void ADD_HL(CPU * cpu, uint16_t value) {
    uint32_t result = cpu->reg.hl + value;

    cpu->flags.subtraction = false;

    if ((cpu->reg.hl & 0x0FFF) + (value & 0x0FFF) > 0x0FFF) {
        cpu->flags.h_carry = true;
    }
    else {
        cpu->flags.h_carry = false;
    }
    if (result > 0xFFFF) {
        cpu->flags.carry = true;
    }
    else {
        cpu->flags.carry = false;
    }

    cpu->reg.hl = (uint16_t) result;
}

void RST_8(CPU * cpu, uint8_t value);

void PREFIX_CB(CPU * cpu);

void LDH_mem_A(CPU *cpu, uint8_t offset);

void LDH_A_mem(CPU *cpu, uint8_t offset);

void LD_C_A(CPU *cpu);

void LD_A_C(CPU *cpu);

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