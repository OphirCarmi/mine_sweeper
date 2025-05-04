#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common/common.h"

struct User
{
  char *revealed_board;
  struct Position pos;
  struct GameConfig config;
};

void MoveByDiff(int sock, int diff_i, int diff_j)
{
  for (int m = 0; m < diff_i; ++m)
  {
    char c = 'x';
    send_message(sock, 1, &c, -1);
#ifdef SHOW
    usleep(100000);
#endif // SHOW
  }
  for (int m = 0; m < -diff_i; ++m)
  {
    char c = 'w';
    send_message(sock, 1, &c, -1);
#ifdef SHOW
    usleep(100000);
#endif // SHOW
  }

  for (int m = 0; m < diff_j; ++m)
  {
    char c = 'd';
    send_message(sock, 1, &c, -1);
#ifdef SHOW
    usleep(100000);
#endif // SHOW
  }
  for (int m = 0; m < -diff_j; ++m)
  {
    char c = 'a';
    send_message(sock, 1, &c, -1);
#ifdef SHOW
    usleep(100000);
#endif // SHOW
  }
}

bool CheckForObviousMines(const struct User *user, int sock)
{
  for (int i = 0; i < user->config.rows; ++i)
  {
    for (int j = 0; j < user->config.cols; ++j)
    {
      if (user->revealed_board[i * user->config.cols + j] == ' ')
        continue;

      if (user->revealed_board[i * user->config.cols + j] == 'f')
        continue;

      // if (revealed_board[i * COLS + j] == '*')
      //   return true;

      int8_t val = user->revealed_board[i * user->config.cols + j] - '0';
      int8_t sum_unrevealed = 0;
      int8_t sum_flags = 0;
      for (int k = 0; k < num_neighbours; ++k)
      {
        // בכל איטרציה נעדכן שכן אחד אם הוא קיים (ולא נופל מחוץ ללוח)
        int neigh_row_ind = i + neighbours[k][0];
        // נבדוק שהוא לא מעל או מתחת ללוח
        if (neigh_row_ind < 0 || neigh_row_ind >= user->config.rows)
          continue;

        int neigh_col_ind = j + neighbours[k][1];
        // נבדוק שהוא לא משמאל או מימין ללוח
        if (neigh_col_ind < 0 || neigh_col_ind >= user->config.cols)
          continue;

        switch (user->revealed_board[neigh_row_ind * user->config.cols + neigh_col_ind])
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
          if (neigh_row_ind < 0 || neigh_row_ind >= user->config.rows)
            continue;

          int neigh_col_ind = j + neighbours[k][1];
          // נבדוק שהוא לא משמאל או מימין ללוח
          if (neigh_col_ind < 0 || neigh_col_ind >= user->config.cols)
            continue;

          if (user->revealed_board[neigh_row_ind * user->config.cols + neigh_col_ind] == ' ')
          {
            int diff_i = neigh_row_ind - user->pos.i;
            int diff_j = neigh_col_ind - user->pos.j;

            // f = fopen("/tmp/user.txt", "a");
            // fprintf(f, "3 di %d dj %d\n", diff_i, diff_j);
            // fclose(f);

            MoveByDiff(sock, diff_i, diff_j);

            char c = ' ';
            send_message(sock, 1, &c, -1);
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
        if (neigh_row_ind < 0 || neigh_row_ind >= user->config.rows)
          continue;

        int neigh_col_ind = j + neighbours[k][1];
        // נבדוק שהוא לא משמאל או מימין ללוח
        if (neigh_col_ind < 0 || neigh_col_ind >= user->config.cols)
          continue;

        if (user->revealed_board[neigh_row_ind * user->config.cols + neigh_col_ind] == ' ')
        {
          // FILE *f = fopen("/tmp/user.txt", "a");
          // fprintf(f, "i %d j %d\n", neigh_row_ind, neigh_col_ind);
          // fclose(f);

          int diff_i = neigh_row_ind - user->pos.i;
          int diff_j = neigh_col_ind - user->pos.j;

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
          send_message(sock, 1, &c, -1);
          // usleep(10000);

          return true;
        }
      }
    }
  }
  return false;
}

bool CheckForAllMines(const struct User *user, int sock)
{
  // return false;
  // search for 1,2,1 horizontally
  for (int i = 0; i < user->config.rows; ++i)
  {
    for (int j = 0; j < user->config.cols; ++j)
    {
      if (user->revealed_board[i * user->config.cols + j] != '2')
        continue;

      // 'X' is out of board or number
      char *expected[4] = {"XXX11   ", "   11XXX", " 1X X 1X", "X1 X X1 "};//, "XXX12   ", "XXX21   ", "   12XXX", "   21XXX", " 1X X 2X", " 2X X 1X", "X2 X X1 ", "X1 X X2 "};
      int flag_positions[4] = {5, 0, 0, 2};//, 6, 5, 1, 0, 3, 0, 2, 4};

      for (int p = 0; p < 4; ++p)
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
            if (neigh_row_ind < 0 || neigh_row_ind >= user->config.rows || neigh_col_ind < 0 || neigh_col_ind >= user->config.cols)
            {
              break;
            }
            char neigh_val = user->revealed_board[neigh_row_ind * user->config.cols + neigh_col_ind];
            if (neigh_val < '0' || neigh_val > '8')
              ok = false;
            break;
          case ' ':
          // FALLTHROUGH
          case '1':
          // // FALLTHROUGH
          // case '2':
            if (neigh_row_ind < 0 || neigh_row_ind >= user->config.rows || neigh_col_ind < 0 || neigh_col_ind >= user->config.cols)
            {
              ok = false;
              break;
            }
            if (user->revealed_board[neigh_row_ind * user->config.cols + neigh_col_ind] != expected[p][k])
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

        int diff_i = neigh_row_ind - user->pos.i;
        int diff_j = neigh_col_ind - user->pos.j;

        MoveByDiff(sock, diff_i, diff_j);

        char c = 'f';
        send_message(sock, 1, &c, -1);
        // usleep(1000000);

        return true;
      }
    }
  }
  return false;
}

bool CheckForSolution(const struct User *user, int sock)
{
  if (CheckForObviousMines(user, sock))
    return true;
  return CheckForAllMines(user, sock);
}

void RevealRandomLocation(const struct User *user, int sock)
{
  int num_cells = user->config.rows * user->config.cols;
  int *indices = (int *)malloc(sizeof(*indices) * num_cells);
  int num_unrevealed = 0;
  for (int i = 0; i < num_cells; ++i)
  {
    if (user->revealed_board[i] != ' ')
      continue;

    // check for neigbours without numbers greater than 0
    int k = 0;
    for (; k < num_neighbours; ++k)
    {
      int neigh_row_ind = i / user->config.cols + neighbours[k][0];
      int neigh_col_ind = i % user->config.cols + neighbours[k][1];
      if (neigh_row_ind < 0 || neigh_row_ind >= user->config.rows || neigh_col_ind < 0 || neigh_col_ind >= user->config.cols)
        continue;
      char val = user->revealed_board[neigh_row_ind * user->config.cols + neigh_col_ind];
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
    for (int i = 0; i < num_cells; ++i)
    {
      if (user->revealed_board[i] != ' ')
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

    int row_ind = temp / user->config.cols;
    int col_ind = temp % user->config.cols;
    int diff_i = row_ind - user->pos.i;
    int diff_j = col_ind - user->pos.j;

    // f = fopen("/tmp/user.txt", "a");
    // fprintf(f, "2 di %d dj %d\n", diff_i, diff_j);
    // fclose(f);

    MoveByDiff(sock, diff_i, diff_j);

    char c = ' ';
    send_message(sock, 1, &c, -1);
    break;
  }
}

enum EndGame
{
  Lose = -1,
  Not,
  Win
};

void run_one_game(int sock, struct User *user)
{
  int num_cells = user->config.rows * user->config.cols;

  user->revealed_board = (char *)malloc(num_cells);

  char *msg = (char *)malloc(num_cells + sizeof(user->pos));

  for (;;)
  {
    int msg_type;
    if (!get_message(sock, &msg_type, msg))
    {
      usleep(10);
      continue;
    }

    enum EndGame end_game = Not;
    char *ptr = msg;
    switch (msg_type)
    {
    case 0:
      memcpy(user->revealed_board, ptr, num_cells);
      ptr += num_cells;
      memcpy(&user->pos, ptr, sizeof(user->pos));
      break;
    case 2:
      end_game = msg[0] ? Win : Lose;
      break;
    }

    if (end_game)
    {
      FILE *f = fopen("/Users/user/work/tutorials/mine_sweeper/user.csv", "a");
      fprintf(f, "end_game,%d,rows,%d,cols,%d,mines,%d\n", end_game, user->config.rows, user->config.cols, user->config.mines);
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

    if (CheckForSolution(user, sock))
      continue;

    RevealRandomLocation(user, sock);
  }

  free(msg);
  free(user->revealed_board);
}

void run_user(int sock)
{
  struct User user;
  for (int i = 0; i < 10000; ++i)
  {
    user.config.rows = rand() % 10 + 8;
    user.config.cols = rand() % 10 + 8;
    user.config.mines = user.config.rows * user.config.cols / 6;

    send_message(sock, 3, &user.config, -1);

    run_one_game(sock, &user);
  }

  char c = 'q';
  send_message(sock, 1, &c, -1);
  // usleep(10000);
}

int main()
{
  int sock = 0;
  struct sockaddr_in serv_addr;

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