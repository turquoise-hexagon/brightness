#include <errno.h>
#include <limits.h>
#include <string.h>
#include <libgen.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdnoreturn.h>

static const float  MIN = 0.75;
static const char *PATH = "/sys/class/backlight/intel_backlight";

#define ERROR(status, ...) {      \
    fprintf(stderr, __VA_ARGS__); \
                                  \
    exit(status);                 \
}

static noreturn void
usage(char *name)
{
    ERROR(
        1,
        "usage : %s [-ar <percentage>] [-q]\n"
        "\n"
        "options :\n"
        "    -a <percentage>    set <percentage> as absolute brightness value\n"
        "    -r <percentage>    set <percentage> as relative brightness value\n"
        "    -q                 query the current brightness value\n",
        basename(name));
}

static float
_strtof(const char *str)
{
    errno = 0;
    float tmp;

    {
        char *ptr;

        tmp = strtof(str, &ptr);
        
        if (errno != 0 || *ptr != 0)
            return 0;
    }

    return tmp;
}

static int
get_value_from_file(const char *path, float *dest)
{
    FILE *file;

    if (!(file = fopen(path, "r")))
        return 0;

    char buffer[LINE_MAX] = {0};

    if (!fgets(buffer, sizeof(buffer), file))
        return 0;

    if (fclose(file))
        return 0;

    /* fix string */
    buffer[strnlen(buffer, sizeof(buffer)) - 1] = 0;

    errno = 0;
    *dest = _strtof(buffer);

    if (errno != 0)
        return 0;

    return 1;
}

static int
write_value_to_file(const char *path, float source)
{
    FILE *file;

    if (!(file = fopen(path, "w")))
        return 0;

    if (fprintf(file, "%.0f\n", source) < 0)
        return 0;

    if (fclose(file))
        return 0;

    return 1;
}

int
main(int argc, char **argv)
{
    char max_path[PATH_MAX] = {0};
    char cur_path[PATH_MAX] = {0};

    if (snprintf(max_path, sizeof(max_path), "%s/max_brightness", PATH) < 0)
        ERROR(1, "error : failed to build path to max brightness file\n");

    if (snprintf(cur_path, sizeof(cur_path), "%s/brightness",     PATH) < 0)
        ERROR(1, "error : failed to build path to current brightness file\n");

    float max;
    float cur;
    float min;

    if (!get_value_from_file(max_path, &max))
        ERROR(1, "error : failed to get value from '%s'\n", max_path);

    if (!get_value_from_file(cur_path, &cur))
        ERROR(1, "error : failed to get value from '%s'\n", cur_path);

    min = max * MIN / 100;

    int write = 0;
    int query = 0;

    {
        int arg;

        while ((arg = getopt(argc, argv, ":a:r:q")) != -1)
            switch (arg) {
                case 'a':
                    write = 1;
                    errno = 0;

                    cur = max * _strtof(optarg) / 100;

                    if (errno != 0)
                        ERROR(1, "error : '%s' invalid parameter\n", optarg);

                    break;
                case 'r':
                    write = 1;
                    errno = 0;

                    cur += max * _strtof(optarg) / 100;

                    if (errno != 0)
                        ERROR(1, "error '%s' invalid parameter\n", optarg);

                    break;
                case 'q':
                    query = 1;

                    break;
                default :
                    usage(argv[0]);
            }
    }

    /* handle mismatched parameters */
    if (optind < argc)
        usage(argv[0]);

    /* keep brightness between min & max values */
    if (cur < min) cur = min;
    if (cur > max) cur = max;

    if (query)
        printf("%.0f\n", cur * 100 / max);

    if (write)
        if (!(write_value_to_file(cur_path, cur)))
            ERROR(1, "error : failed to write value to '%s'\n", cur_path);

    return 0;
}
