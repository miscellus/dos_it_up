#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#define DATA_DECL(name) \
  extern uint8_t name[]; \
  extern uint8_t name##_end;

IMAGE_NAMES(DATA_DECL)

#undef DATA_DECL

#define IMAGE_SIZE(name) (((uint32_t)&name##_end - (uint32_t)&name))

#endif /*IMAGE_DATA_H*/
#ifdef IMAGE_DATA_IMPLEMENTATION

asm(".section .data\n"

#define INLINE_ASM(name) \
  ".global _"#name"\n" \
  ".global _"#name"_end\n" \
  "_"#name":\n" \
  "  .incbin \"build/"#name".img\"\n" /*".byte 0xac,0xdc,0xac,0xdc,0xaa,0xaa,0xcc,0xcc,0xcc \n"*/ \
  "_"#name"_end:\n"

  IMAGE_NAMES(INLINE_ASM)

".section .text\n");

#undef INLINE_ASM

#endif /*IMAGE_DATA_IMPLEMENTATION*/