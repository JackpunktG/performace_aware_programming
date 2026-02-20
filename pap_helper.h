/* Following along with cassy Muratori performace awareness programming course, a header lib for helper functions. */
#ifndef PAP_HELPER_H
#define PAP_HELPER_H


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

/* ========================================================================
    Timer
    ======================================================================== */
typedef struct
{
    struct timespec before, after;
} Timer;

void start_timer(Timer* timer);
void end_timer(Timer* timer);
void timer_print_sec(Timer* timer);
void timer_print_nano(Timer* timer);
double timer_sec(Timer* timer);
uint64_t timer_nano(Timer* timer);
void compare_timers(Timer* timer1, Timer* timer2);

/* ========================================================================
    Arena Memory Allocator
    ======================================================================== */
#define ARENA_BLOCK_SIZE  (1024 * 1024)   //size of block currently 1MB

/* Dynamic size Arena Block Allocation */ //if over the defualt size will make a block for that size
typedef struct Arena_Block
{
    uint8_t* memory;            //raw memory -> 1 per byte
    size_t size;                //total block size
    size_t used;                //how much of the block is used
    struct Arena_Block* next;   //link to next block if we need more space
} Arena_Block;

/* Arena Structure */
typedef struct
{
    Arena_Block* current;       //currently allocating block
    Arena_Block* first;         //start of our block
    size_t total_allocated;     //Total byte allocated
    size_t defualt_block_size;  // size of new block
    size_t alignment;           //number of bits the Arena should be aligned to
} Arena;
/* Initaialize the arena with a default block size and alignment, returns NULL if failed to initialize */
Arena* arena_init(size_t defualt_block_size, size_t alignment);
/* Destroy the arena and free all associated memory */
void arena_destroy(Arena* arena);
/* Allocates memory from the arena. Returns NULL if allocation fails. The out_size_alloc sends back total allocated space, can be NULL if not needed.*/
void* arena_alloc(Arena* arena, size_t size, size_t* out_size_alloc);
void arena_reset(Arena* arena); //just restting all the allocated counters to zero and ptr to the start for the blocks
//to realloc, old_size is needed to add back to free list properly
void* arena_realloc(Arena* arena, void* old_ptr, size_t new_size, size_t* out_size_alloc);

void print_binary_32(const uint32_t var_32);
void print_binary_16(const uint16_t var_16);
void print_binary_8(const uint8_t var_8);



#endif // PAP_HELPER_H
#ifdef PAP_HELPER_IMPLEMENTATION

void start_timer(Timer* timer)
{
    clock_gettime(CLOCK_MONOTONIC, &timer->before);
}

void end_timer(Timer* timer)
{
    clock_gettime(CLOCK_MONOTONIC, &timer->after);
}

void timer_print_sec(Timer* timer)
{
    printf("Time taken: %f seconds\n", (timer->after.tv_sec - timer->before.tv_sec) + (timer->after.tv_nsec - timer->before.tv_nsec) / 1e9);
}
void timer_print_nano(Timer* timer)
{
    printf("Time taken: %lu nanoseconds\n", (uint64_t)((timer->after.tv_sec - timer->before.tv_sec) * 1e9 + (timer->after.tv_nsec - timer->before.tv_nsec)));
}

double timer_sec(Timer* timer)
{
    return ((timer->after.tv_sec - timer->before.tv_sec) + (timer->after.tv_nsec - timer->before.tv_nsec) / 1e9);
}

uint64_t timer_nano(Timer* timer)
{
    return (uint64_t)((timer->after.tv_sec - timer->before.tv_sec) * 1e9 + (timer->after.tv_nsec - timer->before.tv_nsec));
}

void compare_timers(Timer* timer1, Timer* timer2)
{
    double time1 = (timer1->after.tv_sec - timer1->before.tv_sec) + (timer1->after.tv_nsec - timer1->before.tv_nsec) / 1e9;
    double time2 = (timer2->after.tv_sec - timer2->before.tv_sec) + (timer2->after.tv_nsec - timer2->before.tv_nsec) / 1e9;

    if (time1 < time2)
        printf("Timer 1 is faster by %f seconds\n", time2 - time1);
    else if (time2 < time1)
        printf("Timer 2 is faster by %f seconds\n", time1 - time2);
    else
        printf("Both timers are equal\n");
}

// Arena memory implementation
static Arena_Block* arena_add_block(Arena *arena, size_t minimumSize)
{
    //checking if size fits in one block
    size_t block_size = minimumSize > arena->defualt_block_size ? minimumSize : arena->defualt_block_size;

    // allocate block struct
    Arena_Block* block = (Arena_Block*)malloc(sizeof(Arena_Block));
    if(!block)
    {
        printf("ERROR - could not allocate block struct\n");
        return NULL;
    }
    // allocating the actual memory
    block->memory = (uint8_t*)malloc(block_size);
    if(!block->memory)
    {
        free(block);
        printf("ERROR - could not allocate block memory\n");
        return NULL;
    }

    //init the block members
    block->size = block_size;
    block->used = 0;
    block->next = NULL;

    //link into arena
    if (arena->current)   //if its not the first block, linking it to the last block before allocating it to current
        arena->current->next = block;
    else
        arena->first = block;

    arena->current = block;

    return block;
}

//alows for different alignment
static size_t align_to(size_t size, size_t alignment)
{
    return (size + (alignment -1)) & ~(alignment -1);
}

Arena* arena_init(size_t block_size, size_t alignment)
{
    //checking for 0 and must be a power of two
    if (alignment == 0 || (alignment & (alignment -1)))
    {
        printf("ERROR - alignment cannot be 0 and must be a power of two\n");
        return NULL;
    }

    if (block_size != align_to(block_size, alignment))
    {
        printf("WARNING - Mismatch with defualt Block Size and alignment\n");
        block_size = align_to(block_size, alignment);
        printf("New defualt_block_size based on your alignment: %lu\n", block_size);
    }

    //Init arena controller struct
    Arena* arena = (Arena*)malloc(sizeof(Arena));
    if (!arena)
    {
        printf("ERROR - arena_init malloc failed\n");
        return NULL;
    }


    //init fields
    arena->defualt_block_size = block_size;
    arena->total_allocated = 0;
    arena->alignment = alignment;
    arena->current = NULL;
    arena->first = NULL;

    // Create first block
    if(!arena_add_block(arena, block_size))
    {
        free(arena);
        printf("ERROR - Failed to create first block");
        return NULL;
    }
    return arena;
}



void* arena_alloc(Arena* arena, size_t size, size_t* size_alloc)
{
    if (!arena || !size)
    {
        printf("arena? size: %p %lu\n", (void*)arena, size);
        printf("ERROR - arena or size are NULL\n");
        return NULL;
    }

    //align size to next mulitiple of 8 bytes
    size = align_to(size, arena->alignment);

    //check if current block has enough space
    if (!arena->current || arena->current->used + size > arena->current->size)
    {
        //new block if size is not enough
        if(!arena_add_block(arena, size))
            return NULL;
    }


    //get the pointer to the space
    void* ptr = arena->current->memory + arena->current->used;


    //updated current used and total used
    arena->current->used += size;
    arena->total_allocated += size;

    if (size_alloc != NULL)
        *size_alloc = size;

    return ptr;
}

void arena_reset(Arena* arena)
{
    if (!arena)
    {
        printf("ERROR - arena is already NULL before the reset\n");
        return;
    }

    //Marking all blocks as empty
    Arena_Block* block = arena->first;
    while(block)
    {
        block->used = 0;
        block = block->next;
    }

    arena->current = arena->first;
    arena->total_allocated = 0;
}

void* arena_realloc(Arena* arena, void* old_ptr, size_t new_size, size_t* out_size_alloc)
{
    if (!arena || !old_ptr || !new_size)
    {
        printf("ERROR - paramater are NULL\n");
        return NULL;
    }

    //allocate new space
    void* new_ptr = arena_alloc(arena, new_size, out_size_alloc);
    if (!new_ptr)
    {
        printf("ERROR - arena_realloc failed to allocate new memory\n");
        return NULL;
    }

    memcpy(new_ptr, old_ptr, new_size);

    if (out_size_alloc != NULL)
        *out_size_alloc = new_size;

    return new_ptr;
}


void arena_destroy(Arena* arena)
{
    if (!arena)
    {
        printf("ERROR - arena is already NULL before the destroy\n");
        return;
    }
    //free all blocks
    Arena_Block* block = arena->first;
    while(block)
    {
        Arena_Block* next = block->next;
        free(block->memory);
        free(block);
        block = next;
    }

    free(arena);
}

void print_binary_32(const uint32_t var_32)
{
    for (int i = 31; i >= 0; --i)
    {
        if (var_32 & (1<<i))
            printf("1");
        else printf("0");
    }
    printf("\n");
}

void print_binary_16(const uint16_t var_16)
{
    for (int i = 15; i >= 0; --i)
    {
        if (var_16 & (1<<i))
            printf("1");
        else printf("0");
    }
    printf("\n");
}

void print_binary_8(const uint8_t var_8)
{

    for (int i = 7; i >= 0; --i)
    {
        if (var_8 & (1<<i))
            printf("1");
        else printf("0");
    }
    printf("\n");
}
#endif // PAP_HELPER_H_IMPLEMENTATION
