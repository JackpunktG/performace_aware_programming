#include <stdio.h>
#include <math.h>
#define PAP_HELPER_IMPLEMENTATION
#include "pap_helper.h"
#include <string.h>

#define EARTH_RADIUS_KM 6371.0

// Function to convert degrees to radians
double deg2rad(double deg)
{
    return (deg * M_PI / 180.0);
}

double haversine_distance(double lat1d, double lon1d, double lat2d, double lon2d)
{
    // Convert all degrees to radians
    double lat1r = deg2rad(lat1d);
    double lon1r = deg2rad(lon1d);
    double lat2r = deg2rad(lat2d);
    double lon2r = deg2rad(lon2d);

    // Calculate the difference in latitudes and longitudes
    double dLat = lat2r - lat1r;
    double dLon = lon2r - lon1r;

    // Apply the Haversine formula
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1r) * cos(lat2r) *
               sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double distance = EARTH_RADIUS_KM * c;

    return distance;
}


/* Json Values create */
#define MIN_LAT -90.0
#define MAX_LAT 90.0
#define MIN_LON -180.0
#define MAX_LON 180.0

static double rand_double(double min, double max)
{
    return min + ((double)rand() / (double)RAND_MAX * (max - min));
}


void create_points_json(uint32_t count)
{
    srand(time(NULL));

    printf("{\n\"points_on_earth\":[");

    for(uint32_t i = 0; i < count; ++i)
    {
        printf("\n{\"x1\":%0.12lf, \"y1\":%0.12lf, \"x2\":%0.12lf, \"y2\":%0.12lf},",
               rand_double(MIN_LAT, MAX_LAT), rand_double(MIN_LON, MAX_LON),
               rand_double(MIN_LAT, MAX_LAT), rand_double(MIN_LON, MAX_LON));
    }
    printf("\n]\n}\n");
}

/* ===================================================================
    Prologue week, my first basic implementation of the problem
    with no optimisations and course specific learning
    =================================================================*/
void calculate_avg_prologue(const char* input_stream, uint32_t pts_count)
{
    Timer t1, t2;
    start_timer(&t1);
    double* points_on_earth = (double*)malloc(sizeof(double) * pts_count * 4);
    uint32_t count = 0;
    FILE* input = strcmp(input_stream, "stdin") == 0 ? stdin : fopen(input_stream, "r");
    char buffer[128] = {0};
    uint32_t line = 1;
    while (fgets(buffer, sizeof(buffer), input))
    {
        if (sscanf(buffer, "{\"x1\":%lf, \"y1\":%lf, \"x2\":%lf, \"y2\":%lf},",
                   &points_on_earth[count], &points_on_earth[count +1],
                   &points_on_earth[count +2], &points_on_earth[count +3]) == 4)
            count += 4;
    }
    if(strcmp(input_stream, "stdin") != 0)
        fclose(input);
    end_timer(&t1);
    start_timer(&t2);
    double distance = 0;
    for(uint i = 0; i < count; i +=4)
    {
        distance += haversine_distance(points_on_earth[i], points_on_earth[i +1],
                                       points_on_earth[i +2], points_on_earth[i +3]);
    }
    printf("Avg: %0.2fkm\n", distance / pts_count);
    free(points_on_earth);
    end_timer(&t2);
    printf("Time taking:\n\tParse info: %f sec\n\tHarversine Calc: %f sec\n*ns per Haversine Calc %0.3f\n\tTotal: %f sec\n",
           timer_sec(&t1), timer_sec(&t2),(double)timer_nano(&t2)  / pts_count, timer_sec(&t1) + timer_sec(&t2));
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Args required!\n\t<flag (-g--generate / -ca--calculate_avg_prelude)> <param1> <param2>\n");
        return -2;
    }

    if (strcmp(argv[1], "-g") == 0 || strcmp(argv[1], "--generate") == 0)
        create_points_json(atoi(argv[2]));
    else if (strcmp(argv[1], "-ca") == 0 || strcmp(argv[1], "--calculate_avg_prelude") == 0)
        calculate_avg_prologue(argv[2], atoi(argv[3]));

    return 0;

}
