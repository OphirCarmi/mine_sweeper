#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#include "common/common.h"
#include "patterns.h"

static int8_t cell_with_neighbours[49][2];

static int8_t num_cell_with_neighbours = sizeof(cell_with_neighbours) / sizeof(cell_with_neighbours[0]);

struct Step
{
  struct Position pos;
  bool reveal;
};

struct User
{
  char **patterns;
  size_t patterns_len;
  char *revealed_board;
  struct Step *steps;
  int num_steps;
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
  int possible_empty[49];
  for (int i = 0; i < user->config.rows; ++i)
  {
    for (int j = 0; j < user->config.cols; ++j)
    {
      for (int p = 0; p < user->patterns_len; ++p)
      {
        bool ok = true;
        int8_t should_flag = -1;
        int possible_empty_len = 0;
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
              possible_empty[possible_empty_len++] = k;
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

        int ind = should_flag;
        if (ind < 0)
        {
          if (possible_empty_len == 1)
          {
            ind = possible_empty[0];
          }
          else
          {
            ind = possible_empty[rand() % possible_empty_len];
          }
        }

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

  free(user->steps);
}

// An iterative binary search function.
int binarySearch(int *arr, int low, int high, int x)
{
  while (low <= high)
  {
    int mid = low + (high - low) / 2;

    // Check if x is present at mid
    if (arr[mid] == x)
      return mid;

    // If x greater, ignore left half
    if (arr[mid] < x)
      low = mid + 1;

    // If x is smaller, ignore right half
    else
      high = mid - 1;
  }

  // If we reach here, then element was not present
  return -1;
}

void max_entropy_solution(int sock, struct User *user)
{
  int num_cells = user->config.rows * user->config.cols;
  int unrevealed_cnt = 0;
  // get unrevealed count
  for (int i = 0; i < num_cells; ++i)
  {
    unrevealed_cnt += user->revealed_board[i] == ' ';
  }

  int *unrevealed_indices = (int *)malloc(unrevealed_cnt * sizeof(*unrevealed_indices));
  int k = 0;
  // get unrevealed indices
  // printf("unrevealed_indices ");
  for (int i = 0; i < num_cells; ++i)
  {
    if (user->revealed_board[i] != ' ')
      continue;
    unrevealed_indices[k++] = i;
    // printf("%d,", i);
  }
  // printf("\n");

  double *p = (double *)malloc(unrevealed_cnt * sizeof(*p));
  double *last_p = (double *)malloc(unrevealed_cnt * sizeof(*last_p));
  double *q = (double *)malloc(unrevealed_cnt * sizeof(*q));

  // set to 1 as we know the number of total mines
  int constraint_cnt = 1;
  // get constraint count
  for (int i = 0; i < num_cells; ++i)
  {
    char val = user->revealed_board[i];
    constraint_cnt += val >= '1' && val <= '8';
  }
  double *c = (double *)malloc(constraint_cnt * sizeof(*c));

  int *constraint_indices = (int *)malloc((constraint_cnt - 1) * sizeof(*constraint_indices));

  k = 0;
  // printf("c ");
  for (int i = 0; i < num_cells; ++i)
  {
    char val = user->revealed_board[i];
    if (val < '1' || val > '8')
      continue;
    c[k] = val - '0';
    // printf("%g,", c[k]);
    constraint_indices[k++] = i;
  }

  c[k] = user->config.mines - user->num_flags;

  // printf("%g,", c[k]);
  // printf("\n");

  for (int i = 0; i < unrevealed_cnt; ++i)
  {
    p[i] = 1.;
    q[i] = 1.;
  }

  for (int algo_iter = 0;algo_iter < 1000; ++algo_iter)
  {
    // printf("algo_iter %d\n", algo_iter);
    for (int i = 0; i < constraint_cnt - 1; ++i)
    {
      int curr_constrait_ind = constraint_indices[i];
      double curr_constraint_val = c[i];
      int curr_constraint_row = curr_constrait_ind / user->config.cols;
      int curr_constraint_col = curr_constrait_ind % user->config.cols;

      double sum_p = 0.;
      double sum_q = 0.;
      double num_unrevealed_neigh = 0;
      for (int m = curr_constraint_row - 1; m <= curr_constraint_row + 1; ++m)
      {
        if (m < 0 || m >= user->config.rows)
          continue;
        for (int n = curr_constraint_col - 1; n <= curr_constraint_col + 1; ++n)
        {
          if (n < 0 || n >= user->config.cols)
            continue;
          int neigh_ind = m * user->config.cols + n;
          char neigh_val = user->revealed_board[neigh_ind];
          if (neigh_val == 'f')
          {
            curr_constraint_val--;
            continue;
          }

          if (neigh_val != ' ')
            continue;
          // printf("A neigh_ind %d\n", neigh_ind);
          int neigh_ind2 = binarySearch(unrevealed_indices, 0, unrevealed_cnt - 1, neigh_ind);
          // printf("A neigh_ind2 %d\n", neigh_ind2);
          if (neigh_ind2 < 0)
            exit(-3);
          // printf("p[neigh_ind2] %g\n", p[neigh_ind2]);
          sum_p += p[neigh_ind2];
          sum_q += q[neigh_ind2];
          num_unrevealed_neigh++;
        }
      }

      // printf("curr_constraint_val %g\n", curr_constraint_val);
      // printf("num_unrevealed_neigh %g\n", num_unrevealed_neigh);
      if (curr_constraint_val == 0.)
        continue;
      // printf("sum_p %g\n", sum_p);
      // printf("sum_q %g\n", sum_q);

      if (fabs(sum_p - curr_constraint_val) > 0.001)
      {
        for (int m = curr_constraint_row - 1; m <= curr_constraint_row + 1; ++m)
        {
          if (m < 0 || m >= user->config.rows)
            continue;
          for (int n = curr_constraint_col - 1; n <= curr_constraint_col + 1; ++n)
          {
            if (n < 0 || n >= user->config.cols)
              continue;
            int neigh_ind = m * user->config.cols + n;
            if (user->revealed_board[neigh_ind] != ' ')
              continue;
            // printf("B neigh_ind %d\n", neigh_ind);
            int neigh_ind2 = binarySearch(unrevealed_indices, 0, unrevealed_cnt - 1, neigh_ind);
            // printf("B neigh_ind2 %d\n", neigh_ind2);
            if (neigh_ind2 < 0)
              exit(-3);
            p[neigh_ind2] *= curr_constraint_val / sum_p;
          }
        }
      }

      if (fabs(sum_q - num_unrevealed_neigh + curr_constraint_val) > 0.001)
      {
        for (int m = curr_constraint_row - 1; m <= curr_constraint_row + 1; ++m)
        {
          if (m < 0 || m >= user->config.rows)
            continue;
          for (int n = curr_constraint_col - 1; n <= curr_constraint_col + 1; ++n)
          {
            if (n < 0 || n >= user->config.cols)
              continue;
            int neigh_ind = m * user->config.cols + n;
            if (user->revealed_board[neigh_ind] != ' ')
              continue;
            // printf("C neigh_ind %d\n", neigh_ind);
            int neigh_ind2 = binarySearch(unrevealed_indices, 0, unrevealed_cnt - 1, neigh_ind);
            // printf("C neigh_ind2 %d\n", neigh_ind2);
            if (neigh_ind2 < 0)
              exit(-3);
            q[neigh_ind2] *= (num_unrevealed_neigh - curr_constraint_val) / sum_q;
          }
        }
      }
    }

    // last constraint - total mines
    double sum_p = 0.;
    double sum_q = 0.;
    for (int i = 0; i < unrevealed_cnt; ++i)
    {
      sum_p += p[i];
      sum_q += q[i];
    }

    if (fabs(sum_p - c[constraint_cnt - 1]) > 0.001)
    {
      for (int i = 0; i < unrevealed_cnt; ++i)
      {
        p[i] *= c[constraint_cnt - 1] / sum_p;
      }
    }

    if (fabs(sum_q - unrevealed_cnt + c[constraint_cnt - 1]) > 0.001)
    {
      for (int i = 0; i < unrevealed_cnt; ++i)
      {
        q[i] *= (unrevealed_cnt - c[constraint_cnt - 1]) / sum_q;
      }
    }

    for (int i = 0; i < unrevealed_cnt; ++i)
    {
      double sum = p[i] + q[i];
      p[i] = p[i] / sum;
      q[i] = q[i] / sum;
    }

    // printf("p ");
    // for (int i = 0; i < unrevealed_cnt; ++i)
    // {
    //   printf("%g,", p[i]);
    // }
    // printf("\n");
    // printf("last_p ");
    // for (int i = 0; i < unrevealed_cnt; ++i)
    // {
    //   printf("%g,", last_p[i]);
    // }
    // printf("\n");

    // printf("q ");
    // for (int i = 0; i < unrevealed_cnt; ++i)
    // {
    //   printf("%g,", q[i]);
    // }
    // printf("\n");

    bool done = true;
    for (int i = 0; i < unrevealed_cnt; ++i)
    {
      if (fabs(last_p[i] - p[i]) > 0.0001)
      {
        done = false;
        break;
      }
    }
    if (done)
      break;

    memcpy(last_p, p, unrevealed_cnt * sizeof(*p));
  }

  int argmax = -1;
  int argmin = -1;
  double max = -1.;
  double min = 2.;
  for (int i = 0; i < unrevealed_cnt; ++i)
  {
    if (max < p[i])
    {
      argmax = i;
      max = p[i];
    }
    if (min > p[i])
    {
      argmin = i;
      min = p[i];
    }
  }

  if (max > 1.)
    max = 1.;
  if (min < 0.)
    min = 0.;

  int ind = argmax;
  char ch = 'f';
  if (min < (1 - max))
  {
    ch = ' ';
    ind = argmin;
  } else {
    user->num_flags++;
  }

  // TODO(oc) : multiple values the same as max/min random
  int ind2 = unrevealed_indices[ind];
  int row_ind = ind2 / user->config.rows;
  int col_ind = ind2 % user->config.cols;

  int diff_i = row_ind - user->pos.i;
  int diff_j = col_ind - user->pos.j;

  MoveByDiff(sock, diff_i, diff_j);

  send_message(sock, 1, &ch, -1);

  free(c);
  free(p);
  free(last_p);
  free(q);
  free(unrevealed_indices);
  free(constraint_indices);
}

void run_one_game(int sock, struct User *user, int game_i)
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

  for (int iter = 0, step_cnt = 0;; ++iter)
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

    if (step_cnt < user->num_steps)
    {
      struct Step *ptr = &user->steps[step_cnt++];
      int diff_i = ptr->pos.i - user->pos.i;
      int diff_j = ptr->pos.j - user->pos.j;

      MoveByDiff(sock, diff_i, diff_j);
      char c = 'f';
      if (ptr->reveal)
        c = ' ';
      else
        user->num_flags++;
      send_message(sock, 1, &c, -1);
      continue;
    }

    if (CheckForSolution(user, sock))
    {
      random_reveal = false;
      continue;
    }

    // if (game_i == 5)
    // {
    //   printf("revealing random %d\n", iter);
    //   getchar();
    // }

    max_entropy_solution(sock, user);
    // if (step_cnt == 4)
    // exit(-2);

    random_reveal = true;
    // RevealRandomLocation(user, sock);
  }

  free(msg);
  DeInit(user);
}

void run_user(int sock, struct User *user)
{
  for (int i = 0; i < 10000; ++i)
  {
    if (i % 100 == 1)
    {
      printf("\r %d%%", i / 100);
      fflush(stdout);
    }
    send_message(sock, 3, &user->config, -1);

    run_one_game(sock, user, i);
  }

  char c = 'q';
  send_message(sock, 1, &c, -1);
  // usleep(10000);
}

void parse_user_from_file(const char *user_file, struct User *user)
{
  FILE *f = fopen(user_file, "r");

  size_t len = 0;
  ssize_t read;

  if (f == NULL)
    exit(EXIT_FAILURE);

  int rows, cols;
  read = fscanf(f, "%dx%d,%d", &rows, &cols, &user->config.mines);
  if (read != 3 || rows <= 0 || rows > 127 || cols <= 0 || cols > 127)
  {
    printf("failed to read config\n");
    exit(-1);
  }

  user->config.rows = rows;
  user->config.cols = cols;

  long curr = ftell(f);

  int i, j, reveal;
  user->num_steps = 0;
  while ((read = fscanf(f, "%d,%d,%d", &i, &j, &reveal)) == 3)
  {
    ++user->num_steps;
  }

  user->steps = (struct Step *)malloc(sizeof(*user->steps) * user->num_steps);

  fseek(f, curr, SEEK_SET);

  int cnt = 0;
  while ((read = fscanf(f, "%d,%d,%d", &i, &j, &reveal)) == 3)
  {
    if (i < 0 || i > 127 || j < 0 || j > 127 || reveal < 0 || reveal > 1)
      exit(-1);
    struct Step *ptr = &user->steps[cnt++];
    ptr->pos.i = i;
    ptr->pos.j = j;
    ptr->reveal = reveal;
  }

  fclose(f);
}

int main(int argc, char *argv[])
{
  struct User user;
  user.steps = NULL;

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
  default:
    parse_user_from_file(argv[1], &user);
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