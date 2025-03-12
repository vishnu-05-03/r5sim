#include "utils/elf_parser.h"

int extract_instructions(const char *elf_file_path) {
  FILE *elf_file = fopen(elf_file_path, "rb");
  if (!elf_file) {
    perror("elf parser: ELF file open failed.\n");
    return 1;
  }

  Elf32_Ehdr elf_header;
  fread(&elf_header, sizeof(Elf32_Ehdr), 1, elf_file);

  if (memcmp(elf_header.e_ident, ELFMAG, SELFMAG) != 0) {
    fprintf(stderr, "elf parser: not a valid ELF file.\n");
    fclose(elf_file);
    return 2;
  }

  // Check ELF file endianness
  int need_swap = 0;
  if (elf_header.e_ident[EI_DATA] == ELFDATA2LSB) {
    // ELF is little-endian
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      need_swap = 1;  // Host is big-endian, need to swap
    #endif
  } else if (elf_header.e_ident[EI_DATA] == ELFDATA2MSB) {
    // ELF is big-endian
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      need_swap = 1;  // Host is little-endian, need to swap
    #endif
  } else {
    fprintf(stderr, "elf parser: unknown ELF endianness.\n");
    fclose(elf_file);
    return 3;
  }

  fseek(elf_file, elf_header.e_shoff, SEEK_SET);
  Elf32_Shdr section_headers[elf_header.e_shnum];
  fread(section_headers, sizeof(Elf32_Shdr), elf_header.e_shnum, elf_file);

  FILE *output_file = fopen(OUTPUT_FILE_PATH, "w");
  if (!output_file) {
    perror("elf parser: output file open failed.\n");
    fclose(elf_file);
    return 1;
  }

  for (int i = 0; i < elf_header.e_shnum; i++) {
    if (section_headers[i].sh_type == SHT_PROGBITS &&
        section_headers[i].sh_flags & SHF_EXECINSTR) {
      fseek(elf_file, section_headers[i].sh_offset, SEEK_SET);
      uint8_t *instructions = malloc(section_headers[i].sh_size);
      if (!instructions) {
        perror("elf parser: instruction buffer memory allocation failed.\n");
        fclose(output_file);
        fclose(elf_file);
        return 3;
      }
      fread(instructions, section_headers[i].sh_size, 1, elf_file);

      uint32_t base_address = section_headers[i].sh_addr;
      for (size_t j = 0; j < section_headers[i].sh_size; j += 4) {
        uint32_t instruction = 0;
        // Explicitly construct the 32-bit value from bytes
        instruction = (uint32_t)instructions[j] |
                     ((uint32_t)instructions[j+1] << 8) |
                     ((uint32_t)instructions[j+2] << 16) |
                     ((uint32_t)instructions[j+3] << 24);
        
        // Swap bytes if needed
        if (need_swap) {
          instruction = __builtin_bswap32(instruction);
        }
        
        fprintf(output_file, "0x%08x: 0x%08x\n", base_address + j, instruction);
      }
      free(instructions);
    }
  }

  fclose(output_file);
  fclose(elf_file);

  return 0;
}
