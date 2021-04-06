#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "util/so_stdio.h"

#define SIZE 3
#define WRITE 0
#define READ 1

struct _so_file {
    char *pathname;
    char *mode;
    char buffer[4096];
    int fd;
    int cursor;
    int error;
    int append;
    int last_op;
};

int update_mode(SO_FILE *stream) {
    if (strcmp(stream->mode, "r+") == 0 ||
        strcmp(stream->mode, "w+") == 0 ||
        strcmp(stream->mode, "a+") == 0) {
        return 1;
    }
    return 0;
}

SO_FILE *so_fopen(const char *pathname, const char *mode) {

    SO_FILE *file = malloc(sizeof(SO_FILE));
    if (!file) return NULL;
    memset(file->buffer, 0, 4096);
    file->cursor = 0;
    file->error = 0;
    file->append = 0;
    file->last_op = -1;

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
        fd = open(file->pathname, O_APPEND | O_CREAT | O_WRONLY, 0644);
        file->append = 1;
    } else if (strcmp(mode, "a+") == 0) {
        fd = open(file->pathname, O_APPEND | O_RDWR | O_CREAT, 0644);
        file->append = 1;
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
    if (r < 0) {
        stream->error = 0;
        return SO_EOF;
    }
    free(stream->pathname);
    free(stream->mode);
    free(stream);
    return 0;
}

int so_fgetc(SO_FILE *stream) {

//    if (stream->append) {
//        stream->cursor = so_fseek(stream, 1, SEEK_END);
//    }

    if (update_mode(stream)) {
        if (stream->last_op == WRITE) {
            so_fseek(stream, 0, SEEK_CUR);
        }
    }

    if (stream->cursor == SIZE) {
        memset(stream->buffer, 0, SIZE);
        stream->cursor = 0;
    }
    char c = stream->buffer[stream->cursor];
    if (c == 0) {
        int r = read(stream->fd, stream->buffer, SIZE);
        if (r <= 0) return SO_EOF;
        c = stream->buffer[stream->cursor];
    }
    stream->cursor++;
    stream->last_op = READ;
    return (int) c;
}

int so_fputc(int c, SO_FILE *stream) {

//    if (stream->append) {
//        stream->cursor = so_fseek(stream, 1, SEEK_END);
//    }

    if (update_mode(stream)) {
        if (stream->last_op == READ) {
            so_fseek(stream, 0, SEEK_CUR);
        }
    }

    if (stream->cursor == SIZE) {
        so_fflush(stream);

    }
    stream->buffer[stream->cursor] = (char) c;
    stream->cursor++;
    stream->last_op = WRITE;
    return c;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
    int i, j, c = 0;
    for (i = 0; i < nmemb; i++) {
        for (j = 0; j < size; j++) {
            c = so_fgetc(stream);
            if (c == SO_EOF) break;
            memcpy(ptr + i * size + j, &c, 1);
        }
        if (c == SO_EOF) break;
    }
    return i;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
    int i, j, r = 0;
    for (i = 0; i < nmemb; i++) {
        for (j = 0; j < size; j++) {
            r = so_fputc(*(char *) (ptr + i * size + j), stream);
            if (r == SO_EOF) break;
        }
        if (r == SO_EOF) break;
    }
    return i;
}

int so_fseek(SO_FILE *stream, long offset, int whence) {
    if (stream->last_op == READ) {
        memset(stream->buffer, 0, SIZE);
    } else if (stream->last_op == WRITE) {
        so_fflush(stream);
    }
    int off = lseek(stream->fd, offset, whence);
    if (off == SO_EOF) {
        stream->error = 0;
        return SO_EOF;
    }
    stream->cursor = 0;

    return 0;
}

long so_ftell(SO_FILE *stream) {
    return lseek(stream->fd, 0, SEEK_CUR);

}

int so_fileno(SO_FILE *stream) {
    return stream->fd;
}

int so_ferror(SO_FILE *stream) {
    return stream->error;
}

int so_feof(SO_FILE *stream) {
    int curr = lseek(stream->fd, 0, SEEK_CUR);
    int fin = lseek(stream->fd, 0, SEEK_END);
    lseek(stream->fd, curr, SEEK_SET);
    if (curr == fin) {
        return 1;
    }
    return 0;
}

int so_fflush(SO_FILE *stream) {
    char c;
    int char_written = 0, limit = stream->cursor;
    while (char_written != limit) {
        int r = write(stream->fd, stream->buffer + char_written, limit - char_written);
        if (r < 0) return SO_EOF;
        char_written += r;
    }
    memset(stream->buffer, 0, SIZE);
    stream->cursor = 0;
    return 0;
}

SO_FILE *so_popen(const char *command, const char *type) {
    return NULL;
}

int so_pclose(SO_FILE *stream) {
    return 0;
}


