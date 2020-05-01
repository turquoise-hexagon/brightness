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
        "usage : %s [option] <parameter>\n\n"
        "options :\n"
        "    -a <percentage>    set <percentage> as the absolute brightness value\n"
        "    -r <percentage>    set <percentage> as the relative brightness value\n"
        "    -q                 output the current brightness percentage\n",
        basename(name)
    );

    exit(EXIT_FAILURE);
}

static FILE *
open_file(const char *path, const char *mode)
{
    FILE *file = fopen(path, mode);

    if (file == NULL)
        errx(EXIT_FAILURE, "failed to open '%s'", path);

    return file;
}

static void
close_file(const char *path, FILE *file)
{
    if (fclose(file) == EOF)
        errx(EXIT_FAILURE, "failed to close '%s'", path);
}

static long
get_num(const char *str)
{
    errno = 0;
    char *ptr;

    long num = strtol(str, &ptr, 10);

    if (errno != 0 || *ptr != 0)
        errx(EXIT_FAILURE, "'%s' isn't a valid integer", str);

    return num;
}

static unsigned
get_content(const char *path, FILE *file)
{
    char input[LINE_MAX];

    if (fgets(input, LINE_MAX, file) == NULL)
        errx(EXIT_FAILURE, "failed to get content from '%s'", path);

    input[strnlen(input, LINE_MAX) - 1] = 0;

    return get_num(input);
}

int
main(int argc, char **argv)
{
    if (argc == 1)
        usage(argv[0]);

    /* build paths */
    char max_path[256] = {0};

    if (snprintf(max_path, sizeof max_path, "%s/max_brightness", PATH) < 0)
        errx(EXIT_FAILURE, "failed to build path to max brightness");

    char cur_path[256] = {0};

    if (snprintf(cur_path, sizeof cur_path, "%s/brightness", PATH) < 0)
        errx(EXIT_FAILURE, "failed to build path to current brightness");

    /* get values from brightness files */ 
    FILE *file;

    file = open_file(max_path, "r");
    const unsigned max = get_content(max_path, file);
    close_file(max_path, file);

    file = open_file(cur_path, "r");
    long cur = get_content(cur_path, file);
    close_file(cur_path, file);

    const unsigned min = (long)max * MIN / 100;

    /* argument parsing */
    bool write = 0;

    for (int arg; (arg = getopt(argc, argv, ":r:a:q")) != -1;)
        switch (arg) {
            case 'a': cur =  (long)max * get_num(optarg) / 100; write = 1; break;
            case 'r': cur += (long)max * get_num(optarg) / 100; write = 1; break;
            case 'q':
                printf("%ld\n", cur * 100 / max);

                break;
            default:
                usage(argv[0]);
        }

    if (optind < argc)
        usage(argv[0]);

    if (write == 1) {
        if (cur < min) cur = min;
        if (cur > max) cur = max;

        file = open_file(cur_path, "w");

        if (fprintf(file, "%ld\n", cur) < 0)
            errx(EXIT_FAILURE, "failed to write to '%s'", cur_path);

        close_file(cur_path, file);
    }
    
    return EXIT_SUCCESS;
}
