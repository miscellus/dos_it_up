#ifndef DATA_H
#define DATA_H

#include <stdint.h>

#define DATA_LIST(X) \
  X(sprite_sheet_1, img) \
  X(music, mod) \
  /* end */

#define DATA_DECL(name, ext) \
  extern uint8_t name[]; \
  extern uint8_t name##_end;

DATA_LIST(DATA_DECL)

#undef DATA_DECL

#define DATA_SIZE(name) (((uint32_t)&name##_end - (uint32_t)&name))

#endif /*DATA_H*/