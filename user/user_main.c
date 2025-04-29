#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common/common.h"

#define PORT 8080

void MoveByDiff(int write_fd, int diff_i, int diff_j)
{
  for (int m = 0; m < diff_i; ++m)
  {
    char c = 'x';
    write(write_fd, &c, sizeof(c));
    usleep(100000);
  }
  for (int m = 0; m < -diff_i; ++m)
  {
    char c = 'w';
    write(write_fd, &c, sizeof(c));
    usleep(100000);
  }

  for (int m = 0; m < diff_j; ++m)
  {
    char c = 'd';
    write(write_fd, &c, sizeof(c));
    usleep(100000);
  }
  for (int m = 0; m < -diff_j; ++m)
  {
    char c = 'a';
    write(write_fd, &c, sizeof(c));
    usleep(100000);
  }
}

bool CheckForObviousMines(const char *revealed_board, int write_fd)
{
  bool ret = false;
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      if (revealed_board[i * COLS + j] == ' ')
        continue;

      if (revealed_board[i * COLS + j] == 'f')
        continue;

      int8_t val = revealed_board[i * COLS + j] - '0';
      int8_t sum_unrevealed = 0;
      int8_t sum_flags = 0;
      for (int k = 0; k < num_neighbours; ++k)
      {
        // בכל איטרציה נעדכן שכן אחד אם הוא קיים (ולא נופל מחוץ ללוח)
        int neigh_row_ind = i + neighbours[k][0];
        // נבדוק שהוא לא מעל או מתחת ללוח
        if (neigh_row_ind < 0 || neigh_row_ind >= ROWS)
          continue;

        int neigh_col_ind = j + neighbours[k][1];
        // נבדוק שהוא לא משמאל או מימין ללוח
        if (neigh_col_ind < 0 || neigh_col_ind >= COLS)
          continue;

        switch (revealed_board[neigh_row_ind * COLS + neigh_col_ind])
        {
        case ' ':
          ++sum_unrevealed;
          break;
        case 'f':
          ++sum_flags;
          break;
        }
      }

      // FILE *f = fopen("/tmp/user.txt", "a");
      // fprintf(f, "i %d j %d val %d sum_unrevealed %d sum_flags %d\n", i, j, val, sum_unrevealed, sum_flags);
      // fclose(f);

      if (val == sum_flags && sum_unrevealed)
      {
        // reveal unrevealed by logic
        for (int k = 0; k < num_neighbours; ++k)
        {
          // בכל איטרציה נעדכן שכן אחד אם הוא קיים (ולא נופל מחוץ ללוח)
          int neigh_row_ind = i + neighbours[k][0];
          // נבדוק שהוא לא מעל או מתחת ללוח
          if (neigh_row_ind < 0 || neigh_row_ind >= ROWS)
            continue;

          int neigh_col_ind = j + neighbours[k][1];
          // נבדוק שהוא לא משמאל או מימין ללוח
          if (neigh_col_ind < 0 || neigh_col_ind >= COLS)
            continue;

          if (revealed_board[neigh_row_ind * COLS + neigh_col_ind] == ' ')
          {
            int diff_i = neigh_row_ind;// - pos.i;
            int diff_j = neigh_col_ind;// - pos.j;

            // f = fopen("/tmp/user.txt", "a");
            // fprintf(f, "3 di %d dj %d\n", diff_i, diff_j);
            // fclose(f);

            MoveByDiff(write_fd, diff_i, diff_j);

            // char c = ' ';
            // write(write_fd, &c, sizeof(c));
            // usleep(100000);
            return true;
          }
        }
      }
      if (val != sum_unrevealed + sum_flags)
        continue;

      // flag a mine
      for (int k = 0; k < num_neighbours; ++k)
      {
        // בכל איטרציה נעדכן שכן אחד אם הוא קיים (ולא נופל מחוץ ללוח)
        int neigh_row_ind = i + neighbours[k][0];
        // נבדוק שהוא לא מעל או מתחת ללוח
        if (neigh_row_ind < 0 || neigh_row_ind >= ROWS)
          continue;

        int neigh_col_ind = j + neighbours[k][1];
        // נבדוק שהוא לא משמאל או מימין ללוח
        if (neigh_col_ind < 0 || neigh_col_ind >= COLS)
          continue;

        if (revealed_board[neigh_row_ind * COLS + neigh_col_ind] == ' ')
        {
          // FILE *f = fopen("/tmp/user.txt", "a");
          // fprintf(f, "i %d j %d\n", neigh_row_ind, neigh_col_ind);
          // fclose(f);

          int diff_i = neigh_row_ind;// - pos.i;
          int diff_j = neigh_col_ind;// - pos.j;

          // f = fopen("/tmp/user.txt", "a");
          // fprintf(f, "di %d dj %d\n", diff_i, diff_j);
          // fclose(f);

          MoveByDiff(write_fd, diff_i, diff_j);

          char c = 'f';
          write(write_fd, &c, sizeof(c));
          usleep(100000);

          ret = true;
          break;
        }
      }
      if (ret)
        return ret;
    }
  }
  return ret;
}

bool CheckForSolution(const char *revealed_board, int write_fd)
{
  return CheckForObviousMines(revealed_board, write_fd);
}

void RevealRandomLocation(const char *revealed_board, int write_fd)
{
  int indices[ROWS * COLS];
  for (int i = 0; i < ROWS * COLS; ++i)
  {
    indices[i] = i;
  }

  for (int i = 0; i < ROWS * COLS; ++i)
  {
    int ind = i + rand() % (ROWS * COLS - i);
    int temp = indices[ind];
    indices[ind] = indices[i];
    indices[i] = temp;

    int row_ind = temp / COLS;
    int col_ind = temp % COLS;
    if (revealed_board[row_ind * COLS + col_ind] != ' ')
      continue;

    // FILE *f = fopen("/tmp/user.txt", "a");
    // fprintf(f, "1 i %d j %d\n", row_ind, col_ind);
    // fclose(f);

    int diff_i = row_ind;// - pos.i;
    int diff_j = col_ind;// - pos.j;

    // f = fopen("/tmp/user.txt", "a");
    // fprintf(f, "2 di %d dj %d\n", diff_i, diff_j);
    // fclose(f);

    MoveByDiff(write_fd, diff_i, diff_j);

    char c = ' ';
    write(write_fd, &c, sizeof(c));
    break;
  }
}

int CheckEnd(const char *revealed_board)
{
  bool ret = false;
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      if (revealed_board[i * COLS + j] == '*')
        return -1; // lose
    }
  }

  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      if (revealed_board[i * COLS + j] == ' ')
        return 0; // not end
    }
  }

  return 1; // win
}

void run_user(int sock)
{
  // struct all_fds all_fds = *(struct all_fds *)arguments;

  char revealed_board[ROWS * COLS];

  for (int i = 0;; ++i)
  {
    // TODO (oc): get pos too
    bool get_message(int sock, int wanted_type, void *data) {

    if (!get_message(sock, 0, revealed_board))
    {
      usleep(1000);
      continue;
    }

    if (CheckEnd(revealed_board))
      break;

    // FILE *f = fopen("/tmp/user.txt", "a");
    // fprintf(f, "num_read %d\n", num_read);

    // for (int m = 0; m < ROWS; ++m)
    // {
    //   for (int j = 0; j < COLS; ++j)
    //   {
    //     fprintf(f, "----");
    //   }
    //   fprintf(f, "-\n");
    //   for (int n = 0; n < COLS; ++n)
    //   {
    //     fprintf(f, "| %c ", revealed_board[m * COLS + n]);
    //   }
    //   fprintf(f, "|\n");
    // }
    // for (int j = 0; j < COLS; ++j)
    // {
    //   fprintf(f, "----");
    // }
    // fprintf(f, "-\n");

    // fprintf(f, "before CheckForSolution\n");
    // fclose(f);

    if (CheckForSolution(revealed_board, sock))
      continue;

    // f = fopen("/tmp/user.txt", "a");
    // fprintf(f, "before RevealRandomLocation\n");
    // fclose(f);
    RevealRandomLocation(revealed_board, sock);
  }

  char c = 'q';
  write(sock, &c, sizeof(c));
  usleep(100000);
}

int main()
{
  int sock = 0;
  struct sockaddr_in serv_addr;
  char *message = "Hello from client";

  // Creating socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  // Connect to server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }

  run_user(sock);

  // Close the socket
  close(sock);

  return 0;
}