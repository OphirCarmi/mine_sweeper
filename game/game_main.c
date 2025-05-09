#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/queue.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <locale.h>

#include "common/common.h"

#define BUFFER_SIZE 1024

struct Game
{
  int8_t **hidden_board;

  bool **is_revealed_board;
  bool **is_flagged_board;

  char *revealed_board;
  char *last_revealed_board;

  struct Position pos;

  struct GameConfig config;
};

void PlaceMines(struct Game *game)
{
  int num_cells = game->config.rows * game->config.cols;
  int *indices = (int *)malloc(sizeof(*indices) * (num_cells - 1));
  int *ptr = indices;
  for (int i = 0; i < num_cells; ++i)
  {
    if (i / game->config.cols == game->pos.i && i % game->config.cols == game->pos.j)
      continue;
    *ptr = i;
    ++ptr;
  }
  for (int i = 0; i < game->config.mines; ++i)
  {
    int ind = i + rand() % (num_cells - 1 - i);
    int temp = indices[ind];
    indices[ind] = indices[i];
    indices[i] = temp;
  }

  for (int i = 0; i < game->config.mines; ++i)
  {
    int ind = indices[i];

    int row_ind = ind / game->config.cols;
    int col_ind = ind % game->config.cols;

    game->hidden_board[row_ind][col_ind] = -1;

    for (int k = 0; k < num_neighbours; ++k)
    {
      int neigh_row_ind = row_ind + neighbours[k][0];
      if (neigh_row_ind < 0 || neigh_row_ind >= game->config.rows)
        continue;

      int neigh_col_ind = col_ind + neighbours[k][1];
      // נבדוק שהוא לא משמאל או מימין ללוח
      if (neigh_col_ind < 0 || neigh_col_ind >= game->config.cols)
        continue;

      // נבדוק שהוא לא מוקש
      if (game->hidden_board[neigh_row_ind][neigh_col_ind] != -1)
      {
        // נוסיף אחד למנין המוקשים השכנים שלו (זוכרים שאתחלנו אותו לאפס בהתחלה? חשוב חשוב)
        game->hidden_board[neigh_row_ind][neigh_col_ind]++;
      }
    }
  }

  free(indices);
}

void SetColor(struct Game *game, int i, int j, bool red) {
  if (game->pos.i == i && game->pos.j == j)
  {
    if (red)
      attrset(COLOR_PAIR(3));
    else
      attrset(COLOR_PAIR(1));
  }
  else
  {
    attrset(COLOR_PAIR(2));
  }
}

void PrintCellValue(struct Game *game, int i, int j, bool red, bool board_changed)
{
  if (board_changed) SetColor(game, i, j, red);
  printw(" ");
  SetColor(game, i, j, red);

  // נבדוק שהתא הזה כבר חשוף למשתמש
  if (game->is_revealed_board[i][j])
  {
    if (game->hidden_board[i][j] == -1)
    {
      // אם הוא מוקש נסמנו בהתאם
      printw("*");
    }
    else
    {
      // אם הוא לא מוקש נדפיס למשתמש/ת את ערכו
      printw("%d", game->hidden_board[i][j]);
    }
  }
  else
  {
    if (game->is_flagged_board[i][j])
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
  if (board_changed) SetColor(game, i, j, red);
  printw(" ");
  attroff(COLOR_PAIR(1));
}

void PrintHorizontalLine(struct Game *game)
{
  for (int j = 0; j < game->config.cols; ++j)
  {
    printw("----");
  }
  printw("-\n");
}

void DrawBoard(struct Game *game, bool red, bool board_changed)
{
  clear(); // clear screen

  int num_flags = 0;
  for (int i = 0; i < game->config.rows; ++i)
  {
    for (int j = 0; j < game->config.cols; ++j)
    {
      num_flags += game->is_flagged_board[i][j];
    }
  }

  printw("Mine Sweeper, rows: %d, cols: %d, mines: %d, flags: %d\n", game->config.rows, game->config.cols, game->config.mines, num_flags);

  for (int i = 0; i < game->config.rows; ++i)
  {
    PrintHorizontalLine(game);

    for (int j = 0; j < game->config.cols; ++j)
    {
      printw("|");
      PrintCellValue(game, i, j, red, board_changed);
    }
    printw("|\n");
  }
  PrintHorizontalLine(game);
}

void RevealZeroes(struct Game *game)
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

  np = (struct entry *)malloc(sizeof(struct entry)); /* Insert at the head. */
  np->row_ind = game->pos.i;
  np->col_ind = game->pos.j;
  LIST_INSERT_HEAD(&head, np, entries);

  game->is_revealed_board[game->pos.i][game->pos.j] = true;

  while (head.lh_first != NULL)
  {
    int curr_row_ind = head.lh_first->row_ind;
    int curr_col_ind = head.lh_first->col_ind;

    printw("%d %d\n", curr_row_ind, curr_col_ind);

    np = head.lh_first;
    LIST_REMOVE(head.lh_first, entries);
    free(np);

    for (int k = 0; k < num_neighbours; ++k)
    {
      int neigh_row_ind = curr_row_ind + neighbours[k][0];
      if (neigh_row_ind < 0 || neigh_row_ind >= game->config.rows)
        continue;

      int neigh_col_ind = curr_col_ind + neighbours[k][1];
      if (neigh_col_ind < 0 || neigh_col_ind >= game->config.cols)
        continue;

      if (game->is_revealed_board[neigh_row_ind][neigh_col_ind])
        continue;

      game->is_revealed_board[neigh_row_ind][neigh_col_ind] = true;

#ifdef SHOW
      DrawBoard(game, false, false);
      refresh();
      usleep(50000);
#endif // SHOW

      if (game->hidden_board[neigh_row_ind][neigh_col_ind] != 0)
        continue;

      np = (struct entry*)malloc(sizeof(struct entry)); /* Insert at the head. */
      np->row_ind = neigh_row_ind;
      np->col_ind = neigh_col_ind;
      LIST_INSERT_HEAD(&head, np, entries);
    }
  }
}

bool RevealLocation(struct Game *game)
{
  switch (game->hidden_board[game->pos.i][game->pos.j])
  {
  case 0:
    RevealZeroes(game);
    return true;
  case -1:
    game->is_revealed_board[game->pos.i][game->pos.j] = true;
#ifdef SHOW
    DrawBoard(game, true, true);
    printw("\n\nBOOOOOOOOOM!!!! GAME OVER!\n");
    refresh();
    sleep(3);
#endif // SHOW
    return false;
  default:
    game->is_revealed_board[game->pos.i][game->pos.j] = true;
    return true;
  }
}

bool CheckWin(struct Game *game)
{
  int sum_unrevealed = game->config.rows * game->config.cols;
  for (int i = 0; i < game->config.rows; ++i)
  {
    for (int j = 0; j < game->config.cols; ++j)
    {
      sum_unrevealed -= game->is_revealed_board[i][j];
    }
  }

  if (sum_unrevealed > game->config.mines)
    return false;

  int sum_flags = 0;
  for (int i = 0; i < game->config.rows; ++i)
  {
    for (int j = 0; j < game->config.cols; ++j)
    {
      sum_flags += game->is_flagged_board[i][j];
    }
  }

  if (sum_flags != game->config.mines)
    return false;

  for (int i = 0; i < game->config.rows; ++i)
  {
    for (int j = 0; j < game->config.cols; ++j)
    {
      game->is_revealed_board[i][j] = true;
    }
  }
#ifdef SHOW
  DrawBoard(game, false, false);

  printw("\nYOU WON!!!\n");
  refresh();

  sleep(3);
#endif // SHOW
  return true;
}

void Init(struct Game *game)
{
  game->hidden_board = (int8_t **)malloc(game->config.rows * sizeof(*game->hidden_board));
  for (int i = 0; i < game->config.rows; ++i)
    game->hidden_board[i] = (int8_t *)calloc(game->config.cols, 1);

  game->is_flagged_board = (bool **)malloc(game->config.rows * sizeof(*game->is_flagged_board));
  for (int i = 0; i < game->config.rows; ++i)
    game->is_flagged_board[i] = (bool *)calloc(game->config.cols, 1);

  game->is_revealed_board = (bool **)malloc(game->config.rows * sizeof(*game->is_revealed_board));
  for (int i = 0; i < game->config.rows; ++i)
    game->is_revealed_board[i] = (bool *)calloc(game->config.cols, 1);

  int num_cells = game->config.rows * game->config.cols;
  game->revealed_board = (char *)malloc(num_cells);
  game->last_revealed_board = (char *)malloc(num_cells);
  for (int i = 0; i < num_cells; ++i) {
    game->revealed_board[i] = ' ';
    game->last_revealed_board[i] = ' ';
  }

  memset(&game->pos, 0, sizeof(game->pos));
}

void DeInit(struct Game *game)
{
  for (int i = 0; i < game->config.rows; ++i)
    free(game->hidden_board[i]);
  free(game->hidden_board);

  for (int i = 0; i < game->config.rows; ++i)
    free(game->is_flagged_board[i]);
  free(game->is_flagged_board);

  for (int i = 0; i < game->config.rows; ++i)
    free(game->is_revealed_board[i]);
  free(game->is_revealed_board);

  free(game->revealed_board);
  free(game->last_revealed_board);
}

// TODO (oc) : keep a list of changed cells instead of updating all cells
void update_revealed_board(struct Game *game)
{
  char *ptr = game->revealed_board;
  for (int i = 0; i < game->config.rows; ++i)
  {
    for (int j = 0; j < game->config.cols; ++j)
    {
      if (game->is_revealed_board[i][j])
      {
        if (game->hidden_board[i][j] == -1)
          *ptr = '*';
        else
          *ptr = '0' + game->hidden_board[i][j];
      }
      else if (game->is_flagged_board[i][j])
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

void write_revealed_board(struct Game *game, int sock)
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

  int num_cells = game->config.rows * game->config.cols;
  int num_changed = 0;
  for (int i = 0; i < num_cells; ++i)
    if (game->revealed_board[i] != game->last_revealed_board[i])
      num_changed++;

  int tot_size = num_changed * sizeof(struct Cell) + sizeof(game->pos);
  char *mem = (char *)malloc(tot_size);
  char *ptr = mem;
  struct Cell cell;
  for (int i = 0; i < num_cells; ++i) {
    char curr = game->revealed_board[i];
    if (curr != game->last_revealed_board[i]) {
      cell.pos.i = i / game->config.cols;
      cell.pos.j = i % game->config.cols;
      cell.val = curr;
      memcpy(ptr, &cell, sizeof(cell));
      ptr += sizeof(cell);
    }
  }
  memcpy(ptr, &game->pos, sizeof(game->pos));

  send_message(sock, 0, mem, tot_size);

  free(mem);

  memcpy(game->last_revealed_board, game->revealed_board, game->config.rows * game->config.cols);
}

int run_one_game(int sock, struct Game *game)
{
  Init(game);
  update_revealed_board(game);
  write_revealed_board(game, sock);

  bool should_exit = false;

  bool win = false;

  bool first_move = true;

  bool board_changed = false;

  for (;;)
  {
#ifdef SHOW
    DrawBoard(game, false, board_changed);

    // נבקש קלט מהמשתמש/ת
    printw("\nquit anytime with \"q\"\n\n");
    printw("use 'w'=up, 'd'=right, 'x'=down, 'a'=left to move\n");
    printw("use space bar to reveal\n");
    printw("use `f` to flag an existing mine\n");
    refresh();
#endif // SHOW

    char c;
    int8_t msg_type;
    if (sock >= 0)
    {
      if (!get_message(sock, &msg_type, &c, NULL))
      {
        usleep(10);
        continue;
      }
    }
    else
    {
      c = getch();
    }

    board_changed = false;
    bool lose = false;
    int curr;
    switch (c)
    {
    case 'q':
      should_exit = true;
      break;
    case 'w': // up
      curr = game->pos.i - 1;
      if (curr >= 0)
        game->pos.i = curr;
      break;
    case 'x': // down
      curr = game->pos.i + 1;
      if (curr < game->config.rows)
        game->pos.i = curr;
      break;
    case 'a': // left
      curr = game->pos.j - 1;
      if (curr >= 0)
        game->pos.j = curr;
      break;
    case 'd': // right
      curr = game->pos.j + 1;
      if (curr < game->config.cols)
        game->pos.j = curr;
      break;
    case ' ':
      if (first_move)
      {
        PlaceMines(game);
        first_move = false;
      }
      lose = !RevealLocation(game);
      board_changed = true;
      break;
    case 'f':
      game->is_flagged_board[game->pos.i][game->pos.j] = !game->is_flagged_board[game->pos.i][game->pos.j];
      board_changed = true;
      break;
    }

    if (should_exit)
      break;

    // נבדוק האם המשתמש/ת ניצח, כלומר סיימ/ה לחשוף את כל התאים שאינם מוקשים
    win = CheckWin(game);
    if (lose || win)
    {
      send_message(sock, 2, &win, -1);
      break;
    }

    if (board_changed)
    {
      update_revealed_board(game);
      write_revealed_board(game, sock);
    }
  }

  DeInit(game);

  if (should_exit)
    return -1;

  return win;
}

bool GetConfigFromSock(struct Game *game, int sock, bool *should_continue)
{
  char msg[sizeof(game->config)];
  int8_t msg_type;
  if (!get_message(sock, &msg_type, msg, NULL))
  {
    usleep(10);
    *should_continue = true;
    return false;
  }

  if (msg_type == 1 && msg[0] == 'q')
    return true;

  memcpy(&game->config, msg, sizeof(game->config));

  return false;
}

bool GetConfigFromUser(struct Game *game)
{
  printw("enter level (1/2/3/4)\n1: Easy\n2: Intermediate\n3: Hard\n4: Custom\n");
  bool should_exit = false;
  while (true)
  {
    int key = getch();
    bool should_continue = false;
    switch (key)
    {
    case '1':
      game->config.rows = 9;
      game->config.cols = 9;
      game->config.mines = 10;
      break;
    case '2':
      game->config.rows = 16;
      game->config.cols = 16;
      game->config.mines = 40;
      break;
    case '3':
      game->config.rows = 16;
      game->config.cols = 30;
      game->config.mines = 99;
      break;
    case '4':
      printw("enter num rows: ");
      scanw("%d", &game->config.rows);
      if (game->config.rows > INT8_MAX) should_continue = true;
      printw("enter num cols: ");
      scanw("%d", &game->config.cols);
      if (game->config.cols > INT8_MAX) should_continue = true;
      printw("enter num mines: ");
      scanw("%d", &game->config.mines);
      break;
    case 'q':
      should_exit = true;
      break;
    default:
      should_continue = true;
      break;
    }
    refresh();
    if (!should_continue)
      break;
  }

  return should_exit;
}

void run_game(int sock)
{
  int ch;

  setlocale(LC_ALL, "");

  /* Curses Initialisations */
  initscr();
  use_default_colors();
  start_color();
  init_pair(1, -1, COLOR_GREEN);
  init_pair(2, -1, -1);
  init_pair(3, -1, COLOR_RED);
  raw();
  keypad(stdscr, TRUE);

  // srand(time(NULL));

  struct Game game;

  for (;;)
  {
    if (sock >= 0)
    {
      bool should_continue = false;
      if (GetConfigFromSock(&game, sock, &should_continue))
        break;
      if (should_continue)
        continue;
    }
    else
    {
      if (GetConfigFromUser(&game))
        break;
      noecho();
    }

    int ret = run_one_game(sock, &game);
    if (ret < 0)
      break;
  }

  endwin();
}

void CreateSocket(int *server_fd, int *new_socket)
{
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};

  // Creating socket file descriptor
  if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Attaching socket to the port
  if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
  {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Binding the socket to the address
  if (bind(*server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Listening for incoming connections
  if (listen(*server_fd, 3) < 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  printf("Waiting for connections...\n");

  if ((*new_socket = accept(*server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
  {
    perror("accept");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[])
{
  bool should_create_socket = argc > 1 && !strcmp(argv[1], "socket");

  int server_fd, new_socket = -1;
  if (should_create_socket)
  {
    CreateSocket(&server_fd, &new_socket);
  }

  run_game(new_socket);

  if (should_create_socket)
  {
    // Close the socket
    close(new_socket);
    close(server_fd);
  }

  return 0;
}