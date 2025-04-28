#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/queue.h>
#include <ncurses.h>
#include <pthread.h>

#define ROWS 10
#define COLS 10

#define NUM_MINES (ROWS * COLS / 7)

static int8_t hidden_board[ROWS][COLS] = {0};

static bool is_revealed_board[ROWS][COLS] = {0};
static bool is_flagged_board[ROWS][COLS] = {0};

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

struct Position
{
  int i;
  int j;
};

static struct Position pos;

void PlaceMines()
{
  int indices[ROWS * COLS];
  for (int i = 0; i < ROWS * COLS; ++i)
  {
    indices[i] = i;
  }
  // מלא במספרים עוקבים indices עכשיו המערך
  // [0, 1, 2, 3, ..., ROWS * COLS - 1]
  // כל מספר מתאים לתא בלוח המוקשים שלנו
  // אם נניח שמספר השורות הוא 5 ומספר העמודות הוא 10
  // -------------------------------------------
  // | 0  | 1  |  2  | ...            | 8 | 9  |
  // -------------------------------------------
  // | 10 | 11 | 12  | ...            | 18| 19 |
  // -------------------------------------------
  // | ...                                     |
  // -------------------------------------------
  // | ...                                     |
  // -------------------------------------------
  // | 40 | 41 | 42  | ...            | 48| 49 |
  // -------------------------------------------
  //
  // עכשיו ניצור מקומות אקראיים לשים בהם את המוקשים
  // נעשה זאת ע״י ערבוב המקומות הראשונים של המערך
  // זה נושא להרצאה אחרת
  for (int i = 0; i < NUM_MINES; ++i)
  {
    int ind = i + rand() % (ROWS * COLS - i);
    int temp = indices[ind];
    indices[ind] = indices[i];
    indices[i] = temp;
  }

  // עכשיו המקומות הראשונים במערך הם אינדקסים אקראיים, למשל
  // [12, 5, 23, 46, ...]
  // בתאים אלה נשים את המוקשים

  for (int i = 0; i < NUM_MINES; ++i)
  {
    // בכל איטרציה נשים מוקש אחד ונעדכן את הערך של התאים השכנים שלו
    int ind = indices[i];

    // נהפוך את האינדקס האקראי במערך החד-מימדי לשני אינדקסים בלוח שהוא מערך דו-מימדי
    int row_ind = ind / COLS;
    int col_ind = ind % COLS;

    // מוקש יסומן בערך מיוחד (1-). יכולנו גם לבחור ערך אחר שלא מופיע בתא רגיל
    // שאלת בונוס: איזה ערכים אפשר
    hidden_board[row_ind][col_ind] = -1;

    // עכשיו הלוח נראה בערך ככה
    // --------------------------------------------------------
    // | 0 | -1| 0 | 0 | ...                          | 0 | 0 |
    // --------------------------------------------------------
    // | 0 | 0 | 0  |-1 | ...                    | -1 | 0 | 0 |
    // --------------------------------------------------------
    // ...

    // עכשיו נעדכן את ערכי השכנים
    // לכל תא יש בין שלושה לשמונה שכנים
    for (int k = 0; k < num_neighbours; ++k)
    {
      // בכל איטרציה נעדכן שכן אחד אם הוא קיים (ולא נופל מחוץ ללוח)
      int neigh_row_ind = row_ind + neighbours[k][0];
      // נבדוק שהוא לא מעל או מתחת ללוח
      if (neigh_row_ind < 0 || neigh_row_ind >= ROWS)
        continue;

      int neigh_col_ind = col_ind + neighbours[k][1];
      // נבדוק שהוא לא משמאל או מימין ללוח
      if (neigh_col_ind < 0 || neigh_col_ind >= COLS)
        continue;

      // נבדוק שהוא לא מוקש
      if (hidden_board[neigh_row_ind][neigh_col_ind] != -1)
      {
        // נוסיף אחד למנין המוקשים השכנים שלו (זוכרים שאתחלנו אותו לאפס בהתחלה? חשוב חשוב)
        hidden_board[neigh_row_ind][neigh_col_ind]++;
      }
    }
  }
}

void PrintCellValue(int i, int j)
{
  printw(" ");
  if (pos.i == i && pos.j == j)
  {
    attrset(COLOR_PAIR(1));
  }
  else
  {
    attrset(COLOR_PAIR(2));
  }

  // נבדוק שהתא הזה כבר חשוף למשתמש
  if (is_revealed_board[i][j])
  {
    if (hidden_board[i][j] == -1)
    {
      // אם הוא מוקש נסמנו בהתאם
      printw("*");
    }
    else
    {
      // אם הוא לא מוקש נדפיס למשתמש/ת את ערכו
      printw("%d", hidden_board[i][j]);
    }
  }
  else
  {
    if (is_flagged_board[i][j])
    {
      printw("f");
    }
    else
    {
      // אם הוא לא חשוף למשתמש/ת, נשאיר אותו ריק
      printw(" ");
    }
  }
  attroff(COLOR_PAIR(1));
  printw(" ");
}

void PrintHorizontalLine()
{
  for (int j = 0; j < COLS; ++j)
  {
    printw("----");
  }
  printw("-\n");
}

void DrawBoard()
{
  clear(); // clear screen

  int num_flags = 0;
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < ROWS; ++j)
    {
      num_flags += is_flagged_board[i][j];
    }
  }

  printw("Mine Sweeper, num mines: %d, num_flags: %d\n", NUM_MINES, num_flags);

  for (int i = 0; i < ROWS; ++i)
  {
    PrintHorizontalLine();

    for (int j = 0; j < COLS; ++j)
    {
      printw("|");
      PrintCellValue(i, j);
    }
    printw("|\n");
  }
  PrintHorizontalLine();
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

    printw("%d %d\n", curr_row_ind, curr_col_ind);

    LIST_REMOVE(head.lh_first, entries);

    for (int k = 0; k < num_neighbours; ++k)
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
      refresh();
      usleep(100000);

      if (hidden_board[neigh_row_ind][neigh_col_ind] != 0)
        continue;

      np = malloc(sizeof(struct entry)); /* Insert at the head. */
      np->row_ind = neigh_row_ind;
      np->col_ind = neigh_col_ind;
      LIST_INSERT_HEAD(&head, np, entries);
    }
  }
}

bool RevealLocation()
{
  switch (hidden_board[pos.i][pos.j])
  {
  case 0:
    RevealZeroes(pos.i, pos.j);
    return true;
  case -1:
    is_revealed_board[pos.i][pos.j] = true;
    DrawBoard();
    printw("\n\nBOOOOOOOOOM!!!! GAME OVER!\n");
    refresh();
    sleep(3);
    return false;
  default:
    is_revealed_board[pos.i][pos.j] = true;
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

  printw("\nYOU WON!!!\n");
  refresh();

  sleep(3);

  return true;
}

void Init()
{
  memset(is_flagged_board, 0, sizeof(is_flagged_board));
  memset(is_revealed_board, 0, sizeof(is_revealed_board));
  memset(hidden_board, 0, sizeof(hidden_board));
  memset(&pos, 0, sizeof(pos));

  PlaceMines();
}

struct fds
{
  int read_fd;
  int write_fd;
};

struct all_fds
{
  struct fds to_game_fd;
  struct fds to_user_fd;
};

void update_revealed_board(char *revealed_board)
{
  char *ptr = revealed_board;
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      if (is_revealed_board[i][j])
      {
        if (hidden_board[i][j] == -1)
          *ptr = '*';
        else
          *ptr = '0' + hidden_board[i][j];
      }
      else if (is_flagged_board[i][j])
      {
        *ptr = 'f';
      }
      else
      {
        *ptr = ' ';
      }
      ++ptr;
    }
  }
}

void write_revealed_board(char *revealed_board, struct all_fds *all_fds)
{
  FILE *f = fopen("/tmp/game.txt", "a");
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      fprintf(f, "| %c |", revealed_board[i * COLS + j]);
    }
    fprintf(f, "\n");
    for (int j = 0; j < COLS; ++j)
    {
      fprintf(f, "----");
    }
    fprintf(f, "-\n");
  }
  fprintf(f, "\n\n");
  fclose(f);

  write(all_fds->to_user_fd.write_fd, revealed_board, ROWS * COLS);
}

void *run_game(void *arguments)
{
  struct all_fds all_fds = *(struct all_fds *)arguments;

  int ch;

  /* Curses Initialisations */
  initscr();
  use_default_colors();
  start_color();
  init_pair(1, -1, COLOR_GREEN);
  init_pair(2, -1, -1);
  raw();
  keypad(stdscr, TRUE);
  noecho();

  pos.i = 0;
  pos.j = 0;

  // srand(time(NULL));

  // צ׳יט שחושף את כל הלוח
  // memset(is_revealed_board, -1, sizeof(is_revealed_board));

  Init();

  char revealed_board[ROWS * COLS];
  char last_revealed_board[ROWS * COLS];

  update_revealed_board(revealed_board);
  write_revealed_board(revealed_board, &all_fds);

  memcpy(last_revealed_board, revealed_board, ROWS * COLS);

  for (;;)
  {
    DrawBoard();

    // נבקש קלט מהמשתמש/ת
    addstr("\nquit anytime with \"q\"\n\n");
    printw("use arrows to move\n");
    printw("use space bar to reveal\n");
    printw("use `f` to flag an existing mine\n");
    refresh();

    char c;
    ssize_t num_read = read(all_fds.to_game_fd.read_fd, &c, sizeof(c));
    if (num_read <= 0)
    {
      usleep(10000);
      continue;
    }

    bool should_break = false;
    bool new_game = false;
    int curr;
    switch (c)
    {
    case 'q':
      should_break = true;
      break;
    case 'w': // up
      curr = pos.i - 1;
      if (curr >= 0)
        pos.i = curr;
      break;
    case 'x': // down
      curr = pos.i + 1;
      if (curr < ROWS)
        pos.i = curr;
      break;
    case 'a': // left
      curr = pos.j - 1;
      if (curr >= 0)
        pos.j = curr;
      break;
    case 'd': // right
      curr = pos.j + 1;
      if (curr < COLS)
        pos.j = curr;
      break;
    case ' ':
      // אם המיקום המבוקש הוא מוקש נסיים את המשחק ואם לא אז נחשוף תא אחד או יותר
      new_game = !RevealLocation();
      break;
    case 'f':
      is_flagged_board[pos.i][pos.j] = !is_flagged_board[pos.i][pos.j];
      break;
    }

    if (should_break)
      break;

    // נבדוק האם המשתמש/ת ניצח, כלומר סיימ/ה לחשוף את כל התאים שאינם מוקשים
    if (new_game || CheckWin())
      break; // Init();

    update_revealed_board(revealed_board);
    if (memcmp(revealed_board, last_revealed_board, ROWS * COLS))
    {
      write_revealed_board(revealed_board, &all_fds);
      memcpy(last_revealed_board, revealed_board, ROWS * COLS);
    }

    usleep(50000);
  }

  close(all_fds.to_game_fd.read_fd);
  close(all_fds.to_user_fd.write_fd);

  endwin();

  return NULL;
}

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

void open_channel(int *read_fd, int *write_fd)
{
  int vals[2];
  int errc = pipe(vals);
  if (errc)
  {
    fputs("Bad pipe", stderr);
    *read_fd = -1;
    *write_fd = -1;
  }
  else
  {
    *read_fd = vals[0];
    *write_fd = vals[1];
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

      FILE *f = fopen("/tmp/user.txt", "a");
      fprintf(f, "i %d j %d val %d sum_unrevealed %d sum_flags %d\n", i, j, val, sum_unrevealed, sum_flags);
      fclose(f);

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

            f = fopen("/tmp/user.txt", "a");
            fprintf(f, "3 di %d dj %d\n", diff_i, diff_j);
            fclose(f);

            MoveByDiff(write_fd, diff_i, diff_j);

            char c = ' ';
            write(write_fd, &c, sizeof(c));
            usleep(100000);
          }
        }
        return true;
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
          FILE *f = fopen("/tmp/user.txt", "a");
          fprintf(f, "i %d j %d\n", neigh_row_ind, neigh_col_ind);
          fclose(f);

          int diff_i = neigh_row_ind - pos.i;
          int diff_j = neigh_col_ind - pos.j;

          f = fopen("/tmp/user.txt", "a");
          fprintf(f, "di %d dj %d\n", diff_i, diff_j);
          fclose(f);

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

    FILE *f = fopen("/tmp/user.txt", "a");
    fprintf(f, "1 i %d j %d\n", row_ind, col_ind);
    fclose(f);

    int diff_i = row_ind - pos.i;
    int diff_j = col_ind - pos.j;

    f = fopen("/tmp/user.txt", "a");
    fprintf(f, "2 di %d dj %d\n", diff_i, diff_j);
    fclose(f);

    MoveByDiff(write_fd, diff_i, diff_j);

    char c = ' ';
    write(write_fd, &c, sizeof(c));
    break;
  }
}

void *run_user(void *arguments)
{
  struct all_fds all_fds = *(struct all_fds *)arguments;

  char revealed_board[ROWS * COLS];

  for (int i = 0;; ++i)
  {
    // TODO (oc): get pos too
    int num_read = read(all_fds.to_user_fd.read_fd, revealed_board, ROWS * COLS);
    if (num_read <= 0)
    {
      usleep(1000);
      continue;
    }

    FILE *f = fopen("/tmp/user.txt", "a");
    fprintf(f, "num_read %d\n", num_read);

    for (int m = 0; m < ROWS; ++m)
    {
      for (int j = 0; j < COLS; ++j)
      {
        fprintf(f, "----");
      }
      fprintf(f, "-\n");
      for (int n = 0; n < COLS; ++n)
      {
        fprintf(f, "| %c ", revealed_board[m * COLS + n]);
      }
      fprintf(f, "|\n");
    }
    for (int j = 0; j < COLS; ++j)
    {
      fprintf(f, "----");
    }
    fprintf(f, "-\n");

    fprintf(f, "before CheckForSolution\n");
    fclose(f);
    if (CheckForSolution(revealed_board, all_fds.to_game_fd.write_fd))
      continue;

    f = fopen("/tmp/user.txt", "a");
    fprintf(f, "before RevealRandomLocation\n");
    fclose(f);
    RevealRandomLocation(revealed_board, all_fds.to_game_fd.write_fd);
  }

  char c = 'q';
  write(all_fds.to_game_fd.write_fd, &c, sizeof(c));

  close(all_fds.to_game_fd.write_fd);
  close(all_fds.to_user_fd.read_fd);

  return NULL;
}

int main()
{
  struct all_fds all_fds;
  open_channel(&all_fds.to_game_fd.read_fd, &all_fds.to_game_fd.write_fd);

  open_channel(&all_fds.to_user_fd.read_fd, &all_fds.to_user_fd.write_fd);

  pthread_t game;
  pthread_t user;

  pthread_create(&game, NULL, run_game, &all_fds);
  sleep(2);
  pthread_create(&user, NULL, run_user, &all_fds);

  pthread_join(game, NULL);
  pthread_join(user, NULL);

  return 0;
}