#ifndef ARG
#define ARG(flag, arg_string, abbr, ...) {(uint8_t*)(arg_string), sizeof(arg_string) -1, (abbr), __VA_ARGS__},
#define ARG_INFO(info_string)  sizeof(info_string) -1, (uint8_t*)(info_string),
#define ARG_ABBR_NO 0
#endif

/* ===============================================================
   Add your args here, with the syntax: *optional
   ARG(flag, "trigger_input_string", 'char' for single char trigger input(abbr.) / 0 for not used, *ARG_INFO("flag help page info"))
   the below and examples, replace them with your flags of choice
   ==============================================================*/
ARG(FLAG_GENERATE, "--gen", 'g', ARG_INFO("used to generate the haversine points in JSON format. type in the amount of points to be generated"
                                    "\n\tusage: -g [amount]"))
ARG(FLAG_CLUSTER, "--cluster", ARG_ABBR_NO, ARG_INFO("generate points within a given amount of clusters, must be combined with FLAG_GENERATE\n"
        "\tusage: -g cluster [cluster count] [amount]"))
ARG(FLAG_NO_RESULT, "--no_result", ARG_ABBR_NO, ARG_INFO("used in combination with the generation flags to produce a JSON file without the result"))
ARG(FLAG_CALCULATE, "--calc", 'c', ARG_INFO("calculate haversine problem from given JSON "
                                            "file\n\tusage: -c [file.json]"))
ARG(FLAG_ARENA_MEMORY, "--arena", 'a', ARG_INFO("program used an arena memory style of memory management"))

#undef ARG_ABBR_NO
#undef ARG_INFO
#undef ARG

// before
// 24276 - cold
// ~5500 - hot 
// with easy args
// 35200 - cold
// ~15000 - hot
