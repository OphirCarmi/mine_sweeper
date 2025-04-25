#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/queue.h>
#include <ncurses.h>

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
    if (is_flagged_board[i][j]) {
      printw("f");
    } else {
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
  for (int i = 0; i < ROWS; ++i) {
    for (int j = 0; j < ROWS; ++j) {
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

  return true;
}

int main()
{
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

  srand(time(NULL));

  // צ׳יט שחושף את כל הלוח
  // memset(is_revealed_board, -1, sizeof(is_revealed_board));

  PlaceMines();

  for (;;)
  {
    DrawBoard();

    // נבקש קלט מהמשתמש/ת
    addstr("\nquit anytime with \"q\"\n\n");
    printw("use arrows to move\n");
    printw("use space bar to reveal\n");
    printw("use `f` to flag an existing mine\n");
    refresh();

    int c = getch();

    bool should_break = false;
    int curr;
    switch (c)
    {
    case 'q':
      should_break = true;
      break;
    case KEY_UP:
      curr = pos.i - 1;
      if (curr >= 0)
        pos.i = curr;
      break;
    case KEY_DOWN:
      curr = pos.i + 1;
      if (curr < ROWS)
        pos.i = curr;
      break;
    case KEY_LEFT:
      curr = pos.j - 1;
      if (curr >= 0)
        pos.j = curr;
      break;
    case KEY_RIGHT:
      curr = pos.j + 1;
      if (curr < COLS)
        pos.j = curr;
      break;
    case ' ':
      // אם המיקום המבוקש הוא מוקש נסיים את המשחק ואם לא אז נחשוף תא אחד או יותר
      should_break = !RevealLocation();
      break;
    case 'f':
      is_flagged_board[pos.i][pos.j] = !is_flagged_board[pos.i][pos.j];
      break;
    }

    if (should_break)
      break;

    // נבדוק האם המשתמש/ת ניצח, כלומר סיימ/ה לחשוף את כל התאים שאינם מוקשים
    if (CheckWin())
      break;
  }

  endwin();

  return 0;
}