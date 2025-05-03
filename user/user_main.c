#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common/common.h"

static struct Position pos;

void MoveByDiff(int sock, int diff_i, int diff_j)
{
  for (int m = 0; m < diff_i; ++m)
  {
    char c = 'x';
    send_message(sock, 1, &c);
    // usleep(10000);
  }
  for (int m = 0; m < -diff_i; ++m)
  {
    char c = 'w';
    send_message(sock, 1, &c);
    // usleep(10000);
  }

  for (int m = 0; m < diff_j; ++m)
  {
    char c = 'd';
    send_message(sock, 1, &c);
    // usleep(10000);
  }
  for (int m = 0; m < -diff_j; ++m)
  {
    char c = 'a';
    send_message(sock, 1, &c);
    // usleep(10000);
  }
}

bool CheckForObviousMines(const char *revealed_board, int sock)
{
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      if (revealed_board[i * COLS + j] == ' ')
        continue;

      if (revealed_board[i * COLS + j] == 'f')
        continue;

      // if (revealed_board[i * COLS + j] == '*')
      //   return true;

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
            int diff_i = neigh_row_ind - pos.i;
            int diff_j = neigh_col_ind - pos.j;

            // f = fopen("/tmp/user.txt", "a");
            // fprintf(f, "3 di %d dj %d\n", diff_i, diff_j);
            // fclose(f);

            MoveByDiff(sock, diff_i, diff_j);

            char c = ' ';
            send_message(sock, 1, &c);
            // usleep(10000);
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

          int diff_i = neigh_row_ind - pos.i;
          int diff_j = neigh_col_ind - pos.j;

          // f = fopen("/tmp/user.txt", "a");
          // fprintf(f, "di %d dj %d\n", diff_i, diff_j);
          // fclose(f);

          MoveByDiff(sock, diff_i, diff_j);

          // FILE *f = fopen("/tmp/user.txt", "a");
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

          // fprintf(f, "pos %d,%d\n", pos.i, pos.j);

          // fclose(f);

          // printf("f in (%d, %d)\n", neigh_row_ind, neigh_col_ind);
          // getc(stdin);

          char c = 'f';
          send_message(sock, 1, &c);
          // usleep(10000);

          return true;
        }
      }
    }
  }
  return false;
}

bool CheckForAllMines(const char *revealed_board, int sock)
{
  // return false;
  // search for 1,2,1 horizontally
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      if (revealed_board[i * COLS + j] != '2')
        continue;

      // 'X' is out of board or number
      char *expected[12] = {"XXX11   ", "   11XXX", " 1X X 1X", "X1 X X1 ", "XXX12   ", "XXX21   ", "   12XXX", "   21XXX", " 1X X 2X", " 2X X 1X", "X2 X X1 ", "X1 X X2 "};
      int flag_positions[12] = {5, 0, 0, 2, 6, 5, 1, 0, 3, 0, 2, 4};

      for (int p = 0; p < 12; ++p)
      {
        bool ok = true;
        for (int k = 0; k < num_neighbours; ++k)
        {
          int neigh_row_ind = i + neighbours[k][0];
          int neigh_col_ind = j + neighbours[k][1];
          // נבדוק שהוא לא מעל או מתחת ללוח
          switch (expected[p][k])
          {
          case 'X':
            if (neigh_row_ind < 0 || neigh_row_ind >= ROWS || neigh_col_ind < 0 || neigh_col_ind >= COLS)
            {
              break;
            }
            char neigh_val = revealed_board[neigh_row_ind * COLS + neigh_col_ind];
            if (neigh_val < '0' || neigh_val > '8')
              ok = false;
            break;
          case ' ':
          // FALLTHROUGH
          case '1':
          // FALLTHROUGH
          case '2':
            if (neigh_row_ind < 0 || neigh_row_ind >= ROWS || neigh_col_ind < 0 || neigh_col_ind >= COLS)
            {
              ok = false;
              break;
            }
            if (revealed_board[neigh_row_ind * COLS + neigh_col_ind] != expected[p][k])
              ok = false;
            break;
          }
          if (!ok)
            break;
        }
        if (!ok)
          continue;

        int neigh_row_ind = i + neighbours[flag_positions[p]][0];
        int neigh_col_ind = j + neighbours[flag_positions[p]][1];

        // if (p >= 4) {
        //   printf("found %d,%d %d", i, j, p);
        //   getchar();
        // }

        int diff_i = neigh_row_ind - pos.i;
        int diff_j = neigh_col_ind - pos.j;

        MoveByDiff(sock, diff_i, diff_j);

        char c = 'f';
        send_message(sock, 1, &c);
        // usleep(1000000);

        return true;
      }
    }
  }
  return false;
}

bool CheckForSolution(const char *revealed_board, int sock)
{
  if (CheckForObviousMines(revealed_board, sock))
    return true;
  return CheckForAllMines(revealed_board, sock);
}

void RevealRandomLocation(const char *revealed_board, int sock)
{
  int indices[NUM_CELLS];
  int num_unrevealed = 0;
  for (int i = 0; i < NUM_CELLS; ++i)
  {
    if (revealed_board[i] != ' ')
      continue;

    // check for neigbours without numbers greater than 0
    int k = 0;
    for (; k < num_neighbours; ++k)
    {
      int neigh_row_ind = i / COLS + neighbours[k][0];
      int neigh_col_ind = i % COLS + neighbours[k][1];
      if (neigh_row_ind < 0 || neigh_row_ind >= ROWS || neigh_col_ind < 0 || neigh_col_ind >= COLS)
        continue;
      char val = revealed_board[neigh_row_ind * COLS + neigh_col_ind];
      if (val >= '1' && val <= '8')
        break;
    }
    if (k != num_neighbours)
      continue;
    indices[num_unrevealed] = i;
    ++num_unrevealed;
  }

  if (!num_unrevealed)
  {
    for (int i = 0; i < NUM_CELLS; ++i)
    {
      if (revealed_board[i] != ' ')
        continue;

      indices[num_unrevealed] = i;
      ++num_unrevealed;
    }
  }

  for (int i = 0; i < num_unrevealed; ++i)
  {
    int ind = i + rand() % (num_unrevealed - i);
    int temp = indices[ind];
    indices[ind] = indices[i];
    indices[i] = temp;

    // FILE *f = fopen("/tmp/user.txt", "a");
    // fprintf(f, "1 i %d j %d\n", row_ind, col_ind);
    // fclose(f);

    int row_ind = temp / COLS;
    int col_ind = temp % COLS;
    int diff_i = row_ind - pos.i;
    int diff_j = col_ind - pos.j;

    // f = fopen("/tmp/user.txt", "a");
    // fprintf(f, "2 di %d dj %d\n", diff_i, diff_j);
    // fclose(f);

    MoveByDiff(sock, diff_i, diff_j);

    char c = ' ';
    send_message(sock, 1, &c);
    break;
  }
}

enum EndGame
{
  Lose = -1,
  Not,
  Win
};

void run_one_game(int sock)
{
  char revealed_board[NUM_CELLS];

  char msg[NUM_CELLS + sizeof(pos)];

  for (;;)
  {
    int msg_type;
    if (!get_message(sock, &msg_type, &msg))
    {
      usleep(10);
      continue;
    }

    enum EndGame end_game = Not;
    char *ptr = msg;
    switch (msg_type)
    {
    case 0:
      memcpy(revealed_board, ptr, NUM_CELLS);
      ptr += NUM_CELLS;
      memcpy(&pos, ptr, sizeof(pos));
      break;
    case 2:
      end_game = msg[0] ? Win : Lose;
      break;
    }

    // FILE *f = fopen("/tmp/user.txt", "a");
    // fprintf(f, "msg_type %d\n", msg_type);
    // fclose(f);

    if (end_game)
    {
      FILE *f = fopen("/Users/user/work/tutorials/mine_sweeper/user.txt", "a");
      fprintf(f, "end_game %d\n", end_game);
      fclose(f);
      break;
    }
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
}

void run_user(int sock)
{
  for (int i = 0; i < 1000; ++i)
  {
    run_one_game(sock);
  }

  char c = 'q';
  send_message(sock, 1, &c);
  // usleep(10000);
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