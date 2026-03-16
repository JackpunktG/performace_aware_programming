/* =======================================================================
   Following along with cassy Muratori performace awareness programming course,
   a header lib for helper functions.
    ======================================================================== */
#ifndef PERFORMACE_AWARE_PROGRAMMING_HELPER_H
#define PERFORMACE_AWARE_PROGRAMMING_HELPER_H


#ifdef DEBUG
#define DEBUG_PRINT(print) \
    do { \
        fprintf(stderr, "[DEBUG] %s:%d:%s(): ", __FILE__, __LINE__, __func__); \
        print; \
    } while(0)
#else
#define DEBUG_PRINT(print)
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>

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


#define LINE_P 0
#define NEWLINE_P 1
void print_binary_32(const uint32_t var_32, uint8_t new_line);
void print_binary_16(const uint16_t var_16, uint8_t new_line);
void print_binary_8(const uint8_t var_8, uint8_t new_line);


/* ========================================================================
    String
    ======================================================================== */

typedef struct
{
    uint64_t count;
    uint8_t* data;
} String;

#define CONSTANT_STRING(String) {sizeof(String) -1, (uint8_t*)(String)}
#define STRLEN(String) (String->count)

static inline bool are_equal(String* a, String* b);
static inline bool in_bounds(String* a, uint64_t index);
static inline void string_print(String* string);
static inline void string_println(String* string);
static inline String* string_init(uint64_t count, Arena* arena);
//assumes malloc creation
static inline void string_destroy(String* string);
static String* read_entire_file(char* file_name, Arena* arena);


#endif // PERFORMACE_AWARE_PROGRAMMING_HELPER_H
#ifdef PERFORMACE_AWARE_PROGRAMMING_HELPER_IMPLEMENTATION

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

void print_binary_32(const uint32_t var_32, uint8_t new_line)
{
    for (int i = 31; i >= 0; --i)
    {
        if (var_32 & (1<<i))
            printf("1");
        else printf("0");
    }
    if (new_line == NEWLINE_P)
        printf("\n");
}

void print_binary_16(const uint16_t var_16, uint8_t new_line)
{
    for (int i = 15; i >= 0; --i)
    {
        if (var_16 & (1<<i))
            printf("1");
        else printf("0");
    }
    if (new_line == NEWLINE_P)
        printf("\n");
}

void print_binary_8(const uint8_t var_8, uint8_t new_line)
{

    for (int i = 7; i >= 0; --i)
    {
        if (var_8 & (1<<i))
            printf("1");
        else printf("0");
    }
    if (new_line == NEWLINE_P)
        printf("\n");
}


static inline bool are_equal(String* a, String* b)
{
    if(a->count != b->count)
        return false;

    for(uint64_t index = 0; index < a->count; ++index)
    {
        if(a->data[index] != b->data[index])
            return false;
    }
    return true;
}
static inline bool in_bounds(String* a, uint64_t index)
{
    return index < a->count;
}

static inline String* string_init(uint64_t count, Arena* arena)
{
    String* s = NULL;

    if (arena != NULL)
        s = (String*)arena_alloc(arena, sizeof(String) + (sizeof(uint8_t) *count), NULL);
    else
        s = (String*)malloc(sizeof(String) + (sizeof(uint8_t) *count));

    s->data = (uint8_t*)(s +1);
    s->count = count;

    return s;
}

static inline void string_destroy(String* string)
{
    free(string);
    string = NULL;
}

static String* read_entire_file(char* file_name, Arena* arena)
{
    String* s = NULL;

    FILE* file = fopen(file_name, "rb");

    if(file)
    {
        struct stat file_info;
        stat(file_name, &file_info);

        s = string_init(file_info.st_size, arena);
        if(s->data)
        {
            if (fread(s->data, s->count, 1, file) != 1)
            {
                fprintf(stderr, "ERROR - unable to read file %s\n", file_name);
                if (arena == NULL)
                    string_destroy(s);
            }
        }
        fclose(file);
    }
    else
        fprintf(stderr, "ERROR - unable to open file %s\n", file_name);

    return s;
}

static inline void string_print(String* string)
{
    for (uint64_t i = 0; i < string->count; ++i)
        printf("%c", string->data[i]);
}
static inline void string_println(String* string)
{
    for (uint64_t i = 0; i < string->count; ++i)
        printf("%c", string->data[i]);
    printf("\n");
}
#endif // PAP_HELPER_H_IMPLEMENTATION
