#define PERFORMACE_AWARE_PROGRAMMING_HELPER_IMPLEMENTATION
#include "json_parser.h"


int main(int argc, char* argv[])
{
    Json_Element* json = NULL;
    if (argc > 1)
    {
        json = parse_json(argv[1], NULL);
        if (json)
        {
            json_nodes_print(json);
        }
    }

    return 0;
}

