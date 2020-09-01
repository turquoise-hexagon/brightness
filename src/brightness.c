#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

static const float MIN = 0.75;
static const char *PATH = "/sys/class/backlight/intel_backlight";

static noreturn void
usage(char *name)
{
    fprintf(
        stderr,
        "usage : %s [-ar <percentage>] [-q]\n"
        "\n"
        "options :\n"
        "    -a <percentage>    set <percentage> as absolute brightness value\n"
        "    -r <percentage>    set <percentage> as relative brightness value\n"
        "    -q                 query the current brightness value\n",
        basename(name));

    exit(1);
}

static FILE *
open_file(const char *path, const char *mode)
{
    FILE *file;

    if (! (file = fopen(path, mode))) {
        fprintf(stderr, "error : failed to open '%s'\n", path);

        exit(1);
    }

    return file;
}

static void
close_file(const char *path, FILE *file)
{
    if (fclose(file) != 0) {
        fprintf(stderr, "error : failed to close '%s'\n", path);

        exit(1);
    }
}

static float
convert_to_number(const char *str)
{
    errno = 0;
    float num;

    {
        char *ptr;

        num = strtof(str, &ptr);

        if (errno != 0 || *ptr != 0) {
            fprintf(stderr, "error : '%s' isn't a valid brightness value\n", str);

            exit(1);
        }
    }

    return num;
}

static float
get_value_from_file(const char *path)
{
    FILE *file;

    file = open_file(path, "r");

    char input[LINE_MAX] = {0};

    if (! fgets(input, LINE_MAX, file)) {
        fprintf(stderr, "error : failed to get value from '%s'\n", path);

        exit(1);
    }

    close_file(path, file);

    /* fix string */
    input[strnlen(input, LINE_MAX) - 1] = 0;

    return convert_to_number(input);
}

static void
write_value_to_file(const char *path, float value)
{
    FILE *file;

    file = open_file(path, "w");

    if (fprintf(file, "%u\n", (unsigned)value) < 0) {
        fprintf(stderr, "error : failed to write to '%s'\n", path);

        exit(1);
    }

    close_file(path, file);
}

int
main(int argc, char **argv)
{
    char max_path[PATH_MAX] = {0};
    char cur_path[PATH_MAX] = {0};

    if (snprintf(max_path, PATH_MAX, "%s/max_brightness", PATH) < 0) {
        fprintf(stderr, "error : failed to build path to max brightness file\n");

        exit(1);
    }

    if (snprintf(cur_path, PATH_MAX, "%s/brightness", PATH) < 0) {
        fprintf(stderr, "error : failed to build path to curent brightness file\n");

        exit(1);
    }

    float max;
    float cur;
    float min;

    max = get_value_from_file(max_path);
    cur = get_value_from_file(cur_path);
    min = max * MIN / 100;

    bool write = 0;
    bool query = 0;

    for (int arg; (arg = getopt(argc, argv, ":r:a:q")) != -1;)
        switch (arg) {
            case 'a': write = 1; cur  = max * convert_to_number(optarg) / 100; break;
            case 'r': write = 1; cur += max * convert_to_number(optarg) / 100; break;
            case 'q': query = 1; break;
            default :
                usage(argv[0]);
        }

    if (optind < argc)
        usage(argv[0]);

    /* keep brightness between min & max values */
    if (cur < min) cur = min;
    if (cur > max) cur = max;

    if (query == 1)
        printf("%u\n", (unsigned)(cur * 100 / max));

    if (write == 1)
        write_value_to_file(cur_path, cur);

    return 0;
}
