#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "util/so_stdio.h"

#define SIZE 4096

struct _so_file {
    char *pathname;
    char *mode;
    char buffer[4096];
    int fd;
    int cursor;
};

SO_FILE *so_fopen(const char *pathname, const char *mode) {

    SO_FILE *file = malloc(sizeof(SO_FILE));
    if (!file) return NULL;
    memset(file->buffer, 0, 4096);
    file->cursor = 0;

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
    if (strcmp(mode, "r") == 0) {
        fd = open(file->pathname, O_RDONLY);
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
    if (stream == NULL) return SO_EOF;
    int r = close(stream->fd);
    if (r < 0) return SO_EOF;
    free(stream->pathname);
    free(stream->mode);
    free(stream);
    return 0;
}

int so_fgetc(SO_FILE *stream) {
    int poz = stream->cursor % SIZE;
    char c = stream->buffer[poz];
    if (c == 0 || stream->cursor % SIZE == 0) {
        int r = read(stream->fd, stream->buffer + poz, SIZE - poz);
        if (r <= 0) return SO_EOF;
        c = stream->buffer[poz];
    }
    stream->cursor++;
    return (int) c;
}

int so_fputc(int c, SO_FILE *stream) {
    stream->buffer[stream->cursor % SIZE] = (char) c;
    int r = write(stream->fd, stream->buffer + stream->cursor % SIZE, 1);
    if (r != 1) return SO_EOF;
    stream->cursor++;
    return c;
}
