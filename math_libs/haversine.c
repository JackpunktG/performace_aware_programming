#define PERFORMACE_AWARE_PROGRAMMING_HELPER_IMPLEMENTATION
#include "../json_parser.h"

#include "../easy_args.h"
#include "math_lib.h"
#define P_val(value, print_symbol) printf("Value of "#value": "#print_symbol"\n", value)
#include <stdint.h>

#define true 1
#define false 0

/* Json Values create */
#define MIN_X_LAT -90.0
#define MAX_X_LAT 90.0
#define MIN_Y_LON -180.0
#define MAX_Y_LON 180.0

static f64 rand_double(f64 min, f64 max)
{
    return min + ((f64)rand() / (f64)RAND_MAX * (max - min));
}

f64* create_points_json(uint64_t count)
{
    srand(time(NULL));

    printf("{\n\"points_on_earth\":[");

    f64* points = (f64*)malloc(sizeof(f64) *count *4);

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

f64* create_cluster_points(uint8_t clusters, uint64_t count)
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

    f64* points = (f64*)malloc(sizeof(f64) * count * 4);

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

typedef struct
{
    Json_Element* json;
    String* json_name;
    uint64_t json_size;

    f64* points;
    uint64_t count;
    f64 truth_result;

    bool error;
    Arena* arena;
} Haversine_Unit;

#define MB (1024*1024)

Haversine_Unit haversine_unit_init(char* file_name, Arena* arena)
{
    Haversine_Unit unit = {0};
    unit.arena = arena;

    unit.json      = parse_json(file_name, &unit.json_name, arena);
    unit.json_size = (sizeof(uint8_t) * unit.json_name->count);

    printf("json test size = %0.4fmb\n", (f64)unit.json_size / MB);

    if (!unit.json)
    {
        unit.error = true;
        return unit;
    }


    unit.points       = get_points_from_json(unit.json, &unit.count, arena);
    unit.truth_result = get_double_json_value(get_json_element(unit.json, STR("result")));

    return unit;
}

static void destory_haversine(Haversine_Unit* unit)
{
    if (unit->arena)
        arena_destroy(unit->arena);
    else
    {
        if (unit->json)
            json_destroy(unit->json, unit->json_name);
        if (unit->points)
            free(unit->points);
    }
}

void test_json_haversine(Haversine_Unit* unit)
{
    HARNESS_BEGIN(Haversine_calculations, unit->json_size);

    f64 result = calculate_haversine(unit->points, unit->count);

    HARNESS_END(Haversine_calculations);
    if (almost_equal((float)result, (float)unit->truth_result))
        printf("result is the same!! Haversine avg: %0.10f\n", (float)result);
    else
        printf("results differ... calculated: %0.10f, json truth: %0.10f\n", (float)result, (float)unit->truth_result);

}

static inline void bigger_smaller(f64* val_s, f64* val_b, f64 val)
{
    if (val < *val_s)
        *val_s = val;

    if (val > *val_b)
        *val_b = val;
}

void broken_down_haversine(Haversine_Unit* unit)
{
    f64 distance = 0;
    f64 cos_min = 0;
    f64 cos_max = 0;
    f64 sin_min = 0;
    f64 sin_max = 0;
    f64 asin_min = 0;
    f64 asin_max = 0;
    f64 sqrt_min = 0;
    f64 sqrt_max = 0;

    for(uint64_t i = 0; i < unit->count *4; i +=4)
    {
        f64 x1 = unit->points[i];
        f64 y1 = unit->points[i+1];
        f64 x2 = unit->points[i+2];
        f64 y2 = unit->points[i+3];

        f64 lat1r = deg2rad(x1);
        f64 lat2r = deg2rad(x2);
        f64 dLat = deg2rad(x2 - x1);
        f64 dLon = deg2rad(y2 - y1);

        if (i == 0)
        {
            cos_min = cos_max = lat1r;
            sin_min = sin_max = dLon /2;
        }

        bigger_smaller(&cos_min, &cos_max, lat1r);
        bigger_smaller(&cos_min, &cos_max, lat2r);
        bigger_smaller(&sin_min, &sin_max, dLon /2);
        bigger_smaller(&sin_min, &sin_max, dLat /2);
        f64 a = squared(sin(dLat / 2)) + cos(lat1r) * cos(lat2r) * squared(sin(dLon / 2));

        if (i == 0)
            sqrt_min = sqrt_max = a;

        bigger_smaller(&sqrt_min, &sqrt_max, a);
        a = sqrt(a);

        if (i == 0)
            asin_min = asin_max = a;

        bigger_smaller(&asin_min, &asin_max, a);
        f64 c = 2 * asin(a);

        distance += EARTH_RADIUS_KM * c;
    }
    if (almost_equal((float)distance / unit->count, (float)unit->truth_result))
        printf("result is the same!! Haversine avg: %0.10f\n", (float)distance / unit->count);
    else
        printf("results differ... calculated: %0.10f, json truth: %0.10f\n", (float)distance / unit->count, (float)unit->truth_result);


    printf("cos (%f - %f)\n", cos_min, cos_max);
    printf("sin (%f - %f)\n", sin_min, sin_max);
    printf("asin (%f - %f)\n", asin_min, asin_max);
    printf("sqrt (%f - %f)\n", sqrt_min, sqrt_max);
}



int main(int argc, char* argv[])
{
    PROFILER_START;
    Program_Flags flags  = {0};
    Arena* arena         = NULL;
    test_math_h();
    return 0;

    if (!set_flags(&flags, argc, argv, EXPECTING_UNKNOWN))
        return 1;

    if (is_flag_set(&flags, FLAG_GENERATE))
    {
        assert(flags.unknown_arg_count > 0 && "ERROR - no amount given\n");

        f64* points         = NULL;
        uint64_t count         = atoi(flags.unknown_arg[0]);
        uint64_t cluster_count = 0;

        if (is_flag_set(&flags, FLAG_CLUSTER))
        {
            assert(flags.unknown_arg_count > 1 && "ERROR - no cluster amount given\n");
            cluster_count = atoi(flags.unknown_arg[1]);
            points = create_cluster_points(count < cluster_count ? count : cluster_count, count < cluster_count ? cluster_count : count);
        }
        else
            points = create_points_json(count);

        if (!(is_flag_set(&flags, FLAG_NO_RESULT)))
            printf(",\n\"result\":%0.12lf\n}\n", calculate_haversine(points, cluster_count > count ? cluster_count : count));
        else
            printf("\n}\n");

        free(points);
    }
    else if (is_flag_set(&flags, FLAG_CALCULATE))
    {
        assert(flags.unknown_arg_count > 0 && "ERROR - no amount given\n");

        if (is_flag_set(&flags, FLAG_ARENA_MEMORY))
            arena = (Arena*)arena_init(ARENA_BLOCK_SIZE, 8);

        Haversine_Unit unit = haversine_unit_init(flags.unknown_arg[0], arena);


        if (!unit.error)
        {
            if (almost_equal(calculate_haversine(unit.points, unit.count), unit.truth_result))
                printf("truth_works\n");
            broken_down_haversine(&unit);
        }

        destory_haversine(&unit);
    }
    else
        fprintf(stderr, "ERROR - args unknown\n");

    PROFILER_PRINT;
    PROFILER_END;

    return 0;
}




