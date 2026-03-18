#define PERFORMACE_AWARE_PROGRAMMING_HELPER_IMPLEMENTATION
#include "json_parser.h"



/* Json Values create */
#define MIN_X_LAT -90.0
#define MAX_X_LAT 90.0
#define MIN_Y_LON -180.0
#define MAX_Y_LON 180.0

static double rand_double(double min, double max)
{
    return min + ((double)rand() / (double)RAND_MAX * (max - min));
}

double* create_points_json(uint64_t count)
{
    srand(time(NULL));

    printf("{\n\"points_on_earth\":[");

    double* points = (double*)malloc(sizeof(double) *count *4);

    uint64_t total = 0;
    for(uint64_t i = 0; i < count -1; ++i)
    {
        points[total] = rand_double(MIN_X_LAT, MAX_X_LAT);
        points[total +1] = rand_double(MIN_Y_LON, MAX_Y_LON);
        points[total +2] = rand_double(MIN_X_LAT, MAX_X_LAT);
        points[total +3] =rand_double(MIN_Y_LON, MAX_Y_LON);

        printf("\n{\"x1\":%0.12lf, \"y1\":%0.12lf, \"x2\":%0.12lf, \"y2\":%0.12lf},", points[total], points[total +1], points[total +2], points[total +3]);
        total += 4;
    }

    points[total] = rand_double(MIN_X_LAT, MAX_X_LAT);
    points[total +1] = rand_double(MIN_Y_LON, MAX_Y_LON);
    points[total +2] = rand_double(MIN_X_LAT, MAX_X_LAT);
    points[total +3] =rand_double(MIN_Y_LON, MAX_Y_LON);

    printf("\n{\"x1\":%0.12lf, \"y1\":%0.12lf, \"x2\":%0.12lf, \"y2\":%0.12lf}", points[total], points[total +1], points[total +2], points[total +3]);

    printf("\n]");

    return points;
}

typedef struct
{
    float x, y, r;
} Cluster_Point;

double* create_cluster_points(uint8_t clusters, uint64_t count)
{
    srand(time(NULL));

    Cluster_Point point[clusters] = {};

    for (uint8_t i = 0; i < clusters; ++i)
    {
        point[i].x = (float)rand_double(MIN_X_LAT, MAX_X_LAT);
        point[i].y = (float)rand_double(MIN_Y_LON, MAX_Y_LON);
        point[i].r = (float)rand_double(i % 2 == 0 ? 1.5 : 100, i % 2 == 0 ? 100 : 500);
        DEBUG_PRINT(printf("Cluster %d - x: %0.2f, y: %0.2f, r: %0.2f\n", i, point[i].x, point[i].y, point[i].r))
    }

    double* points = (double*)malloc(sizeof(double) * count * 4);

    printf("{\n\"points_on_earth\":[");

    uint8_t k = 0;
    uint64_t total = 0;
    uint64_t points_per_cluster = count / clusters;
    for(uint64_t i = 0; i < count -1; ++i)
    {
        points[total] = rand_double(point[k].x, point[k].x + point[k].r);
        points[total +1] = rand_double(point[k].y, point[k].y + point[k].r);
        points[total +2] = rand_double(point[k].x, point[k].x + point[k].r);
        points[total +3] = rand_double(point[k].y, point[k].y + point[k].r);
        printf("\n{\"x1\":%0.12lf, \"y1\":%0.12lf, \"x2\":%0.12lf, \"y2\":%0.12lf},", points[total], points[total +1], points[total +2], points[total +3]);
        total += 4;

        if ((i +1) % points_per_cluster == 0 && k < clusters -1)
            ++k;
    }

    points[total] = rand_double(point[k].x, point[k].x + point[k].r);
    points[total +1] = rand_double(point[k].y, point[k].y + point[k].r);
    points[total +2] = rand_double(point[k].x, point[k].x + point[k].r);
    points[total +3] = rand_double(point[k].y, point[k].y + point[k].r);
    printf("\n{\"x1\":%0.12lf, \"y1\":%0.12lf, \"x2\":%0.12lf, \"y2\":%0.12lf}", points[total], points[total +1], points[total +2], points[total +3]);

    printf("\n]");

    return points;
}

double* get_points_from_json(Json_Element* json, uint64_t* count_out, Arena* arena)
{
    Json_Element* points_on_earth = get_json_element(json, STR("points_on_earth"));
    Json_Element* next            = points_on_earth->first_sub_elem;

    uint64_t count = 0;
    while (next != NULL)
    {
        ++count;
        next = next->next_elem;
    }

    double* points = NULL;
    if (arena != NULL)
        points = (double*)arena_alloc(arena, sizeof(double) * count *4, NULL);
    else
        points = (double*)malloc(sizeof(double) * count *4);

    uint64_t test =0;
    next = points_on_earth->first_sub_elem;
    for (uint64_t i = 0; i < count; ++i)
    {
        Json_Element* point_node = next->first_sub_elem;
        while (point_node != NULL)
        {
            ++test;
            // get the vaules
            point_node = point_node->next_elem;
        }
        next = next->next_elem;
    }



    printf("test: %lu, count: %lu\n", test, count);
    return points;
}


enum
{
    FLAG_GENERATE     = (1<<0),
    FLAG_CLUSTER      = (1<<1),
    FLAG_CALCULATE    = (1<<2),
    FLAG_JSON_BASIC   = (1<<3),
    FLAG_NO_REAULT    = (1<<4),
    FLAG_ARENA_MEMORY = (1<<5),
};

int main(int argc, char* argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "Usage: -g [amount]\n");
        fprintf(stderr, "Usage: -g cluster [cluster count] [amount]\n");
        fprintf(stderr, "Usage: -c \n");
        return 1;
    }
    uint32_t flags  = 0;
    char* filename  = NULL;
    char* count_str = NULL;
    Arena* arena    = NULL;

    for (uint8_t i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--generate") == 0)
            flags |= FLAG_GENERATE;
        else if (strcmp(argv[i], "cluster") == 0)
            flags |= FLAG_CLUSTER;
        else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--calculate") == 0)
            flags |= FLAG_CALCULATE;
        else if (strcmp(argv[i], "json_basic") == 0)
            flags |= FLAG_JSON_BASIC;
        else if (strcmp(argv[i], "no_result") == 0)
            flags |= FLAG_NO_REAULT;
        else if (strcmp(argv[i], "--arena") == 0 || strcmp(argv[i], "-a") == 0)
            flags |= FLAG_ARENA_MEMORY;
        else
        {
            // assume the first unknown is filename - second count_str
            if (filename == NULL)
                filename = argv[i];
            else if (count_str == NULL)
                count_str = argv[i];
            else
                fprintf(stderr, "WARNING - arg: %s invalid\n", argv[i]);
        }
    }

    if (flags & FLAG_GENERATE)
    {
        assert(filename != NULL && "ERROR - no amount given\n");

        double* points = NULL;
        uint64_t count = atoi(filename);

        if (flags & FLAG_CLUSTER)
        {
            assert(count_str != NULL && "ERROR - no cluster amount given\n");
            uint64_t cluster_count = atoi(count_str);
            points = create_cluster_points(count < cluster_count ? count : cluster_count, count < cluster_count ? cluster_count : count);
        }
        else
            points = create_points_json(count);

        if (!(flags & FLAG_NO_REAULT))
            printf(",\n\"result\":%0.12lf\n}\n", calculate_haversine(points, count));
        else
            printf("\n}\n");

        free(points);
    }
    else if (flags & FLAG_CALCULATE)
    {
        assert(filename != NULL && "ERROR - no filename given\n");

        Json_Element* json = NULL;

        if (flags & FLAG_ARENA_MEMORY)
            arena = (Arena*)arena_init(ARENA_BLOCK_SIZE, 8);


        String* json_string = NULL;
        json = parse_json(filename, &json_string, arena);
        if (json)
        {
            json_nodes_print(json);
            uint64_t count;
            get_points_from_json(json, &count, arena);

            json_destroy(json, json_string, arena);
        }

    }
    else
        fprintf(stderr, "ERROR - args unknown\n");

    return 0;
}

