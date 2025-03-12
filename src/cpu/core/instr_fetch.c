#include "cpu/core/instr_fetch.h"
#include "memory/load_store.h"

extern register_file reg_file;

instr_t fetch_instruction(memory *mem) {
    // Check memory access validity
    mem_access_status status = check_mem_access(reg_file.pc, 4);
    if (status != MEM_ACCESS_OK){
        printf("instr_fetch: invalid memory access at address 0x%08x (fetch instruction)\n", reg_file.pc);
        return 0;
    }
    else{
        // Read four consecutive bytes and combine them (little-endian)
        data_t byte0 = mem_read(mem, reg_file.pc) & 0xFF;
        data_t byte1 = mem_read(mem, reg_file.pc + 1) & 0xFF;
        data_t byte2 = mem_read(mem, reg_file.pc + 2) & 0xFF;
        data_t byte3 = mem_read(mem, reg_file.pc + 3) & 0xFF;
        reg_file.pc += 4;
        return (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0;
    }
}
