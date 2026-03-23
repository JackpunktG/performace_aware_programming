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
        (print); \
    } while(0);
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
#include <math.h>
#include <sys/time.h>
#include <x86intrin.h>
#include <assert.h>

/* ========================================================================
    Haversine Calculation
    ======================================================================== */
double calculate_haversine(double* points, uint64_t count);

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

#define STR(litteral) (String){sizeof(litteral) -1, (uint8_t*)(litteral)}
#define STRLEN(String) (String->count)

static inline bool are_equal(String* a, String* b);
static inline bool in_bounds(String* a, uint64_t index);
static inline void string_print(String* string);
static inline void string_println(String* string);
static inline String* string_init(uint64_t count, Arena* arena);
//assumes malloc creation
static inline void string_destroy(String* string);
static String* read_entire_file(char* file_name, Arena* arena);


/* ========================================================================
    Timer
    ======================================================================== */

#define OS_TIMER_FREQ (uint64_t)1000000

typedef struct
{
    uint64_t start, end;
    //uint64_t min, max, total, next;  //for recursive functions
    String label;
} Time_Block;

#define MAX_TIME_STAMPS 5
typedef struct
{
    uint64_t start, end;
    Time_Block block[MAX_TIME_STAMPS];
    String function_label;
    uint32_t recursive_calls;
    uint8_t block_index;
} Timer;

#define MAX_TIMERS 10
typedef struct
{
    uint64_t start, end;
    Timer timer[MAX_TIMERS];
    uint8_t timer_index;
} Profiler;

static inline void profiler_start(Profiler* p);
static inline void profiler_end(Profiler* p);
static inline void time_block_end(Time_Block* block);
static inline Time_Block* time_block_start(Timer* timer, String label);
static inline void timer_end(Timer* timer);
static inline Timer* timer_start(Profiler* p, String function_name);
static void print_profiling_stats(Profiler* p);
static inline void basic_program_runtime(Time_Block* block);

static inline uint64_t os_timer();
static inline uint64_t get_rdtsc();
static inline uint64_t test_rdtsc_frequency(uint32_t milli_sec);

#ifdef PROFILER
#define TIME_FUNCTION Timer* function_timer = timer_start(&global_profiler, STR(__func__))
#define TIME_FUNCTION_END timer_end(function_timer)
#define TIME_BLOCK(label) Time_Block* block##label = time_block_start(function_timer, STR(#label))
#define TIME_BLOCK_END(label) time_block_end(block##label)
#define PROFILER_START profiler_start(&global_profiler)
#define PROFILER_END profiler_end(&global_profiler)
#define PROFILER_PRINT print_profiling_stats(&global_profiler)
#define TIME_BLOCK_RECURSIVE(label) Time_Block* block##label = time_block_start_recursive(&global_profiler, STR(__func__), STR(#label))

static Profiler global_profiler;
#else
#define TIME_BLOCK_RECURSIVE(label)
#define PROFILER_START Time_Block program_runtime = {get_rdtsc()};
#define PROFILER_END program_runtime.end = get_rdtsc();
#define PROFILER_PRINT basic_program_runtime(&program_runtime);
#define TIME_FUNCTION
#define TIME_FUNCTION_END
#define TIME_BLOCK(label)
#define TIME_BLOCK_END(label)
#endif





#endif // PERFORMACE_AWARE_PROGRAMMING_HELPER_H
#ifdef PERFORMACE_AWARE_PROGRAMMING_HELPER_IMPLEMENTATION


static inline void profiler_start(Profiler* p)
{
    memset(p, 0, sizeof(Profiler));
    p->start = get_rdtsc();
}

static inline void profiler_end(Profiler* p)
{
    p->end = get_rdtsc();
}

static inline Timer* timer_start(Profiler* p, String function_name)
{
    assert((p->timer_index < MAX_TIMERS || p->timer[p->timer_index -1].end == 0)  && "ERROR - max amount of timers have done");
    if (p->timer[p->timer_index -1].end == 0)
    {
        ++p->timer[p->timer_index -1].recursive_calls;
        return NULL;
    }
    p->timer[p->timer_index].function_label = function_name;
    p->timer[p->timer_index++].start = get_rdtsc();

    return &p->timer[p->timer_index -1];
}

static inline void timer_end(Timer* timer)
{
    if (timer == NULL)
        return;
    timer->end = get_rdtsc();
}

static inline bool find_recurseive_block(Timer* t, String* label, uint8_t* k)
{
    for (uint8_t j = t->block_index -1; j >= 0; --j)
    {
        if (are_equal(&t->block[j].label, label))
        {
            *k = j;
            return true;
        }
    }
    return false;
}

// static inline Time_Block* time_block_start_recursive(Profiler* p, String function_name, String label)
// {
//     for (uint8_t i = p->timer_index -1; i >= 0; --i)
//     {
//         if (are_equal(&p->timer[i].function_label, &function_name))
//         {
//             Timer* t = &p->timer[i];
//             uint8_t k = 0;
//             if (t->block_index == 0 || !find_recurseive_block(t, &label, &k))
//                 return time_block_start(t, label);
//             else
//             {
//                 Time_Block* b = &t->block[k];
//                 b->next = get_rdtsc();
//                 return b;
//             }
//         }
//     }
//
//     printf("WARNING - time block for recursive function not found\n");
//     return NULL;
// }

static inline Time_Block* time_block_start(Timer* timer, String label)
{
    if (timer == NULL)
        return NULL;
    assert(timer->block_index < MAX_TIME_STAMPS && "ERROR - max amount of time stamps already taken\n");

    timer->block[timer->block_index].start = get_rdtsc();
    timer->block[timer->block_index++].label = label;

    return &timer->block[timer->block_index -1];
}

static inline void time_block_end(Time_Block* block)
{
    if (block == NULL || block->end > 0)
        return;
    // if (block->end > 0)
    // {
    //     assert(block->next > 0 && "ERROR - next was not precalculated");
    //     uint64_t ticks = get_rdtsc() - block->next;
    //     block->min = ticks > block->min ? block->min : ticks;
    //     block->max = ticks < block->max ? block->min : ticks;
    //     block->total += ticks;
    //     block->next = 0;
    //     return;
    // }
    block->end = get_rdtsc();
}

static inline void basic_program_runtime(Time_Block* block)
{
    const uint64_t rdtsc_freq_est = test_rdtsc_frequency(500);
    const uint64_t total_rdtsc    = block->end - block->start;
    printf("\n\t=========================== Runtime =============================\n");
    printf("\trdtsc freq: %lu (est)\n", rdtsc_freq_est);
    printf("\tprogram start: %lu -> end %lu\n\tprogram total: %lu, 100%% %0.4fsec (est)\n", block->start, block->end, total_rdtsc, (float)total_rdtsc / rdtsc_freq_est);
}

static void print_block_stats(const uint64_t rdtsc_freq_est, Time_Block* block, const uint64_t total_rdtsc, const uint64_t function_rdtsc, const uint32_t recursive)
{
    const uint64_t block_total = block->end - block->start;

    printf("\t\tblock ");
    string_print(&block->label);
    printf(" start: %lu -> end %lu\n\t\tblock total: %lu, %0.2f%% (func.), %0.2f%% (total) %s\n", block->start, block->end, block_total, 100*((float)block_total/function_rdtsc), 100*((float)block_total/total_rdtsc), recursive ? "first recursion" : "");
    if (recursive)
        printf("\t\t*recursive total est: %lu, %0.2f%% (func.), %0.2f%% (total)\n", block_total * recursive, 100*((float)(recursive*block_total)/function_rdtsc), 100*((float)(recursive*block_total)/total_rdtsc));
}

static void print_timer_stats(const uint64_t rdtsc_freq_est, Timer* t, const uint64_t total_rdtsc)
{
    const bool recursive          = t->recursive_calls != 0;
    const uint64_t function_rdtsc = t->end - t->start;

    printf("\tfunc. ");
    string_print(&t->function_label);
    if (recursive)
        printf("[%u]", t->recursive_calls +1);
    printf(": start %lu -> end %lu\n\tfunc. total: %lu, %0.2f%%, %0.4fsec (est)\n", t->start, t->end, function_rdtsc, 100*((float)function_rdtsc/total_rdtsc), (float)function_rdtsc / rdtsc_freq_est);

    for(uint8_t i = 0; i < t->block_index; ++i)
        print_block_stats(rdtsc_freq_est, &t->block[i], total_rdtsc, function_rdtsc, t->recursive_calls);
}

static void print_quick_summary(const uint64_t rdtsc_freq_est, Profiler* p, const uint64_t total_rdtsc)
{
    printf("\n\t=========================== Summary =============================\n");
    for (uint8_t i = 0; i < p->timer_index; ++i)
    {
        Timer* t                      = &p->timer[i];
        const uint64_t function_rdtsc = t->end - t->start;
        const uint32_t recursive      = t->recursive_calls;

        printf("\tfunc. ");
        string_print(&t->function_label);
        if (recursive)
            printf("[%u]", t->recursive_calls +1);
        printf(": %lu, %0.2f%%, %0.4fsec (est)\n", function_rdtsc, 100*((float)function_rdtsc/total_rdtsc), (float)function_rdtsc / rdtsc_freq_est);
        for (uint8_t k = 0; k < t->block_index; ++k)
        {
            printf("\t\tblock ");
            string_print(&t->block[k].label);
            const uint64_t block_total = t->block[k].end - t->block[k].start;
            printf(": %lu, %0.2f%% (func.), %0.2f%% (total) \n", block_total, 100*((float)block_total/function_rdtsc), 100*((float)block_total/total_rdtsc));
            if (recursive)
                printf("\t\t*recursive total est: %lu, %0.2f%% (func.), %0.2f%% (total)\n", block_total * recursive, 100*((float)(recursive*block_total)/function_rdtsc), 100*((float)(recursive*block_total)/total_rdtsc));
        }
    }
    printf("\n\ttotal: %lu, 100%% %0.4fsec (est)\n",total_rdtsc, (float)total_rdtsc / rdtsc_freq_est);

}

static void print_profiling_stats(Profiler* p)
{
    const uint64_t rdtsc_freq_est = test_rdtsc_frequency(500);
    printf("\n\t=========================== Profiler =============================\n");
    printf("\trdtsc freq: %lu (est)\n", rdtsc_freq_est);
    const uint64_t total_rdtsc = p->end - p->start;
    printf("\tprogram start: %lu -> end %lu\n\tprogram total: %lu, 100%% %0.4fsec (est)\n", p->start, p->end, total_rdtsc, (float)total_rdtsc / rdtsc_freq_est);
    for (uint8_t i = 0; i < p->timer_index; ++i)
    {
        printf("\n");
        print_timer_stats(rdtsc_freq_est, &p->timer[i], total_rdtsc);
    }
    print_quick_summary(rdtsc_freq_est, p, total_rdtsc);
}

// static void timer_print_stats(Profiler* p)
// {
//     uint64_t total_ticks   = timer->end - timer->start;
//     uint64_t running_total = timer->start;
//
//     printf("\tStarting block: %lu\n", running_total);
//     for(uint8_t i = 0; i < timer->block; ++i)
//     {
//         uint64_t ticks = timer->timer[i].block - running_total;
//
//         printf("\t");
//         string_print(&timer->timer[i].label);
//         printf(": %lu, %%%0.2f, (%0.4fsec est)\n", ticks, 100*((float)ticks/total_ticks), (float)ticks / rdtsc_freq_est);
//
//         running_total = timer->timer[i].block;
//     }
//     printf("\t");
//     string_print(&timer->end_label);
//     printf(": %lu, %%%0.2f, (%0.6fsec est)\n", timer->end - running_total, 100*((float)(timer->end - running_total)/total_ticks), (float)(timer->end - running_total) / rdtsc_freq_est);
//
//     printf("\tTotal: %lu, (%0.4fsec est)\n", total_ticks, (float)(total_ticks) / rdtsc_freq_est);
// }


static inline uint64_t os_timer()
{
    struct timeval Value;
    gettimeofday(&Value, 0);

    return (OS_TIMER_FREQ*(uint64_t)Value.tv_sec) + (uint64_t)Value.tv_usec;
}
static inline uint64_t get_rdtsc()
{
    return __rdtsc();
}

static uint64_t test_rdtsc_frequency(uint32_t milli_sec)
{
    uint64_t waiting_time = milli_sec * (OS_TIMER_FREQ / 1000);


    uint64_t os_start = os_timer();
    uint64_t rdtsc_start = get_rdtsc();
    uint64_t os_end = 0;
    uint64_t os_elapsed = 0;
    while(os_elapsed < waiting_time)
    {
        os_end = os_timer();
        os_elapsed = os_end - os_start;
    }

    uint64_t rdtsc_end = get_rdtsc();

    uint64_t rdtsc_freq;
    if(os_elapsed)
    {
        rdtsc_freq = OS_TIMER_FREQ * (rdtsc_end - rdtsc_start) / os_elapsed;
    }

    return rdtsc_freq;
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
    {
        s = (String*)arena_alloc(arena, sizeof(String) + (sizeof(uint8_t) *count), NULL);
        s->data = (uint8_t*)(s +1);
    }
    else
    {
        s = (String*)malloc(sizeof(String));
        s->data = (uint8_t*)malloc((sizeof(uint8_t) *count));
    }

    s->count = count;

    return s;
}

static inline void string_destroy(String* string)
{
    free(string->data);
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


#define EARTH_RADIUS_KM 6372.8
// Function to convert degrees to radians
static inline double deg2rad(double deg)
{
    return (deg * M_PI / 180.0);
}

static inline double squared(double x)
{
    return x * x;
}

double haversine_distance(double x1, double y1, double x2, double y2)
{
    // latitudes to radians
    double lat1r = deg2rad(x1);
    double lat2r = deg2rad(x2);

    // Calculate the difference in latitudes and longitudes conerted to radians
    double dLat = deg2rad(x2 - x1);
    double dLon = deg2rad(y2 - y1);

    // Apply the Haversine formula
    double a = squared(sin(dLat / 2)) + cos(lat1r) * cos(lat2r) * squared(sin(dLon / 2));
    double c = 2 * asin(sqrt(a));
    double distance = EARTH_RADIUS_KM * c;

    return distance;
}
double calculate_haversine(double* points, uint64_t count)
{
    double distance = 0;
    for(uint64_t i = 0; i < count *4; i +=4)
        distance += haversine_distance(points[i], points[i +1], points[i +2], points[i +3]);

    return distance / count;
}

#endif // PAP_HELPER_H_IMPLEMENTATION
