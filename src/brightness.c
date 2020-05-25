#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

static void
usage(char *name)
{
    fprintf(
        stderr,
        "usage : %s [-ar <percentage>] [-q]\n\n"
        "options :\n"
        "    -a <percentage>   set <percentage> as the absolute brightness value\n"
        "    -r <percentage>   set <percentage> as the relative brightness value\n"
        "    -q                query the current brightness percentage\n",
        basename(name));

    exit(1);
}

/*
 * helper functions
 */
static FILE *
open_file(const char *path, const char *mode)
{
    FILE *file;

    if ((file = fopen(path, mode)) == NULL)
        errx(1, "failed to open '%s'", path);

    return file;
}

static void
close_file(const char *path, FILE *file)
{
    if (fclose(file) == EOF)
        errx(1, "failed to close '%s'", path);
}

static long
convert_to_number(const char *str)
{
    errno = 0;

    char *ptr;
    long number;

    number = strtol(str, &ptr, 10);

    if (errno != 0 || *ptr != 0)
        errx(1, "'%s' isn't a valid integer", str);

    return number;
}

static long
get_number_from_file(const char *path, FILE *file)
{
    char line[LINE_MAX] = {0};

    if (fgets(line, LINE_MAX, file) == NULL)
        errx(1, "failed to get content from '%s'", path);

    /* fix string */
    line[strnlen(line, LINE_MAX) - 1] = 0;

    long number;

    if ((number = convert_to_number(line)) < 0)
        errx(1, "'%s' isn't a valid positive integer", line);

    return number;
}

int
main(int argc, char **argv)
{
    if (argc < 2)
        usage(argv[0]);

    /* build path to relevant files */
    char max_path[PATH_MAX] = {0};
    char cur_path[PATH_MAX] = {0};

    if (snprintf(max_path, sizeof(max_path), "%s/max_brightness", PATH) < 0)
        errx(1, "failed to build path to max brightness file");

    if (snprintf(cur_path, sizeof(cur_path), "%s/brightness", PATH) < 0)
        errx(1, "failed to build path to current brightness file");

    /* get values from relevant files */
    FILE *file;
    long max;
    long cur;
    long min;

    file = open_file(max_path, "r");
    max = get_number_from_file(max_path, file);
    close_file(max_path, file);

    file = open_file(cur_path, "r");
    cur = get_number_from_file(cur_path, file);
    close_file(cur_path, file);

    min = max * (long)MIN / 100;

    /* argument parsing */
    bool write = 0;
    bool query = 0;

    for (int arg; (arg = getopt(argc, argv, ":r:a:q")) != -1;)
        switch (arg) {
            case 'a': write = 1; cur  = max * convert_to_number(optarg) / 100; break;
            case 'r': write = 1; cur += max * convert_to_number(optarg) / 100; break;
            case 'q': query = 1; break;
            default:
                usage(argv[0]);
        }

    if (optind < argc) /* handle mismatched parameters */
        usage(argv[0]);

    /* make new value stay between the defined boundaries */
    if (cur < min) cur = min;
    if (cur > max) cur = max;

    if (query == 1)
        printf("%ld\n", cur * 100 / max);

    if (write == 1) {
        /* write new value */
        file = open_file(cur_path, "w");

        if (fprintf(file, "%ld\n", cur) < 0)
            errx(1, "failed to write to '%s'", cur_path);

        close_file(cur_path, file);
    }

    return 0;
}
