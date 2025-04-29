#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdbool.h>

#define ROWS 10
#define COLS 10

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

struct message messages[] = {
  {0, ROWS * COLS}, // board
  {1, 2 * sizeof(int)}, // pos
  {2, 1} // key
};

bool get_message(int sock, int wanted_type, void *data) {
  struct message msg;
  int num_read = read(sock, &msg.type, sizeof(msg.type));
  if (num_read != sizeof(msg.type)) return false;

  if (msg.type != wanted_type) return false;

  int num_read = read(sock, &msg.len, sizeof(msg.len));
  if (num_read != sizeof(msg.type)) return false;

  struct message curr = messages[msg.type];
  if (msg.len != curr.len) return false;

  int num_read = read(sock, data, msg.len);
  if (num_read != msg.len) return false;

  return true;
}

#endif // COMMON_H
