#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common/common.h"
#include "patterns.h"

// 'X' is out of board or number
// 'Y' is out of board or number or space
// 'S' is where we should flag
// 'E' is where we should reveal
// 'R' can be anything

static int8_t cell_with_neighbours[49][2];

static int8_t num_cell_with_neighbours = sizeof(cell_with_neighbours) / sizeof(cell_with_neighbours[0]);

struct User
{
  char **patterns;
  size_t patterns_len;
  char *revealed_board;
  int num_flags;
  struct Position pos;
  struct GameConfig config;
};

void MoveByDiff(int sock, int diff_i, int diff_j)
{
  for (int m = 0; m < diff_i; ++m)
  {
    char c = 'x';
    send_message(sock, 1, &c, -1);
  }
  for (int m = 0; m < -diff_i; ++m)
  {
    char c = 'w';
    send_message(sock, 1, &c, -1);
  }

  for (int m = 0; m < diff_j; ++m)
  {
    char c = 'd';
    send_message(sock, 1, &c, -1);
  }
  for (int m = 0; m < -diff_j; ++m)
  {
    char c = 'a';
    send_message(sock, 1, &c, -1);
  }
}

bool CheckForObviousMines(struct User *user, int sock)
{
  for (int i = 0; i < user->config.rows; ++i)
  {
    for (int j = 0; j < user->config.cols; ++j)
    {
      if (user->revealed_board[i * user->config.cols + j] == ' ')
        continue;

      if (user->revealed_board[i * user->config.cols + j] == 'f')
        continue;

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

            MoveByDiff(sock, diff_i, diff_j);

            char c = ' ';
            send_message(sock, 1, &c, -1);
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
          int diff_i = neigh_row_ind - user->pos.i;
          int diff_j = neigh_col_ind - user->pos.j;

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

          // printf("found %d,%d i %d, j %d val %c sum_unrevealed %d sum_flags %d\n", neigh_row_ind, neigh_col_ind, i, j, val, sum_unrevealed, sum_flags);
          // fflush(stdout);
          // getchar();
          char c = 'f';
          send_message(sock, 1, &c, -1);
          user->num_flags++;
          return true;
        }
      }
    }
  }
  return false;
}

bool CheckForAllMines(struct User *user, int sock)
{
  // return false;
  // search for 1,2,1 horizontally
  for (int i = 0; i < user->config.rows; ++i)
  {
    for (int j = 0; j < user->config.cols; ++j)
    {
      for (int p = 0; p < user->patterns_len; ++p)
      {
        bool ok = true;
        int8_t should_flag = -1;
        int8_t should_reveal = -1;
        int k = 0;
        for (int neigh_row_ind = i - 7 / 2; neigh_row_ind < i + 7 / 2 + 1; neigh_row_ind++)
        {
          for (int neigh_col_ind = j - 7 / 2; neigh_col_ind < j + 7 / 2 + 1; neigh_col_ind++, ++k)
          {
            // printf("p %d k %d\n", p, k);
            // printf("%c\n", user->patterns[p][k]);
            char neigh_val;
            switch (user->patterns[p][k])
            {
            case 'R':
              break;
            case 'X':
              if (neigh_row_ind < 0 || neigh_row_ind >= user->config.rows || neigh_col_ind < 0 || neigh_col_ind >= user->config.cols)
                break;
              neigh_val = user->revealed_board[neigh_row_ind * user->config.cols + neigh_col_ind];
              if (neigh_val < '0' || neigh_val > '8')
                ok = false;
              break;
            case 'Y':
              if (neigh_row_ind < 0 || neigh_row_ind >= user->config.rows || neigh_col_ind < 0 || neigh_col_ind >= user->config.cols)
                break;
              neigh_val = user->revealed_board[neigh_row_ind * user->config.cols + neigh_col_ind];
              if (neigh_val != ' ' && (neigh_val < '0' || neigh_val > '8'))
                ok = false;
              break;
            case 'I':
              if (user->num_flags != user->config.mines - 1)
              {
                ok = false;
                break;
              }
              // FALLTHROUGH
            case 'S':
              if (neigh_row_ind < 0 || neigh_row_ind >= user->config.rows || neigh_col_ind < 0 || neigh_col_ind >= user->config.cols)
              {
                ok = false;
                break;
              }
              if (user->revealed_board[neigh_row_ind * user->config.cols + neigh_col_ind] != ' ')
              {
                ok = false;
                break;
              }
              should_flag = k;
              break;
            case 'E':
              if (neigh_row_ind < 0 || neigh_row_ind >= user->config.rows || neigh_col_ind < 0 || neigh_col_ind >= user->config.cols)
              {
                ok = false;
                break;
              }
              if (user->revealed_board[neigh_row_ind * user->config.cols + neigh_col_ind] != ' ')
              {
                ok = false;
                break;
              }
              should_reveal = k;
              break;
            case ' ':
            // FALLTHROUGH
            case 'f':
            // FALLTHROUGH
            case '1':
            // FALLTHROUGH
            case '2':
            // FALLTHROUGH
            case '3':
              // FALLTHROUGH
            case '4':
              // FALLTHROUGH
            case '5':
              // FALLTHROUGH
            case '6':
              // FALLTHROUGH
            case '7':
              // FALLTHROUGH
            case '8':
              if (neigh_row_ind < 0 || neigh_row_ind >= user->config.rows || neigh_col_ind < 0 || neigh_col_ind >= user->config.cols)
              {
                ok = false;
                break;
              }
              if (user->revealed_board[neigh_row_ind * user->config.cols + neigh_col_ind] != user->patterns[p][k])
                ok = false;
              break;
            }
            if (!ok)
              break;
          }
        }
        if (!ok)
          continue;

        int ind = should_flag >= 0 ? should_flag : should_reveal;

        int neigh_row_ind = i + cell_with_neighbours[ind][0];
        int neigh_col_ind = j + cell_with_neighbours[ind][1];

        // if (p > 3)
        // {
        //   printf("found %d,%d %d", i, j, p);
        //   fflush(stdout);
        //   getchar();
        // }

        int diff_i = neigh_row_ind - user->pos.i;
        int diff_j = neigh_col_ind - user->pos.j;

        MoveByDiff(sock, diff_i, diff_j);

        char c = should_flag >= 0 ? 'f' : ' ';
        send_message(sock, 1, &c, -1);
        if (c == 'f')
          user->num_flags++;
        // printf("c %c\n", c);
        // printf("p %d\n", p);

        // for (int ii = 0; ii < 7; ii++)
        // {
        //   for (int jj = 0; jj < 7; jj++)
        //   {
        //     printf("%c", user->patterns[p][ii * 7 + jj]);
        //   }
        //   printf("\n");
        // }
        return true;
      }
    }
  }
  return false;
}

bool CheckForSolution(struct User *user, int sock)
{
  if (CheckForObviousMines(user, sock))
  {
    // printf("obvious\n");
    return true;
  }
  // printf("all\n");
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

    int row_ind = temp / user->config.cols;
    int col_ind = temp % user->config.cols;
    int diff_i = row_ind - user->pos.i;
    int diff_j = col_ind - user->pos.j;

    MoveByDiff(sock, diff_i, diff_j);

    char c = ' ';
    send_message(sock, 1, &c, -1);
    break;
  }

  free(indices);
}

enum EndGame
{
  Lose = -1,
  Not,
  Win
};

void parse_board_message(char *msg, int len, struct User *user)
{
  // printf("len %d\n", len);
  int num_cells = (len - sizeof(struct Position)) / sizeof(struct Cell);
  char *ptr = msg;
  struct Cell cell;
  for (int i = 0; i < num_cells; ++i)
  {
    cell.pos.i = *ptr++;
    cell.pos.j = *ptr++;
    cell.val = *ptr++;
    user->revealed_board[cell.pos.i * user->config.cols + cell.pos.j] = cell.val;
  }

  memcpy(&user->pos, ptr, sizeof(user->pos));
}

bool AlreadyExists(char **patterns, int len, char *curr)
{
  for (int i = 0; i < len; ++i)
    if (!memcmp(patterns[i], curr, 49))
      return true;
  return false;
}

void CreatePatterns(struct User *user)
{
  size_t pattern_len = sizeof(patterns[0]);
  int ind = 0;
  user->patterns = (char **)malloc(patterns_len * rotations_len * sizeof(*user->patterns));
  char curr_rotated_pattern[sizeof(patterns[0])] = {0};
  for (int i = 0; i < patterns_len; ++i)
  {
    char *curr_pattern = patterns[i];
    for (int j = 0; j < rotations_len; ++j)
    {
      int8_t *curr_rot = rotations[j];

      for (int k = 0; k < pattern_len; ++k)
        curr_rotated_pattern[k] = curr_pattern[curr_rot[k]];

      if (AlreadyExists(user->patterns, ind, curr_rotated_pattern))
        continue;

      user->patterns[ind] = (char *)calloc(pattern_len, 1);
      memcpy(user->patterns[ind], curr_rotated_pattern, pattern_len);
      ind++;
    }
  }
  user->patterns_len = ind;
}

void Init(struct User *user)
{
  int num_cells = user->config.rows * user->config.cols;
  user->revealed_board = (char *)malloc(num_cells);
  for (int i = 0; i < num_cells; ++i)
    user->revealed_board[i] = ' ';

  CreatePatterns(user);

  for (int i = 0; i < 7; ++i)
  {
    for (int j = 0; j < 7; ++j)
    {
      cell_with_neighbours[i * 7 + j][0] = i - 7 / 2;
      cell_with_neighbours[i * 7 + j][1] = j - 7 / 2;
    }
  }
  user->num_flags = 0;
}

void DeInit(struct User *user)
{
  free(user->revealed_board);
  for (int i = 0; i < user->patterns_len; ++i)
    free(user->patterns[i]);
  free(user->patterns);
}

void run_one_game(int sock, struct User *user)
{
  int num_cells = user->config.rows * user->config.cols;

  Init(user);

  // printf("user->patterns_len %zu\n", user->patterns_len);
  // for (int i = 0; i < user->patterns_len; ++i)
  // {
  //   printf("'%s'\n", user->patterns[i]);
  // }
  // fflush(stdout);
  // // return;

  char *msg = (char *)malloc(num_cells * sizeof(struct Cell) + sizeof(user->pos));

  bool random_reveal = false;

  for (int iter = 0;; ++iter)
  {
    int8_t msg_type;
    int len;
    if (!get_message(sock, &msg_type, msg, &len))
    {
      usleep(10);
      continue;
    }

    enum EndGame end_game = Not;
    char *ptr = msg;
    switch (msg_type)
    {
    case 0:
      parse_board_message(msg, len, user);
      break;
    case 2:
      end_game = msg[0] ? Win : Lose;
      // if (!random_reveal && end_game == Lose)
      // {
      //   printf("lost after random reveal\n");
      //   exit(-1);
      // }
      break;
    }

    if (end_game)
    {
      FILE *f = fopen("/Users/user/work/tutorials/mine_sweeper/user.csv", "a");
      fprintf(f, "end_game,%d,rows,%d,cols,%d,mines,%d\n", end_game, user->config.rows, user->config.cols, user->config.mines);
      fclose(f);
      break;
    }

    // for (int m = 0; m < user->config.rows; ++m)
    // {
    //   for (int j = 0; j < user->config.cols; ++j)
    //   {
    //     printf("----");
    //   }
    //   printf("-\n");
    //   for (int n = 0; n < user->config.cols; ++n)
    //   {
    //     printf("| %c ", user->revealed_board[m * user->config.cols + n]);
    //   }
    //   printf("|\n");
    // }
    // for (int j = 0; j < user->config.cols; ++j)
    // {
    //   printf("----");
    // }
    // printf("-\n");

    if (CheckForSolution(user, sock))
    {
      random_reveal = false;
      continue;
    }

    // printf("revealing random %d\n", iter);
    // getchar();
    random_reveal = true;
    RevealRandomLocation(user, sock);
  }

  free(msg);
  DeInit(user);
}

void run_user(int sock, struct User *user)
{
  for (int i = 0; i < 10000; ++i)
  {
    if (i % 100 == 1)
      printf("\r %d%%", i / 100);
    fflush(stdout);
    send_message(sock, 3, &user->config, -1);

    run_one_game(sock, user);
  }

  char c = 'q';
  send_message(sock, 1, &c, -1);
  // usleep(10000);
}

int main(int argc, char *argv[])
{
  struct User user;

  switch (argv[1][0])
  {
  case '1':
    user.config.rows = 9;
    user.config.cols = 9;
    user.config.mines = 10;
    break;
  case '2':
    user.config.rows = 16;
    user.config.cols = 16;
    user.config.mines = 40;
    break;
  case '3':
    user.config.rows = 16;
    user.config.cols = 30;
    user.config.mines = 99;
    break;
  }

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

  run_user(sock, &user);

  // Close the socket
  close(sock);

  return 0;
}