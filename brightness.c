#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

#define ERROR(status, ...) {      \
    fprintf(stderr, __VA_ARGS__); \
                                  \
    exit(status);                 \
}

static const double MIN = 0.5;
static const char *PATH = "/sys/class/backlight/intel_backlight";

static noreturn void
usage(char *name)
{
    ERROR(
        1,
        "usage : %s [-ar <percentage>] [-q]                               \n"
        "                                                                 \n"
        "options :                                                        \n"
        "    -a <percentage>    change brightness by relative <percentage>\n"
        "    -r <percentage>    change brightness by absolute <percentage>\n"
        "    -q                 query current brightness percentage       \n",
        basename(name)
    )
}

static double
_strtod(const char *str)
{
    errno = 0;
    char *ptr;

    double n; n = strtod(str, &ptr);

    if (errno != 0 || *ptr != 0) {
        errno = EINVAL;

        return 0;
    }

    return n;
}

static void
get_value_from_file(const char *path, double *value)
{
    FILE *file;

    if (!(file = fopen(path, "r")))
        ERROR(1, "error : failed to open '%s'\n", path)

    char buf[LINE_MAX] = {0};

    if (!fgets(buf, sizeof(buf), file))
        ERROR(1, "error : failed to read from '%s'\n", path)

    /* fix string */
    buf[strnlen(buf, sizeof(buf)) - 1] = 0;

    errno = 0; *value = _strtod(buf);

    if (errno != 0)
        ERROR(1, "error : invalid value '%s' read from '%s'\n", buf, path)

    if (fclose(file))
        ERROR(1, "error : failed to close '%s'\n", path)
}

static void
write_value_to_file(const char *path, double value)
{
    FILE *file;

    if (!(file = fopen(path, "w")))
        ERROR(1, "error : failed to open '%s'\n", path)

    if (fprintf(file, "%.0f\n", value) < 0)
        ERROR(1, "error : failed to write to '%s'\n", path)

    if (fclose(file))
        ERROR(1, "error : failed to close '%s'\n", path)
}

int
main(int argc, char **argv)
{
    if (argc < 2)
        usage(argv[0]);

    char max_path[PATH_MAX] = {0};
    char cur_path[PATH_MAX] = {0};

    if (snprintf(max_path, sizeof(max_path), "%s/%s", PATH, "max_brightness") < 0)
        ERROR(1, "error : failed to build path to brightness files\n")

    if (snprintf(cur_path, sizeof(cur_path), "%s/%s", PATH,     "brightness") < 0)
        ERROR(1, "error : failed to build path to brightness files\n")

    double max;
    double min;
    double cur;

    get_value_from_file(max_path, &max);
    get_value_from_file(cur_path, &cur);

    min = max * MIN / 100;

    int write = 0;
    int query = 0;

    for (int arg; (arg = getopt(argc, argv, ":a:r:q")) != -1;)
        switch (arg) {
            case 'a':
                write = 1;
                errno = 0;

                cur = max * _strtod(optarg) / 100;

                if (errno != 0)
                    ERROR(1, "error : '%s' invalid parameter\n", optarg)

                break;
            case 'r':
                write = 1;
                errno = 0;

                cur += max * _strtod(optarg) / 100;

                if (errno != 0)
                    ERROR(1, "error : '%s' invalid parameter\n", optarg)

                break;
            case 'q':
                query = 1;

                break;
            default :
                usage(argv[0]);
        }

    /* handle mismatched parameters */
    if (optind < argc)
        usage(argv[0]);

    /* keep brightness between min and max values */
    if (cur < min) cur = min;
    if (cur > max) cur = max;

    if (query)
        printf("%.0f\n", cur * 100 / max);

    if (write)
        write_value_to_file(cur_path, cur);

    return 0;
}
