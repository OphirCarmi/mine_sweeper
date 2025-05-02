#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/queue.h>
#include <ncurses.h>
#include <arpa/inet.h>

#include "common/common.h"

#define BUFFER_SIZE 1024

#define NUM_MINES (NUM_CELLS / 7)

static int8_t hidden_board[ROWS][COLS] = {0};

static bool is_revealed_board[ROWS][COLS] = {0};
static bool is_flagged_board[ROWS][COLS] = {0};

static struct Position pos;

void PlaceMines()
{
  int indices[NUM_CELLS];
  for (int i = 0; i < NUM_CELLS; ++i)
  {
    indices[i] = i;
  }
  // מלא במספרים עוקבים indices עכשיו המערך
  // [0, 1, 2, 3, ..., NUM_CELLS - 1]
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
    int ind = i + rand() % (NUM_CELLS - i);
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
    return false;
  default:
    is_revealed_board[pos.i][pos.j] = true;
    return true;
  }
}

bool CheckWin()
{
  int sum_unrevealed = NUM_CELLS;
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      sum_unrevealed -= is_revealed_board[i][j];
    }
  }

  if (sum_unrevealed > NUM_MINES)
    return false;

  int sum_flags = 0;
  for (int i = 0; i < ROWS; ++i)
  {
    for (int j = 0; j < COLS; ++j)
    {
      sum_flags += is_flagged_board[i][j];
    }
  }

  if (sum_flags != NUM_MINES)
    return false;

  // memset(is_revealed_board, -1, sizeof(is_revealed_board));

  DrawBoard();

  printw("\nYOU WON!!!\n");
  refresh();

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

void write_revealed_board(char *revealed_board, int sock)
{
  // FILE *f = fopen("/tmp/game.txt", "a");
  // for (int i = 0; i < ROWS; ++i)
  // {
  //   for (int j = 0; j < COLS; ++j)
  //   {
  //     fprintf(f, "| %c |", revealed_board[i * COLS + j]);
  //   }
  //   fprintf(f, "\n");
  //   for (int j = 0; j < COLS; ++j)
  //   {
  //     fprintf(f, "----");
  //   }
  //   fprintf(f, "-\n");
  // }
  // fprintf(f, "\n\n");
  // fclose(f);

  char mem[NUM_CELLS + sizeof(pos)];
  char *ptr = mem;
  memcpy(ptr, revealed_board, NUM_CELLS);
  ptr += NUM_CELLS;
  memcpy(ptr, &pos, sizeof(pos));

  send_message(sock, 0, mem);
}

int run_one_game(int sock) {
  char revealed_board[NUM_CELLS];

  Init();
  update_revealed_board(revealed_board);
  write_revealed_board(revealed_board, sock);

  bool should_exit = false;

  bool win = false;

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
    int msg_type;
    if (!get_message(sock, &msg_type, &c))
    {
      usleep(1000);
      continue;
    }

    bool lose = false;
    bool board_changed = false;
    int curr;
    switch (c)
    {
    case 'q':
      should_exit = true;
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
      lose = !RevealLocation();
      board_changed = true;
      break;
    case 'f':
      is_flagged_board[pos.i][pos.j] = !is_flagged_board[pos.i][pos.j];
      board_changed = true;
      break;
    }

    if (should_exit)
      break;

    // נבדוק האם המשתמש/ת ניצח, כלומר סיימ/ה לחשוף את כל התאים שאינם מוקשים
    win = CheckWin();
    if (lose || win)
    {
      // FILE *f = fopen("/tmp/game.txt", "a");
      // fprintf(f, "win %d\n", win);
      // fclose(f);
      send_message(sock, 2, &win);

      sleep(3);
      break;
    }

    update_revealed_board(revealed_board);
    if (board_changed)
    {
      write_revealed_board(revealed_board, sock);
    }
  }

  if (should_exit) return -1;

  return win;
}

void run_game(int sock)
{
  // struct all_fds all_fds = *(struct all_fds *)arguments;

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

  srand(time(NULL));

  // צ׳יט שחושף את כל הלוח
  // memset(is_revealed_board, -1, sizeof(is_revealed_board));

  for (;;) {
    int ret = run_one_game(sock);
    if (ret < 0) break;
  }

  endwin();
}

int main()
{
  int server_fd, new_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Attaching socket to the port
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
  {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Binding the socket to the address
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Listening for incoming connections
  if (listen(server_fd, 3) < 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  printf("Waiting for connections...\n");

  if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
  {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  run_game(new_socket);

  // Close the socket
  close(new_socket);
  close(server_fd);

  return 0;
}