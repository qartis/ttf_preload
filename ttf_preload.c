#include <fontconfig/fontconfig.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define TEMP_FILE_PATTERN "ttf_preload_XXXXXX"


int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <font_file> <program> [args...]\n", argv[0]);
        return 1;
    }

    /* Locate global fonts.conf, respecting FONTCONFIG_FILE, FONTCONFIG_SYSROOT
       and FONTCONFIG_PATH */
    const char *fontconfig_file = getenv("FONTCONFIG_FILE");
    if (fontconfig_file == NULL) {
        FcConfig* config = FcInitLoadConfig();
        fontconfig_file = (const char *)FcConfigGetFilename(config, NULL);
    }

    char temp_config_path[PATH_MAX];
    char *temp_dir = getenv("TMPDIR");
    if (temp_dir == NULL) {
        temp_dir = "/tmp";
    }

    snprintf(temp_config_path, sizeof(temp_config_path),
             "%s/%s", temp_dir, TEMP_FILE_PATTERN);

    int fd = mkstemp(temp_config_path);
    if (fd == -1) {
        perror("Failed to create temporary file");
        return 1;
    }

    FILE *f = fdopen(fd, "w");
    if (f == NULL) {
        perror("Failed to open temporary file");
        return 1;
    }

    char font_path[PATH_MAX];
    if (realpath(argv[1], font_path) == NULL) {
        perror("Failed to get real path of font file");
        return 1;
    }

    fprintf(f, "<?xml version=\"1.0\"?>\n"
               "<!DOCTYPE fontconfig SYSTEM \"urn:fontconfig:fonts.dtd\">\n"
               "<fontconfig>\n"
               "  <dir>%s</dir>\n"
               "  <include ignore_missing=\"yes\">%s</include>\n"
               "</fontconfig>\n",
            dirname(font_path), fontconfig_file);
    fclose(f);

    setenv("FONTCONFIG_FILE", temp_config_path, 1);

    execvp(argv[2], &argv[2]);

    perror("execvp");
    return 1;
}
