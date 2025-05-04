#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdbool.h>

#define PORT 1212

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

// ------------------------------------
// | למעלה מימין | למעלה | למעלה משמאל |
// ------------------------------------
// |     מימין |   התא |   משמאל       |
// ------------------------------------
// | למטה מימין |  למטה |  למטה משמאל  |
// ------------------------------------

static int8_t num_neighbours = sizeof(neighbours) / sizeof(neighbours[0]);

struct message {
  int type;
  int len;
};

struct GameConfig
{
  int rows;
  int cols;
  int mines;
};

struct message messages[] = {
  {0, -1}, // board + pos
  {1, 1}, // key
  {2, 1}, // win/loose
  {3, sizeof(struct GameConfig)} // win/loose
};

struct Position
{
  int i;
  int j;
};

bool get_message(int sock, int *type, void *data) {
  struct message msg;
  int num_read = read(sock, &msg.type, sizeof(msg.type));
  // FILE *f = fopen("/tmp/common.txt", "a");
  // fprintf(f, "1 num_read %d\n", num_read);
  // fclose(f);
  if (num_read != sizeof(msg.type)) return false;

  *type = msg.type;
  // if (msg.type != wanted_type) return false;

  num_read = read(sock, &msg.len, sizeof(msg.len));
  if (num_read != sizeof(msg.len)) return false;

  struct message curr = messages[msg.type];
  if (curr.len != -1 && msg.len != curr.len) return false;

  num_read = read(sock, data, msg.len);
  if (num_read != msg.len) return false;

  return true;
}

void send_message(int sock, int wanted_type, void *data, int len) {
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
