#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <wait.h>
#include "so_stdio.h"

#define SIZE 4096
#define WRITE 0
#define READ 1

struct _so_file {
	char buffer[SIZE];
	int fd;
	int cursor;
	int error;
	int append;
	int last_op;
	int update;
	int eof;
	long file_cursor;
	int current_chunk;
	int pid;
};

void init_file(SO_FILE *stream)
{
	memset(stream->buffer, 0, SIZE);
	stream->cursor = 0;
	stream->error = 0;
	stream->append = 0;
	stream->last_op = -1;
	stream->update = 0;
	stream->file_cursor = 0;
	stream->eof = 0;
	stream->current_chunk = SIZE;
	stream->pid = -1;
}


SO_FILE *so_fopen(const char *pathname, const char *mode)
{

	SO_FILE *file = malloc(sizeof(SO_FILE));

	if (!file)
		return NULL;

	init_file(file);

	int fd = -1;

	if (strcmp(mode, "r") == 0) {
		fd = open(pathname, O_RDONLY);
	} else if (strcmp(mode, "r+") == 0) {
		fd = open(pathname, O_RDWR);
		file->update = 1;
	} else if (strcmp(mode, "w") == 0) {
		fd = open(pathname, O_WRONLY | O_TRUNC | O_CREAT, 0700);
	} else if (strcmp(mode, "w+") == 0) {
		fd = open(pathname, O_RDWR | O_TRUNC | O_CREAT, 0700);
		file->update = 1;
	} else if (strcmp(mode, "a") == 0) {
		fd = open(pathname, O_APPEND | O_CREAT | O_WRONLY, 0700);
		file->append = 1;
	} else if (strcmp(mode, "a+") == 0) {
		fd = open(pathname, O_APPEND | O_RDWR | O_CREAT, 0700);
		file->update = 1;
	}

	if (fd < 0) {
		free(file);
		return NULL;
	}
	file->fd = fd;

	return file;
}

int so_fclose(SO_FILE *stream)
{

	if (stream->last_op == WRITE)
		so_fflush(stream);

	if (stream == NULL)
		return SO_EOF;
	int r = close(stream->fd);

	if (r < 0) {
		stream->error = 3;
		free(stream);
		return SO_EOF;
	}

	if (stream->error == 2) {
		free(stream);
		return SO_EOF;
	}
	free(stream);
	return 0;
}

int so_fgetc(SO_FILE *stream)
{

	if (stream->update) {
		if (stream->last_op == WRITE)
			so_fseek(stream, 0, SEEK_CUR);
	}

	if (stream->cursor == stream->current_chunk) {
		memset(stream->buffer, 0, SIZE);
		stream->cursor = 0;
	}
	char c = stream->buffer[stream->cursor];

	if (stream->cursor == 0) {
		int r = read(stream->fd, stream->buffer, SIZE);

		stream->current_chunk = r;
		if (r == 0) {
			stream->eof = 1;
			return SO_EOF;
		}

		if (r < 0) {
			stream->error = 1;
			return SO_EOF;
		}
		c = stream->buffer[stream->cursor];
	}
	stream->cursor++;
	stream->last_op = READ;

	stream->file_cursor++;

	return (int) c;
}

int so_fputc(int c, SO_FILE *stream)
{
	if (stream->update) {
		if (stream->last_op == READ)
			so_fseek(stream, 0, SEEK_CUR);
	}

	if (stream->cursor == SIZE)
		so_fflush(stream);

	stream->buffer[stream->cursor] = (char) c;
	stream->cursor++;
	stream->last_op = WRITE;
	stream->file_cursor++;
	return c;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	unsigned long i, j;
	int c = 0;

	for (i = 0; i < nmemb; i++) {
		for (j = 0; j < size; j++) {
			if (so_feof(stream))
				break;
			c = so_fgetc(stream);
			if (stream->error != 0 || so_feof(stream))
				break;
			memcpy(ptr + i * size + j, &c, 1);
		}

		if (so_feof(stream))
			break;

		if (stream->error != 0)
			break;
	}
	return i;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	unsigned long i, j;

	for (i = 0; i < nmemb; i++) {
		for (j = 0; j < size; j++) {
			so_fputc(*(char *) (ptr + i * size + j), stream);
			if (so_feof(stream))
				break;
			if (stream->error != 0)
				break;
		}
		if (so_feof(stream))
			break;
		if (stream->error != 0)
			break;
	}
	return i;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
	if (stream->last_op == READ)
		memset(stream->buffer, 0, SIZE);
	else if (stream->last_op == WRITE)
		so_fflush(stream);

	int off = lseek(stream->fd, offset, whence);

	if (off == SO_EOF) {
		stream->error = 1;
		return SO_EOF;
	}
	stream->cursor = 0;
	stream->file_cursor = off;

	return 0;
}

long so_ftell(SO_FILE *stream)
{
	return stream->file_cursor;
}

int so_fileno(SO_FILE *stream)
{
	return stream->fd;
}

int so_ferror(SO_FILE *stream)
{
	return stream->error;
}

int so_feof(SO_FILE *stream)
{
	return stream->eof;
}

int so_fflush(SO_FILE *stream)
{
	int char_written = 0, limit = stream->cursor;

	while (char_written < limit) {
		int r = write(stream->fd, stream->buffer + char_written, limit - char_written);

		if (r <= 0) {
			stream->error = 2;
			return SO_EOF;
		}
		char_written += r;
	}
	memset(stream->buffer, 0, SIZE);
	stream->cursor = 0;
	return 0;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	int pipedes[2];

	pipe(pipedes);
	pid_t pid = fork();

	SO_FILE *file = malloc(sizeof(SO_FILE));

	switch (pid) {

	case -1:
		free(file);
		return NULL;

	case 0:
		if (strcmp(type, "r") == 0) {
			close(pipedes[0]);
			dup2(pipedes[1], STDOUT_FILENO);
		} else if (strcmp(type, "w") == 0) {
			close(pipedes[1]);
			dup2(pipedes[0], STDIN_FILENO);
		}

		execlp("sh", "sh", "-c", command, NULL);
		return NULL;

	default:
		break;
	}

	file->fd = 0;
	if (strcmp(type, "r") == 0) {
		close(pipedes[1]);
		file->fd = pipedes[0];
	} else if (strcmp(type, "w") == 0) {
		close(pipedes[0]);
		file->fd = pipedes[1];
	}

	init_file(file);
	file->pid = pid;

	return file;
}

int so_pclose(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	int status;
	int pid = stream->pid;

	so_fclose(stream);

	int response = waitpid(pid, &status, 0);

	if (response == -1)
		return -1;

	return WEXITSTATUS(status);
}


