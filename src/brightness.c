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
print_usage(char *program_name)
{
    fprintf(
        stderr,
        "usage : %s [option] <parameter>\n\n"
        "options :\n"
        "    -a <percentage>    set <percentage> as the absolute brightness value\n"
        "    -r <percentage>    set <percentage> as the relative brightness value\n"
        "    -q                 query the current brightness percentage\n",
        basename(program_name)
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
convert_to_number(const char *str)
{
    errno = 0;
    char *ptr;

    long number = strtol(str, &ptr, 10);

    if (errno != 0 || *ptr != 0)
        errx(EXIT_FAILURE, "'%s' isn't a valid integer", str);

    return number;
}

static unsigned
get_number_from_file(const char *path, FILE *file)
{
    char str[LINE_MAX] = {0};

    if (fgets(str, LINE_MAX, file) == NULL)
        errx(EXIT_FAILURE, "failed to get content from '%s'", path);

    /* fix string */
    str[strnlen(str, LINE_MAX) - 1] = 0;

    return convert_to_number(str);
}

int
main(int argc, char **argv)
{
    if (argc < 2)
        print_usage(argv[0]);

    /* build paths */
    char max_brightness_path[PATH_MAX] = {0};

    if (snprintf(max_brightness_path, \
            sizeof(max_brightness_path), "%s/max_brightness", PATH) < 0)
        errx(EXIT_FAILURE, "failed to build path to max_brightness");

    char cur_brightness_path[PATH_MAX] = {0};

    if (snprintf(cur_brightness_path, \
            sizeof(cur_brightness_path), "%s/brightness", PATH) < 0)
        errx(EXIT_FAILURE, "failed to build path to brightness");

    /* get values from brightness files */
    FILE *file;

    file = open_file(max_brightness_path, "r");
    const unsigned max_brightness = get_number_from_file(max_brightness_path, file);
    close_file(max_brightness_path, file);

    file = open_file(cur_brightness_path, "r");
    long cur_brightness = get_number_from_file(cur_brightness_path, file);
    close_file(cur_brightness_path, file);

    const unsigned min_brightness = (long)max_brightness * MIN / 100;

    /* argument parsing */
    bool write = 0;
    bool query = 0;

    for (int arg; (arg = getopt(argc, argv, ":r:a:q")) != -1;)
        switch (arg) {
            case 'a': write = 1; cur_brightness  = (long)max_brightness * \
                          convert_to_number(optarg) / 100; break;
            case 'r': write = 1; cur_brightness += (long)max_brightness * \
                          convert_to_number(optarg) / 100; break;
            case 'q': query = 1; break;
            default :
                print_usage(argv[0]);
        }

    if (optind < argc) /* handle mismatched parameters */
        print_usage(argv[0]);

    if (query == 1)
        printf("%ld\n", cur_brightness * 100 / max_brightness);

    if (write == 1) {
        /* make new value stay between defined boundaries */
        if (cur_brightness < min_brightness) cur_brightness = min_brightness;
        if (cur_brightness > max_brightness) cur_brightness = max_brightness;

        file = open_file(cur_brightness_path, "w");

        if (fprintf(file, "%ld\n", cur_brightness) < 0)
            errx(EXIT_FAILURE, "failed to write to '%s'", cur_brightness_path);

        close_file(cur_brightness_path, file);
    }

    return EXIT_SUCCESS;
}
