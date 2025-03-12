#include "cpu/core/registers.h"
#include "memory/memory.h"
#include "cpu/core/instr_fetch.h"
#include "cpu/core/instr_exec.h"
#include "cpu/core/mem_access.h"
#include "cpu/core/write_back.h"

void single_cycle_pipeline(register_file *reg_file);

void five_stage_pipeline(register_file *reg_file);