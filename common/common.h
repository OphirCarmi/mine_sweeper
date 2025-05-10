#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdbool.h>

#define PORT 1212

// #define SHOW

static int8_t neighbours[][2] = {
    {-1, -1}, // למעלה משמאל
    {-1, 0},  // למעלה
    {-1, 1},  // למעלה מימין
    {0, -1},  // משמאל
    {0, 1},   // מימין
    {1, -1},  // למטה משמאל
    {1, 0},   // למטה
    {1, 1},   // למטה מימין
};

static int8_t num_neighbours = sizeof(neighbours) / sizeof(neighbours[0]);

struct __attribute__((__packed__)) message {
  int8_t type;
  int len;
};

struct __attribute__((__packed__)) GameConfig
{
  int8_t rows;
  int8_t cols;
  int mines;
};

struct message messages[] = {
  {0, -1}, // board + pos
  {1, 1}, // key
  {2, 1}, // win/loose
  {3, sizeof(struct GameConfig)} // win/loose
};

struct __attribute__((__packed__)) Position
{
  int8_t i;
  int8_t j;
};

struct __attribute__((__packed__)) Cell {
  struct Position pos;
  char val;
};

bool get_message(int sock, int8_t *type, void *data, int *len) {
  struct message msg;
  int num_read = read(sock, &msg.type, sizeof(msg.type));
  if (num_read != sizeof(msg.type)) return false;

  *type = msg.type;

  num_read = read(sock, &msg.len, sizeof(msg.len));
  if (num_read != sizeof(msg.len)) return false;

  struct message curr = messages[msg.type];
  if (curr.len != -1 && msg.len != curr.len) return false;

  num_read = read(sock, data, msg.len);
  if (num_read != msg.len) return false;

  if (len) *len = num_read;

  return true;
}

void send_message(int sock, int8_t wanted_type, void *data, int len) {
  struct message curr = messages[wanted_type];
  if (curr.len == -1) curr.len = len;
  int tot_size = sizeof(curr) + curr.len;

  char *mem = (char *)malloc(tot_size);
  char *ptr = mem;
  memcpy(ptr, &curr.type, sizeof(curr.type));
  ptr += sizeof(curr.type);
  memcpy(ptr, &curr.len, sizeof(curr.len));
  ptr += sizeof(curr.len);
  memcpy(ptr, data, curr.len);

  write(sock, mem, tot_size);

  free(mem);
}


#endif // COMMON_H
