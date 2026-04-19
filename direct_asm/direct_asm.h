#include <stdint.h>


extern void mov_all_bytes_asm(uint8_t* buffer, uint64_t size);
extern void cmp_all_bytes_asm(uint64_t size);
extern void dec_all_bytes_asm(uint64_t size);
extern void mov_cond_jump_asm(uint8_t* buffer, uint64_t size);

extern void nop_all_bytes_asm(uint8_t* buffer, uint64_t size);

extern void nop1x3_asm(uint8_t* buffer, uint64_t size);
extern void nop1x3_extended_asm(uint8_t* buffer, uint64_t size);
extern void nop1x9_asm(uint8_t* buffer, uint64_t size);

extern void aligned_asm(uint64_t size);
extern void plus1_aligned_asm(uint64_t size);
extern void plus15_aligned_asm(uint64_t size);
extern void plus31_aligned_asm(uint64_t size);
extern void plus63_aligned_asm(uint64_t size);

extern void RAT_add();
extern void RAT_mov_add();

extern void mov_8x1_asm(void* buffer);
extern void mov_8x2_asm(void* buffer);
extern void mov_8x3_asm(void* buffer);
extern void mov_8x4_asm(void* buffer);
extern void mov_1x2_asm(void* buffer);
extern void write_8x1_asm(void* buffer);
extern void write_8x2_asm(void* buffer);
extern void write_8x3_asm(void* buffer);
extern void write_8x4_asm(void* buffer);
extern void write_1x2_asm(void* buffer);


extern void mov_8bit_asm(uint8_t* buffer, uint64_t size);
extern void mov_16bit_asm(uint8_t* buffer, uint64_t size);
extern void mov_32bit_asm(uint8_t* buffer, uint64_t size);
extern void mov_64bit_asm(uint8_t* buffer, uint64_t size);
extern void mov_128bit_asm(uint8_t* buffer, uint64_t size);
extern void mov_256bit_asm(uint8_t* buffer, uint64_t size);


extern void cache_test_asm(uint8_t* buffer, uint64_t test_size, uint64_t test_count);
extern void cache_test_unaligned_asm(uint8_t* buffer, uint64_t test_size, uint64_t test_count, uint64_t offset);
extern void cache_test_mask_asm(uint8_t* buffer, uint64_t size, uint64_t bit_mask);
extern void cache_sets_test_asm(uint8_t* buffer, uint64_t inner_loop_count, uint64_t outer_loop_count, uint64_t pt_advance);

