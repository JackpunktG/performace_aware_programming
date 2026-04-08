#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <sys/resource.h>
#include <stdint.h>
#include <unistd.h>
#include "helper_functions.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("ERROR: no args - usage: [pagecount]\n");
        return 1;
    }

    const uint64_t page_kb = sysconf(_SC_PAGE_SIZE);
    const uint64_t page_count = atoi(argv[1]);
    const uint64_t total_size = page_count * page_kb;

    printf("pages_written,faults\n");
    for (uint64_t page = 0; page < page_count; ++page)
    {
        uint64_t before = get_page_faults();
        uint8_t* buffer = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
        if (!buffer)
            assert(0 && "ERROR: memory allocation failed\n");
        for (uint64_t i = 0; i < page * page_kb; ++i)
            buffer[i] = (uint8_t)i;

        uint64_t after = get_page_faults();
        munmap(buffer, total_size);
        printf("%lu,%lu\n", page, after - before);
    }
    return 0;
}
