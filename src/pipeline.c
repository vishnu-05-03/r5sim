#include "pipeline.h"

extern memory *mem;

void single_cycle_pipeline(register_file *reg_file){

    instr_t fetched_instr = fetch_instruction(mem);
    printf("instr_fetch: single instruction fetch successful.\n");

    exec_data dec_exec_data = decode_instr(fetched_instr, reg_file);
    printf("instr_decode: instruction decode successful.\n");
    // printf("instr_decode: decoded instruction: %d\n", dec_exec_data.instruction_name);
    // printf("instr_decode: rs1_data: %d\n", dec_exec_data.rs1_data);
    // printf("instr_decode: rs2_data: %d\n", dec_exec_data.rs2_data);
    // printf("instr_decode: rd: x%d\n", dec_exec_data.rd);
    // printf("instr_decode: sign_ext_imm: %d\n", dec_exec_data.sign_ext_imm);
    // printf("instr_decode: pc: 0x%x\n", dec_exec_data.pc);

    mem_data exec_mem_data = execute_instr(dec_exec_data);
    printf("instr_exec: instruction execution successful.\n");
    // printf("instr_exec: executed instruction: %d\n", exec_mem_data.instruction_name);
    // printf("instr_exec: rd: x%d\n", exec_mem_data.rd);
    // printf("instr_exec: arith_result: %d\n", exec_mem_data.arith_result);
    // printf("instr_exec: mem_addr: 0x%x\n", exec_mem_data.mem_addr);
    // printf("instr_exec: updated_pc: 0x%x\n", exec_mem_data.updated_pc);

    write_back_data mem_wb_data = memory_transaction(exec_mem_data, mem);
    print_mem_byte(mem, exec_mem_data.mem_addr);
    printf("mem_access: memory transaction successful.\n");
    // printf("mem_access: executed instruction: %d\n", mem_wb_data.instruction_name);
    // printf("mem_access: rd: x%d\n", mem_wb_data.rd);
    // printf("mem_access: rd data: %d\n", mem_wb_data.write_data);
    // printf("mem_access: updated pc: 0x%x\n", mem_wb_data.updated_pc);

    write_back(mem_wb_data, reg_file);

    printf("registers: --- register file status ---\n");
    reg_file_print(reg_file);
}

void five_stage_pipeline(register_file *reg_file) {
    // Pipeline registers
    typedef struct {
        instr_t instruction;
        reg_t pc;
        int valid;
    } if_id_reg;
    
    typedef struct {
        instr_name instruction_name;
        reg_t rs1_data;
        reg_t rs2_data;
        uint8_t rs1;
        uint8_t rs2;
        uint8_t rd;
        int32_t sign_ext_imm;
        reg_t pc;
        int valid;
    } id_ex_reg;
    
    typedef struct {
        instr_name instruction_name;
        uint8_t rd;
        reg_t arith_result;
        reg_t mem_addr;
        reg_t rs2_data;  // For store instructions
        reg_t updated_pc;
        int valid;
    } ex_mem_reg;
    
    typedef struct {
        instr_name instruction_name;
        uint8_t rd;
        reg_t write_data;
        reg_t updated_pc;
        int valid;
    } mem_wb_reg;
    
    // Initialize pipeline registers
    if_id_reg if_id = {0};
    id_ex_reg id_ex = {0};
    ex_mem_reg ex_mem = {0};
    mem_wb_reg mem_wb = {0};
    
    // RAW hazard tracking
    int stall_pipeline = 0;
    int insert_nop = 0;
    
    // Program end detection
    int end_of_program = 0;
    
    // Temp PC register to maintain PC correctly during stalls
    reg_t temp_pc = reg_file->pc;
    int cycles = 0;
    int instructions_completed = 0;
    int max_instructions = 100; // Safety limit
    
    printf("\n===== Starting Five-Stage Pipeline Simulation =====\n");
    
    while (instructions_completed < max_instructions && !end_of_program) {
        cycles++;
        printf("\n--- Clock Cycle %d ---\n", cycles);
        
        // =====================================================================
        // WRITE BACK STAGE
        // =====================================================================
        if (mem_wb.valid) {
            printf("WB: Writing back instruction %d\n", mem_wb.instruction_name);
            write_back_data wb_data = {
                .instruction_name = mem_wb.instruction_name,
                .rd = mem_wb.rd,
                .write_data = mem_wb.write_data,
                .updated_pc = mem_wb.updated_pc
            };
            write_back(wb_data, reg_file);
            instructions_completed++;
        } else {
            printf("WB: No valid instruction\n");
        }
        
        // =====================================================================
        // MEMORY STAGE
        // =====================================================================
        mem_wb.valid = ex_mem.valid;
        if (ex_mem.valid) {
            printf("MEM: Processing instruction %d\n", ex_mem.instruction_name);
            mem_data mem_data_in = {
                .instruction_name = ex_mem.instruction_name,
                .rd = ex_mem.rd,
                .arith_result = ex_mem.arith_result,
                .mem_addr = ex_mem.mem_addr,
                .updated_pc = ex_mem.updated_pc
            };
            
            // For store instructions, we need to pass the data to be stored
            // but we can't directly set rs2_data as it's not in mem_data struct
            // Instead, we'll need to handle it in our custom memory transaction function
            
            write_back_data mem_data_out = memory_transaction(mem_data_in, mem);
            
            // For debugging memory operations
            if (ex_mem.instruction_name == sb || ex_mem.instruction_name == lb) {
                print_mem_byte(mem, ex_mem.mem_addr);
            }
            
            mem_wb.instruction_name = mem_data_out.instruction_name;
            mem_wb.rd = mem_data_out.rd;
            mem_wb.write_data = mem_data_out.write_data;
            mem_wb.updated_pc = mem_data_out.updated_pc;
        } else {
            printf("MEM: No valid instruction\n");
            mem_wb.valid = 0;
        }
        
        // =====================================================================
        // EXECUTE STAGE
        // =====================================================================
        ex_mem.valid = id_ex.valid && !insert_nop;
        if (id_ex.valid && !insert_nop) {
            printf("EX: Executing instruction %d\n", id_ex.instruction_name);
            exec_data exec_data_in = {
                .instruction_name = id_ex.instruction_name,
                .rs1_data = id_ex.rs1_data,
                .rs2_data = id_ex.rs2_data,
                .rd = id_ex.rd,
                .sign_ext_imm = id_ex.sign_ext_imm,
                .pc = id_ex.pc
            };
            mem_data exec_data_out = execute_instr(exec_data_in);
            
            ex_mem.instruction_name = exec_data_out.instruction_name;
            ex_mem.rd = exec_data_out.rd;
            ex_mem.arith_result = exec_data_out.arith_result;
            ex_mem.mem_addr = exec_data_out.mem_addr;
            ex_mem.updated_pc = exec_data_out.updated_pc;
            ex_mem.rs2_data = id_ex.rs2_data; // Save for store instructions
        } else {
            printf("EX: No valid instruction or NOP inserted\n");
            ex_mem.valid = 0;
        }
        
        // =====================================================================
        // DECODE STAGE
        // =====================================================================
        id_ex.valid = if_id.valid && !stall_pipeline;
        if (if_id.valid && !stall_pipeline) {
            printf("ID: Decoding instruction 0x%08x\n", if_id.instruction);
            exec_data decoded = decode_instr(if_id.instruction, reg_file);
            
            // Extract rs1 and rs2 register numbers for hazard detection
            uint32_t instr = if_id.instruction;
            uint8_t rs1 = (instr >> 15) & 0x1F;
            uint8_t rs2 = (instr >> 20) & 0x1F;
            
            // Special case for S-type instructions where rs2 is in different position
            uint8_t opcode = instr & 0x7F;
            if (opcode == 0x23) { // Store instructions
                rs2 = (instr >> 20) & 0x1F;
            }
            
            id_ex.instruction_name = decoded.instruction_name;
            id_ex.rs1_data = decoded.rs1_data;
            id_ex.rs2_data = decoded.rs2_data;
            id_ex.rs1 = rs1;
            id_ex.rs2 = rs2;
            id_ex.rd = decoded.rd;
            id_ex.sign_ext_imm = decoded.sign_ext_imm;
            id_ex.pc = if_id.pc;
            
            // RAW Hazard Detection - Check both EX/MEM and MEM/WB stages
            insert_nop = 0;
            if (ex_mem.valid && ex_mem.rd != 0) {
                if ((rs1 != 0 && rs1 == ex_mem.rd) || 
                    (rs2 != 0 && rs2 == ex_mem.rd)) {
                    printf("ID: RAW hazard detected with EX/MEM stage (reg x%d). Inserting NOP\n", 
                           rs1 == ex_mem.rd ? rs1 : rs2);
                    insert_nop = 1;
                    stall_pipeline = 1; // Also stall the pipeline to keep fetch from advancing
                }
            }

            // Add check for MEM/WB stage too - needed for address dependencies
            if (mem_wb.valid && mem_wb.rd != 0) {
                if ((rs1 != 0 && rs1 == mem_wb.rd) || 
                    (rs2 != 0 && rs2 == mem_wb.rd)) {
                    printf("ID: RAW hazard detected with MEM/WB stage (reg x%d). Inserting NOP\n", 
                           rs1 == mem_wb.rd ? rs1 : rs2);
                    insert_nop = 1;
                    stall_pipeline = 1;
                }
            }
        } else {
            printf("ID: No valid instruction or pipeline stalled\n");
            id_ex.valid = 0;
        }
        
        // =====================================================================
        // FETCH STAGE 
        // =====================================================================
        if (!stall_pipeline) {
            // Save the current PC for this instruction
            reg_t current_pc = reg_file->pc;
            
            if (current_pc < 0x08000000 || current_pc >= 0x08000100) {
                printf("IF: PC out of program memory range (0x%08x), assuming end of program\n", current_pc);
                end_of_program = 1;
                if_id.valid = 0;
            } else {
                instr_t fetched = fetch_instruction(mem);
                printf("IF: Fetched instruction 0x%08x from PC 0x%08x\n", fetched, current_pc);
                
                if_id.instruction = fetched;
                if_id.pc = current_pc;
                if_id.valid = 1;
                
                // Check for program termination - null instruction (all zeros)
                if (fetched == 0) {
                    printf("IF: Detected null instruction, end of program\n");
                    end_of_program = 1;
                }
            }
        } else {
            printf("IF: Pipeline stalled, no fetch\n");
            stall_pipeline = 0;  // Reset stall for next cycle if no new hazards
        }
        
        // Print register file state periodically
        if (cycles % 5 == 0 || cycles == 1) {
            printf("\nRegister file state after cycle %d:\n", cycles);
            reg_file_print(reg_file);
        }
        
        // Check for empty pipeline - only if we've also reached end of program
        if (end_of_program && !if_id.valid && !id_ex.valid && !ex_mem.valid && !mem_wb.valid) {
            printf("\nAll pipeline stages empty and end of program reached. Simulation complete.\n");
            break;
        }
    }
    
    printf("\n===== Five-Stage Pipeline Simulation Complete =====\n");
    printf("Total cycles: %d, Instructions completed: %d\n", cycles, instructions_completed);
    printf("CPI (Cycles Per Instruction): %.2f\n", (float)cycles / instructions_completed);
    
    // Final register file state
    printf("\nFinal register file state:\n");
    reg_file_print(reg_file);
}