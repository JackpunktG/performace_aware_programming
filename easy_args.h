#ifndef EASY_ARGS_H
#define EASY_ARGS_H

#if defined(__GNUC__) || defined(__clang__)
#define DISABLE_WARNINGS \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"")

#define ENABLE_WARNINGS \
    _Pragma("GCC diagnostic pop")
#else
#define DISABLE_WARNINGS
#define ENABLE_WARNINGS
#endif

#ifdef ARG_INFO
#define ARG_PRINT(...) printf(__VA_ARGS__)
#else
#define ARG_PRINT(...)
#endif

#define ARG_array_count(arr) (sizeof(arr) / sizeof(arr[0]))

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef enum : uint8_t
{
#define ARG(flag, ...) flag,
#include "easy_args.inc"
    FLAG_COUNT
} Flag_Options;

static char* flag_string(Flag_Options flag)
{
    switch(flag)
    {
#define ARG(flag, ...) case flag: return #flag;
#include "easy_args.inc"
    default:
        assert(0 && "ERROR - failed to return flag string\n");
    }
}

typedef struct
{
    uint8_t* string_data;
    uint8_t string_count;
    char short_arg;
    uint16_t info_count;
    /* 4-byte hole */
    uint8_t* info_data;
} Arg_Table;

#define MAX_ARGS         64
#define MAX_UNKNOWN_ARGS 5

typedef struct
{
    uint64_t flags;
    char* unknown_arg[MAX_UNKNOWN_ARGS];
    uint8_t unknown_arg_count;
    uint8_t ready;
    /* 6-byte hole */
} Program_Flags;


DISABLE_WARNINGS
const Arg_Table arg_table[] =
{
#include "easy_args.inc"
};
ENABLE_WARNINGS

#define ARG_COLOR_RESET   "\033[0m"
#define ARG_COLOR_GREEN   "\033[0;32m"
#define ARG_COLOR_YELLOW  "\033[0;33m"
#define ARG_COLOR_BLUE    "\033[0;34m"
#define ARG_COLOR_BOLD_RED     "\033[1;31m"
#define ARG_COLOR_BOLD_MAGENTA "\033[1;35m"
#define ARG_FALSE 0
#define ARG_TRUE  1

static inline uint8_t arg_string_match(const char* arg, const Arg_Table arg_option)
{
    if (strlen(arg) != arg_option.string_count)
        return ARG_FALSE;

    uint8_t i = 0;
    while (arg[i] != '\0')
    {
        if  (arg[i] != arg_option.string_data[i])
            return ARG_FALSE;
        ++i;
    }
    return ARG_TRUE;
}

static inline uint8_t arg_help_defined()
{
    for (uint8_t i = 0; i < ARG_array_count(arg_table); ++i)
    {
        if (arg_string_match("--help", arg_table[i]))
            return ARG_TRUE;
    }
    return ARG_FALSE;
}

#define ARG_LINE_LENGTH      20
#define ARG_INFO_LINE_LENGTH 64
void print_args_help()
{
    printf(ARG_COLOR_BOLD_MAGENTA "\nArgs help page:\n" ARG_COLOR_RESET);
    printf("Short args with '-' are combinable i.e: -xyz\n\n");

    for (uint8_t i = 0; i < ARG_array_count(arg_table); ++i)
    {
        char info[ARG_INFO_LINE_LENGTH] = {0};
        char arg[ARG_LINE_LENGTH]       = {0};
        uint16_t printed                = ARG_INFO_LINE_LENGTH -1;

        snprintf(arg, ARG_LINE_LENGTH, "%c%c %.*s", arg_table[i].short_arg ? '-' : ' ',  arg_table[i].short_arg ? arg_table[i].short_arg : ' ', arg_table[i].string_count, arg_table[i].string_data);
        if (arg_table[i].info_count > 0)
        {
            // Testing more newline in the first row
            uint8_t k = 0;
            while (k < printed && arg_table[i].info_data[k] != '\n')
                ++k;
            if (k == printed)
            {
                while (printed > 0 && arg_table[i].info_data[printed] != '\t' && arg_table[i].info_data[printed] != ' ')
                    --printed;
            }
            else
                printed = k;

            snprintf(info, ARG_INFO_LINE_LENGTH, "%.*s", ++printed, arg_table[i].info_data);
            info[printed-1] = ' ';
        }
        else
            snprintf(info, ARG_INFO_LINE_LENGTH, "%s (flag)", flag_string((Flag_Options)i));

        printf("\t%-*s  %.*s\n", ARG_LINE_LENGTH, arg, printed, info);


        if (printed < arg_table[i].info_count)
        {
            int remaining    = arg_table[i].info_count - printed;
            while (remaining > 0)
            {
                // Trim staring white space
                while (arg_table[i].info_data[printed] == '\n' || arg_table[i].info_data[printed] == ' ')
                {
                    ++printed;
                    --remaining;
                }

                snprintf(info, ARG_INFO_LINE_LENGTH, "%.*s", remaining, arg_table[i].info_data + printed);
                uint8_t to_print = 0;
                while (to_print < remaining && to_print < ARG_INFO_LINE_LENGTH)
                {
                    if (info[to_print] == '\n')
                        break;
                    ++to_print;
                }
                if (to_print == 48)
                {
                    --to_print;
                    while (to_print > 0)
                    {
                        if (info[to_print] == '\t' || info[to_print] == ' ')
                            break;
                        --to_print;
                    }
                }

                info[to_print] = ' ';
                printf("\t%-*s  %.*s\n", ARG_LINE_LENGTH, "", ++to_print, info);
                remaining -= to_print;
                printed   += to_print;
            }
        }
        printf("\n");
    }
}

static inline uint8_t arg_table_check()
{
    if (ARG_array_count(arg_table) > MAX_ARGS)
    {
        printf(ARG_COLOR_BOLD_RED "ERROR " ARG_COLOR_RESET "- too many args set, max is 64\n");
        return ARG_FALSE;
    }
    for (uint8_t i = 0; i < ARG_array_count(arg_table); ++i)
    {
        if(arg_table[i].string_data[0] == '-' && arg_table[i].string_data[1] == '-')
            continue;
        else
        {
            printf(ARG_COLOR_BOLD_RED "ERROR " ARG_COLOR_RESET "- user input for flag %s does not start with \"--\" currently: " ARG_COLOR_BOLD_MAGENTA "%.*s\n" ARG_COLOR_RESET, flag_string((Flag_Options)i), arg_table[i].string_count, arg_table[i].string_data);
            return ARG_FALSE;
        }
    }
    return ARG_TRUE;
}

#define EXPECTING_UNKNOWN 1
#define NO_UNKNOWN        0
uint8_t set_flags(Program_Flags* program_flags, const int argc, char* argv[], const uint8_t expecting_unknown)
{
    Program_Flags flags = {0};

    if (argc < 2)
    {
        printf("No args set --help for help page\n");
        return ARG_FALSE;
    }
    else if (!arg_table_check())
        return ARG_FALSE;
    else if (strcmp(argv[1], "--help") == 0 && !arg_help_defined())
    {
        print_args_help();
        return ARG_FALSE;
    }

    uint8_t unknown[MAX_ARGS] = {0};
    uint8_t unknown_count     = 0;

    for (uint8_t i = 1; i < argc; ++i)
    {
        uint8_t found = 0;
        if (argv[i][0] == '-' && argv[i][1] != '-')
        {
            uint8_t index = 1;
            char char_test = argv[i][index];
            while(char_test != '\0')
            {
                uint8_t correct_input = 0;
                for (uint8_t k = 0; k < ARG_array_count(arg_table); ++k)
                {
                    if (arg_table[k].short_arg && char_test == arg_table[k].short_arg)
                    {
                        correct_input = 1;
                        flags.flags |= (1<<k);
                        ARG_PRINT(ARG_COLOR_GREEN "Flag %.*s set with arg: -%c\n" ARG_COLOR_RESET, arg_table[k].flag_count, arg_table[k].flag_data, char_test);
                        break;
                    }
                }
                if (!correct_input)
                    printf("WARNING: arg '%s' contains invaild flag:" ARG_COLOR_BOLD_RED " %c\n" ARG_COLOR_RESET, argv[i], char_test);
                char_test = argv[i][++index];
            }
            found = 1;

        }
        else if (argv[i][0] == '-' && argv[i][1] == '-')
        {
            uint8_t correct_input = 0;
            for (uint8_t k = 0; k < ARG_array_count(arg_table); ++k)
            {
                if (arg_string_match(argv[i], arg_table[k]))
                {
                    correct_input = 1;
                    ARG_PRINT(ARG_COLOR_GREEN "Flag %.*s set with arg: -%s\n" ARG_COLOR_RESET, arg_table[k].flag_count, arg_table[k].flag_data, argv[i]);
                    flags.flags |= (1<<k);

                    break;
                }
            }
            if (!correct_input)
                printf("WARNING: arg " ARG_COLOR_BOLD_RED "'%s'" ARG_COLOR_RESET " is an invaild flag\n", argv[i]);
            found = 1;
        }
        if (!found)
        {
            if (unknown_count >= MAX_ARGS)
            {
                printf("WARNING: Max amount of unknown args reached\n");
                assert(0);
            }
            else
                unknown[unknown_count++] = i;
            if (!expecting_unknown)
                printf("WARNING: arg " ARG_COLOR_BOLD_RED "'%s'" ARG_COLOR_RESET " is an invaild flag\n", argv[i]);

        }
    }

    if (unknown_count > 0 && expecting_unknown)
    {
        for (uint8_t i = 0; i < unknown_count; ++i)
        {
            if (i >= MAX_UNKNOWN_ARGS)
            {
                printf("WARNING: Max amount of unknown args reached, only the first %d unknown args are kept track of\n", MAX_UNKNOWN_ARGS);
                break;
            }
            ARG_PRINT("Unknown arg " ARG_COLOR_BLUE "'%s'" ARG_COLOR_RESET " is ready for use at index: " ARG_COLOR_BLUE "%hhu\n" ARG_COLOR_RESET, argv[unknown[i]], i);
            flags.unknown_arg[i] = argv[unknown[i]];
        }
        flags.unknown_arg_count = unknown_count > MAX_UNKNOWN_ARGS ? MAX_UNKNOWN_ARGS : unknown_count;
    }

    flags.ready = 1;
    *program_flags = flags;

    return ARG_TRUE;
}


void print_program_flags(Program_Flags* flags)
{
    printf("\nProgram_Flags:\n");
    for (uint8_t i = 0; i < ARG_array_count(arg_table); ++i)
    {
        if (flags->flags & (1<<i))
            printf(ARG_COLOR_GREEN"\t%s\n" ARG_COLOR_RESET, flag_string((Flag_Options)i));
        else
            printf(ARG_COLOR_YELLOW"\t%s\n" ARG_COLOR_RESET, flag_string((Flag_Options)i));
    }
    printf("\nuint64_t flags:\n\t");
    for (int i = 63; i >= 0; --i)
    {
        if (flags->flags & (1ULL<<i))
            printf("1");
        else
            printf("0");
    }


    if (flags->unknown_arg_count > 0)
    {
        printf("\n\nunknown set:\n");
        for (uint8_t i = 0; i < flags->unknown_arg_count; ++i)
            printf(ARG_COLOR_BLUE"\t%s\n", flags->unknown_arg[i]);
        printf(ARG_COLOR_RESET);
    }
    else
        printf("\n\nno unknown flags\n");
    printf("\n");
}

void print_args_table()
{
    for (uint8_t i = 0; i < ARG_array_count(arg_table); ++i)
    {
        printf("\tFlag: %-15s set with: %.*s", flag_string((Flag_Options)i), arg_table[i].string_count, arg_table[i].string_data);
        if (arg_table[i].short_arg)
            printf(", or -%c", arg_table[i].short_arg);
        printf("\n");

    }
}

static inline uint8_t is_flag_set(Program_Flags* flags, Flag_Options flag)
{
    return (flags->flags & (1ULL<<flag));
}

#endif // EASY_ARGS_H
