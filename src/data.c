#include "data.h"

asm(".section .data\n"

#define INLINE_ASM(name, ext) \
  ".global _"#name"\n" \
  ".global _"#name"_end\n" \
  "_"#name":\n" \
  "  .incbin \"data/"#name"."#ext"\"\n" \
  "_"#name"_end:\n"

DATA_LIST(INLINE_ASM)

".section .text\n");

#undef INLINE_ASM
