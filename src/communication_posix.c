#include "communication.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PIPE_IN_PATH "/tmp/chess_pipe_to_frontend"
#define PIPE_OUT_PATH "/tmp/chess_pipe_to_backend"
#define BUFFER_SIZE 1024

static int fd_in = -1;  // read from backend (client -> frontend)
static int fd_out = -1; // write to backend (frontend -> client)
static bool g_bConnected = false;
static char chReply[BUFFER_SIZE];

static void ensure_fifos_exist(void) {
  struct stat st;
  if (stat(PIPE_IN_PATH, &st) != 0) {
    mkfifo(PIPE_IN_PATH, 0666);
  }
  if (stat(PIPE_OUT_PATH, &st) != 0) {
    mkfifo(PIPE_OUT_PATH, 0666);
  }
}

bool pipe_is_connected() {
  if (g_bConnected) return true;

  if (fd_in != -1 && fd_out != -1) {
    g_bConnected = true;
    return true;
  }

  // Try to open missing ends without blocking
  if (fd_in == -1) {
    fd_in = open(PIPE_IN_PATH, O_RDONLY | O_NONBLOCK);
    if (fd_in == -1 && errno != ENXIO && errno != ENOENT) {
      perror("open pipe_in");
    }
  }
  if (fd_out == -1) {
    fd_out = open(PIPE_OUT_PATH, O_WRONLY | O_NONBLOCK);
    if (fd_out == -1 && errno != ENXIO && errno != ENOENT) {
      perror("open pipe_out");
    }
  }

  if (fd_in != -1 && fd_out != -1) {
    g_bConnected = true;
    return true;
  }
  return false;
}

bool pipe_init() {
  ensure_fifos_exist();

  // Try to open read end and keep it non-blocking initially
  fd_in = open(PIPE_IN_PATH, O_RDONLY | O_NONBLOCK);
  if (fd_in == -1) {
    // it's fine: no writer yet
  }

  // Try to open write end (will fail until a reader opens the other side)
  fd_out = open(PIPE_OUT_PATH, O_WRONLY | O_NONBLOCK);
  if (fd_out == -1) {
    // no reader yet
  }

  g_bConnected = (fd_in != -1 && fd_out != -1);
  return true;
}

bool pipe_send_message(char *msg) {
  if (!pipe_is_connected()) {
    // attempt to re-open
    pipe_is_connected();
    if (!pipe_is_connected()) {
      fprintf(stderr, "Pipe not initialized or connected.\n");
      return false;
    }
  }

  size_t len = strlen(msg) + 1;
  ssize_t w = write(fd_out, msg, len);
  if (w < 0) {
    perror("write to pipe_out");
    return false;
  }
  return (size_t)w == len;
}

char *pipe_get_message() {
  if (!pipe_is_connected()) {
    pipe_is_connected();
    if (!pipe_is_connected()) {
      fprintf(stderr, "Pipe not initialized or connected.\n");
      return NULL;
    }
  }

  struct pollfd pfd;
  pfd.fd = fd_in;
  pfd.events = POLLIN;

  int ret = poll(&pfd, 1, -1); // block until data arrives
  if (ret <= 0) {
    return NULL;
  }

  ssize_t r = read(fd_in, chReply, BUFFER_SIZE - 1);
  if (r <= 0) {
    // read EOF or error
    if (r == 0) {
      // writer closed; reset fd and mark disconnected
      close(fd_in);
      fd_in = -1;
      g_bConnected = false;
    }
    return NULL;
  }
  chReply[r] = '\0';
  return chReply;
}

bool pipe_has_new_message() {
  if (fd_in == -1) return false;
  int bytes = 0;
  if (ioctl(fd_in, FIONREAD, &bytes) == -1) {
    return false;
  }
  return bytes > 0;
}

void pipe_close() {
  if (fd_in != -1) {
    close(fd_in);
    fd_in = -1;
  }
  if (fd_out != -1) {
    close(fd_out);
    fd_out = -1;
  }
  g_bConnected = false;
}
