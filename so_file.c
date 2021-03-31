#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "util/so_stdio.h"

struct _so_file {
    char *pathname;
    char *mode;
    char buffer[4096];
    int fd;
};

SO_FILE *so_fopen(const char *pathname, const char *mode) {

    SO_FILE *file = malloc(sizeof(SO_FILE));
    if (!file) return NULL;

    file->pathname = malloc(strlen(pathname));
    if (!file->pathname) {
        free(file);
        return NULL;
    }
    strcpy(file->pathname, pathname);

    file->mode = malloc(strlen(mode));
    if (!file->mode) {
        free(file->pathname);
        free(file);
        return NULL;
    }
    int fd = -1;
    strcpy(file->mode, mode);
    // fd = open(file->pathname, O_WRONLY | O_CREAT, 0644);
    if (strcmp(mode, "r") == 0) {
        fd = open(file->pathname, O_WRONLY);
    } else if (strcmp(mode, "r+") == 0) {
        fd = open(file->pathname, O_RDWR);
    } else if (strcmp(mode, "w") == 0) {
        fd = open(file->pathname, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    } else if (strcmp(mode, "w+") == 0) {
        fd = open(file->pathname, O_RDWR | O_TRUNC | O_CREAT, 0644);
    } else if (strcmp(mode, "a") == 0) {
        fd = open(file->pathname, O_APPEND | O_CREAT, 0644);
    } else if (strcmp(mode, "a+") == 0) {
        fd = open(file->pathname, O_APPEND | O_WRONLY | O_CREAT, 0644);
    }
    if (fd < 0) {
        free(file->pathname);
        free(file->mode);
        free(file);
        return NULL;
    }
    file->fd = fd;

    return file;
}

int so_fclose(SO_FILE *stream) {
    int r = close(stream->fd);
    if (r < 0) return SO_EOF;
    free(stream->pathname);
    free(stream->mode);
    free(stream);
    return 0;
}