#include "utils/config_parser.h"
#include "utils/elf_parser.h"

#include "memory/memory.h"
#include "cpu/core/registers.h"
#include "cpu/core/instr_fetch.h"
#include "cpu/core/instr_exec.h"
#include "cpu/core/mem_access.h"
#include "cpu/core/write_back.h"

#include "pipeline.h"

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

  // for(int i=0; i<6; i++)
  // {
  //   single_cycle_pipeline(&reg_file);
  // }

  five_stage_pipeline(&reg_file);

  return 0;
}
