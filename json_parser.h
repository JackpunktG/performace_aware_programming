#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "helper_functions.h"

typedef enum
{
    TOKEN_END_OF_STREAM,
    TOKEN_ERROR,

    TOKEN_OPEN_BRACE,
    TOKEN_OPEN_BRACKET,
    TOKEN_CLOSE_BRACE,
    TOKEN_CLOSE_BRACKET,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_SEMI_COLON,
    TOKEN_STRING_LITERAL,
    TOKEN_NUMBER,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,

    TOKEN_COUNT
} Json_Token_Type;


typedef struct Json_Token
{
    Json_Token_Type type;
    String string;
} Json_Token;

typedef struct Json_Element
{
    String label;
    String value;
    struct Json_Element* first_sub_elem;
    struct Json_Element* next_elem;
} Json_Element;

typedef struct Json_Parser
{
    String* source;
    uint64_t cursor;
} Json_Parser;

static inline bool is_white_space(uint8_t* data, uint64_t cursor)
{
    return data[cursor] == '\n' || data[cursor] == '\t' || data[cursor] == ' ' || data[cursor] == '\r';
}

static inline bool is_json_number(String* json, uint64_t cursor)
{
    // check in boundes then if number
    return (json->count > cursor) && (json->data[cursor] >= '0' && json->data[cursor] <= '9');
}

static Json_Token get_json_token(Json_Parser* parser)
{
    Json_Token token = {};
    token.type       = TOKEN_END_OF_STREAM;

    String* json    = parser->source;
    uint64_t cursor = parser->cursor;

    while(is_white_space(json->data, cursor) && in_bounds(json, cursor))
        ++cursor;

    if (in_bounds(json, cursor))
    {
        token.type         = TOKEN_ERROR;
        token.string.data  = json->data + cursor;
        token.string.count = 0;

        uint8_t val = json->data[cursor++];

        switch (val)
        {
        case '{':
            token.type = TOKEN_OPEN_BRACE;
            break;
        case '[':
            token.type = TOKEN_OPEN_BRACKET;
            break;
        case '}':
            token.type = TOKEN_CLOSE_BRACE;
            break;
        case ']':
            token.type = TOKEN_CLOSE_BRACKET;
            break;
        case ',':
            token.type = TOKEN_COMMA;
            break;
        case ':':
            token.type = TOKEN_COLON;
            break;
        case ';':
            token.type = TOKEN_SEMI_COLON;
            break;

        case '"':
        {
            token.type = TOKEN_STRING_LITERAL;

            uint64_t start_index = cursor;

            while (in_bounds(json, cursor) && json->data[cursor] != '"')
            {
                // Skiping escaped quotation marks - \"
                if (in_bounds(json, cursor + 1) && json->data[cursor] == '\\' && json->data[cursor +1] == '"')
                    ++cursor;
                ++cursor;
            }

            token.string.data  = json->data + start_index;
            token.string.count = cursor - start_index;

            if (in_bounds(json, cursor))
                ++cursor;
            break;
        }
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            token.type = TOKEN_NUMBER;

            uint64_t start_index =  cursor -1;


            while (is_json_number(json, cursor))
                ++cursor;

            if (in_bounds(json, cursor) && json->data[cursor] == '.')
            {
                ++cursor;
                while (is_json_number(json, cursor))
                    ++cursor;
            }

            if (in_bounds(json, cursor) && (json->data[cursor] == 'e' || json->data[cursor] == 'E'))
                assert(0 && "ERROR - scientific notation not yet implamented\n");

            token.string.data  = json->data + start_index;
            token.string.count = cursor - start_index;

            break;
        }
        case 'f':
        case 'n':
        case 't':
            assert(0 && "ERROR - keyword not yet implamented\n");
        default:
            assert(0 && "ERROR - unknown symbol when getting token\n");
        }
    }

    parser->cursor = cursor;

    return token;
};

static inline Json_Parser load_json(char* json_file, Arena* arena)
{
    Json_Parser json = {};

    json.cursor = 0;
    json.source = read_entire_file(json_file, arena);

    return json;
}


Json_Element* parse_json_element(Json_Parser* parser, String label, Json_Token token, Arena* arena);
Json_Element* parse_json_list(Json_Parser* parser, Json_Token start_token, Json_Token_Type end_type, bool has_label, Arena* arena)
{
    Json_Element* first_elem = NULL;
    Json_Element* last_elem  = NULL;
    bool error               = false;

    while (in_bounds(parser->source, parser->cursor) && !error)
    {
        String label = {};
        Json_Token next_token = get_json_token(parser);

        if (has_label)
        {
            if (next_token.type == TOKEN_STRING_LITERAL)
            {
                label = next_token.string;

                Json_Token colon = get_json_token(parser);
                if (colon.type == TOKEN_COLON)
                    next_token = get_json_token(parser);
                else
                {
                    fprintf(stderr, "WARNING - Expected colon after field name\n");
                    error = true;
                }
            }
            else if (next_token.type != end_type)
            {
                fprintf(stderr, "WARNING - Unexpected token in JSON. Postion: %lu\n", parser->cursor);
                error = true;
            }
        }

        Json_Element* elem = parse_json_element(parser, label, next_token, arena);
        if (elem)
        {
            if (first_elem == NULL)
                first_elem = elem;
            else
                last_elem->next_elem = elem;
            last_elem = elem;
        }
        else if (next_token.type == end_type)
            break;
        else
        {
            fprintf(stderr, "WARNING - Unexpected token in JSON. Postion: %lu\n", parser->cursor);
            error = true;
        }

        Json_Token comma = get_json_token(parser);
        if (comma.type == end_type)
            break;
        else if (comma.type != TOKEN_COMMA)
        {
            fprintf(stderr, "WARNING - Unexpected token in JSON. Postion: %lu (Expected comma)\n", parser->cursor);
            error = true;
        }
    }

    return first_elem;
}

Json_Element* parse_json_element(Json_Parser* parser, String label, Json_Token token, Arena* arena)
{
    bool item = true;

    Json_Element *sub_elem = NULL;
    switch (token.type)
    {
    case TOKEN_OPEN_BRACKET:
        sub_elem = parse_json_list(parser, token, TOKEN_CLOSE_BRACKET, false, arena);
        break;
    case TOKEN_OPEN_BRACE:
        sub_elem = parse_json_list(parser, token, TOKEN_CLOSE_BRACE, true, arena);
        break;
    case TOKEN_STRING_LITERAL:
    case TOKEN_TRUE:
    case TOKEN_FALSE:
    case TOKEN_NULL:
    case TOKEN_NUMBER:
        break;
    default:
        item = false;
        break;
    }

    Json_Element* elem = NULL;

    if (item)
    {
        if (arena != NULL)
            elem = (Json_Element*)arena_alloc(arena, sizeof(Json_Element), NULL);
        else
            elem = (Json_Element*)malloc(sizeof(Json_Element));

        elem->label          = label;
        elem->value          = token.string;
        elem->first_sub_elem = sub_elem;
        elem->next_elem      = NULL;

        DEBUG_PRINT(string_println(&elem->label));
        DEBUG_PRINT(string_println(&elem->value));
    }

    return elem;
}

static void json_free(Json_Element* json)
{
    if (json->first_sub_elem != NULL)
        json_free(json->first_sub_elem);
    if (json->next_elem != NULL)
        json_free(json->next_elem);
    free(json);
}

static void json_destroy(Json_Element* json, String* json_string, Arena* arena)
{
    if (arena != NULL)
    {
        arena_destroy(arena);
        arena = NULL;
        json  = NULL;

        return;
    }
    json_free(json);
    string_destroy(json_string);

    json = NULL;
}

typedef enum
{
    JVT_INT,
    JVT_DOUBLE
} Json_Value_Type;
void* get_json_value(Json_Element* json, String label, Json_Value_Type type, Arena* arena)
{
    Json_Element* next = json;
    void* result = malloc(sizeof(type) * 100);
    while(next != NULL)
    {
        if (are_equal(&next->label, &label))
        {
            printf("SAMR)\n");
            break;
        }

    }
    return NULL;
}

static Json_Element* get_json_element(Json_Element* json, String label)
{
    for (Json_Element* search = json->first_sub_elem; search; search = search->next_elem)
    {
        if (are_equal(&search->label, &label))
            return search;
    }
    return NULL;
}

static Json_Element* parse_json(char* json_file, String** parser_out,  Arena* arena)
{
    Json_Parser parser = load_json(json_file, arena);
    String label = {0};

    // to free the Json File String
    *parser_out = parser.source;

    return parse_json_element(&parser, label, get_json_token(&parser), arena);
}

static void json_nodes_print(Json_Element* elem)
{
    if (elem->first_sub_elem != NULL)
    {
        if (elem->label.count > 0)
        {
            string_print(&elem->label);
            printf(": ");
        }
        if (elem->value.count > 0)
            string_print(&elem->value);
        if (elem->value.count > 0 || elem->label.count > 0)
            printf("\n");


        json_nodes_print(elem->first_sub_elem);

        if(elem->next_elem != NULL)
            json_nodes_print(elem->next_elem);
    }
    else
    {
        Json_Element* next = elem;
        bool first         = true;
        while(next != NULL)
        {
            if (!first)
                printf(", ");
            string_print(&next->label);
            printf(": ");
            string_print(&next->value);

            first = false;
            if (next->first_sub_elem != NULL)
            {
                printf("\n");
                json_nodes_print(next->first_sub_elem);
            }
            next = next->next_elem;
        }
        printf("\n");
    }
}
