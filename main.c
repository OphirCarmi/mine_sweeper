#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/queue.h>

#define ROWS 5
#define COLS 10

#define NUM_MINES 7

static int8_t hidden_board[ROWS][COLS] = {0};

static bool is_revealed_board[ROWS][COLS] = {0};

static int8_t neighbours[][2] = {
    {-1, -1}, // top left
    {-1, 0},  // top
    {-1, 1},  // top right
    {0, -1},  // left
    {0, 1},   // right
    {1, -1},  // bottom left
    {1, 0},   // bottom
    {1, 1},   // bottom right
};

void PlaceMines()
{
  int indices[ROWS * COLS];
  for (int i = 0; i < ROWS * COLS; ++i)
  {
    indices[i] = i;
  }

  for (int i = 0; i < NUM_MINES; ++i)
  {
    int ind = i + rand() % (ROWS * COLS - i);
    int temp = indices[ind];
    indices[ind] = indices[i];
    indices[i] = temp;
  }

  for (int i = 0; i < NUM_MINES; ++i)
  {
    int ind = indices[i];

    int row_ind = ind / COLS;
    int col_ind = ind % COLS;
    hidden_board[row_ind][col_ind] = -1;

    for (int k = 0; k < 8; ++k)
    {
      int neigh_row_ind = row_ind + neighbours[k][0];
      if (neigh_row_ind < 0 || neigh_row_ind >= ROWS)
        continue;

      int neigh_col_ind = col_ind + neighbours[k][1];
      if (neigh_col_ind < 0 || neigh_col_ind >= COLS)
        continue;

      if (hidden_board[neigh_row_ind][neigh_col_ind] != -1)
      {
        hidden_board[neigh_row_ind][neigh_col_ind]++;
      }
    }
  }
}

void PrintCellValue(int i, int j)
{
  if (is_revealed_board[i][j])
  {
    if (hidden_board[i][j] == -1)
    {
      printf(" * ");
    }
    else
    {
      printf(" %d ", hidden_board[i][j]);
    }
  }
  else
  {
    printf("   ");
  }
}

void PrintHorizontalLine()
{
  printf(" ");
  for (int j = 0; j < COLS; ++j)
  {
    printf("----");
  }
  printf("-\n");
}

void DrawBoard()
{
  printf("\e[1;1H\e[2J"); // clear screen

  for (int i = 0; i < ROWS; ++i)
  {
    PrintHorizontalLine();

    printf("%c", 'a' + i);
    for (int j = 0; j < COLS; ++j)
    {
      printf("|");
      PrintCellValue(i, j);
    }
    printf("|\n");
  }
  PrintHorizontalLine();
  printf(" ");
  for (int j = 0; j < COLS; ++j)
  {
    printf("  %d ", j);
  }
  printf("\n");
}

bool CheckValidInput(const char *word, int *row_ind, int *col_ind)
{
  if (strlen(word) != 3)
    return false;

  *row_ind = word[0] - 'a';
  if (*row_ind < 0 || *row_ind >= ROWS)
    return false;

  *col_ind = word[1] - '0';
  if (*col_ind < 0 || *col_ind >= COLS)
    return false;

  return true;
}

void RevealZeroes(int row_ind, int col_ind)
{
  LIST_HEAD(listhead, entry)
  head;
  struct listhead *headp; /* List head. */
  struct entry
  {
    int row_ind;
    int col_ind;
    LIST_ENTRY(entry)
    entries; /* List. */
  } *np;

  LIST_INIT(&head); /* Initialize the list. */

  np = malloc(sizeof(struct entry)); /* Insert at the head. */
  np->row_ind = row_ind;
  np->col_ind = col_ind;
  LIST_INSERT_HEAD(&head, np, entries);

  is_revealed_board[row_ind][col_ind] = true;

  while (head.lh_first != NULL)
  {
    int curr_row_ind = head.lh_first->row_ind;
    int curr_col_ind = head.lh_first->col_ind;

    printf("%d %d\n", curr_row_ind, curr_col_ind);

    LIST_REMOVE(head.lh_first, entries);

    for (int k = 0; k < 8; ++k)
    {
      int neigh_row_ind = curr_row_ind + neighbours[k][0];
      if (neigh_row_ind < 0 || neigh_row_ind >= ROWS)
        continue;

      int neigh_col_ind = curr_col_ind + neighbours[k][1];
      if (neigh_col_ind < 0 || neigh_col_ind >= COLS)
        continue;

      if (is_revealed_board[neigh_row_ind][neigh_col_ind])
        continue;

      is_revealed_board[neigh_row_ind][neigh_col_ind] = true;

      DrawBoard();
      usleep(200000);

      if (hidden_board[neigh_row_ind][neigh_col_ind] != 0)
        continue;

      np = malloc(sizeof(struct entry)); /* Insert at the head. */
      np->row_ind = neigh_row_ind;
      np->col_ind = neigh_col_ind;
      LIST_INSERT_HEAD(&head, np, entries);
    }
  }
}

bool RevealLocation(int row_ind, int col_ind)
{
  switch (hidden_board[row_ind][col_ind])
  {
  case 0:
    RevealZeroes(row_ind, col_ind);
    return true;
  case -1:
    is_revealed_board[row_ind][col_ind] = true;
    DrawBoard();
    printf("\n\nBOOOOOOOOOM!!!! GAME OVER!\n");
    fflush(stdout);
    sleep(3);
    return false;
  default:
    is_revealed_board[row_ind][col_ind] = true;
    return true;
  }
}

bool CheckWin()
{
  int sum_unrevealed = ROWS * COLS;
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      sum_unrevealed -= is_revealed_board[i][j];
    }
  }

  if (sum_unrevealed > NUM_MINES)
    return false;

  memset(is_revealed_board, -1, sizeof(is_revealed_board));

  DrawBoard();

  printf("\nYOU WON!!!\n");
  fflush(stdout);

  return true;
}

int main()
{
  srand(time(NULL));

  char word[16] = "";

  // memset(is_revealed_board, -1, sizeof(is_revealed_board));

  PlaceMines();

  for (;;)
  {
    DrawBoard();

    printf("\nquit anytime with \"q\"\n\n");
    printf("enter location (for example: \"c6\") : ");
    fgets(word, sizeof(word), stdin);

    if (!strcmp(word, "q\n"))
      break;

    int row_ind = -1;
    int col_ind = -1;
    if (!CheckValidInput(word, &row_ind, &col_ind))
    {
      printf(" invalid input!");
      fflush(stdout);
      sleep(1);
      continue;
    }

    if (!RevealLocation(row_ind, col_ind))
      break;

    if (CheckWin())
      break;
  }

  return 0;
}