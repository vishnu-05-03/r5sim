#include "cpu/core/registers.h"
#include "memory/memory.h"
#include "utils/config_parser.h"
#include "utils/elf_parser.h"

#include "cpu/core/instr_fetch.h"
#include "cpu/core/instr_exec.h"
#include "cpu/core/mem_access.h"
#include "cpu/core/write_back.h"

register_file reg_file;
extern memory *mem;

int main(int argc, char *argv[]) {
  if (parse_configs())
    return 1;
  printf("yaml stream: parsed successfully.\n"); 

  if (extract_instructions(argv[1]))
    return 1;
  printf("elf parser: parsed successfully; dump generated.\n");

  reg_file_init(&reg_file);
  printf("registers: register file init successful.\n");

  if (mem_init())
    return 1;
  printf("mem: memory init successful.\n");

  for(int i=0; i<6; i++)
  {
  instr_t fetched_instr = fetch_instruction(mem);
  printf("instr_fetch: single instruction fetch successful.\n");

  exec_data dec_exec_data = decode_instr(fetched_instr, &reg_file);
  printf("instr_decode: instruction decode successful.\n");

  printf("instr_decode: decoded instruction: %d\n", dec_exec_data.instruction_name);
  printf("instr_decode: rs1_data: %d\n", dec_exec_data.rs1_data);
  printf("instr_decode: rs2_data: %d\n", dec_exec_data.rs2_data);
  printf("instr_decode: rd: x%d\n", dec_exec_data.rd);
  printf("instr_decode: sign_ext_imm: %d\n", dec_exec_data.sign_ext_imm);
  printf("instr_decode: pc: 0x%x\n", dec_exec_data.pc);

  mem_data exec_mem_data = execute_instr(dec_exec_data);
  printf("instr_exec: instruction execution successful.\n");

  printf("instr_exec: executed instruction: %d\n", exec_mem_data.instruction_name);
  printf("instr_exec: rd: x%d\n", exec_mem_data.rd);
  printf("instr_exec: arith_result: %d\n", exec_mem_data.arith_result);
  printf("instr_exec: mem_addr: 0x%x\n", exec_mem_data.mem_addr);
  printf("instr_exec: updated_pc: 0x%x\n", exec_mem_data.updated_pc);

  write_back_data mem_wb_data = memory_transaction(exec_mem_data, mem);
  print_mem_byte(mem, exec_mem_data.mem_addr);
  printf("mem_access: memory transaction successful.\n");
  printf("mem_access: executed instruction: %d\n", mem_wb_data.instruction_name);
  printf("mem_access: rd: x%d\n", mem_wb_data.rd);
  printf("mem_access: rd data: %d\n", mem_wb_data.write_data);
  printf("mem_access: updated pc: 0x%x\n", mem_wb_data.updated_pc);

  write_back(mem_wb_data, &reg_file);

  printf("registers: --- register file status ---\n");
  reg_file_print(&reg_file);
}

  return 0;
}
