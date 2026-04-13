#include <stdint.h>


extern void  mov_all_bytes_asm(uint8_t* buffer, uint64_t size);
extern void  cmp_all_bytes_asm(uint64_t size);
extern void  dec_all_bytes_asm(uint64_t size);

extern void  nop_all_bytes_asm(uint8_t* buffer, uint64_t size);
extern void  nop1x3_asm(uint8_t* buffer, uint64_t size);
extern void  nop1x9_asm(uint8_t* buffer, uint64_t size);
