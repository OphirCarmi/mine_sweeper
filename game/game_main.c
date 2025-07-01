#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <locale.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>
#include <unistd.h>

#include "common/common.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define USE_NCURSES

static bool show = false;

struct Game;

struct _Cell {
  int x;
  int y;
  GtkImage *image;
  struct Game *game;
};
typedef struct _Cell Cell;

struct Game {
  pthread_t display_thread;

  Cell ***gtk_cells;
  GtkButton ***gtk_buttons;
  GtkWindow *display_window;

  int8_t **hidden_board;

  bool **is_revealed_board;
  bool **is_flagged_board;

  char *changed_cells;
  int changed_cells_len;

  struct Position pos;

  struct GameConfig config;
};

void GenerateRandomMines(struct Game *game, int **indices) {
  int num_cells = game->config.rows * game->config.cols;
  *indices = (int *)malloc(sizeof(*indices) * (num_cells - 1));
  int *ptr = *indices;
  for (int i = 0; i < num_cells; ++i) {
    if (i / game->config.cols == game->pos.i &&
        i % game->config.cols == game->pos.j)
      continue;
    *ptr = i;
    ++ptr;
  }
  for (int i = 0; i < game->config.mines; ++i) {
    int ind = i + rand() % (num_cells - 1 - i);
    int temp = (*indices)[ind];
    (*indices)[ind] = (*indices)[i];
    (*indices)[i] = temp;
  }
}

void PlaceMines(struct Game *game, int *indices_from_file) {
  int *indices = indices_from_file;
  if (!indices) GenerateRandomMines(game, &indices);
  for (int i = 0; i < game->config.mines; ++i) {
    int ind = indices[i];

    int row_ind = ind / game->config.cols;
    int col_ind = ind % game->config.cols;

    game->hidden_board[row_ind][col_ind] = -1;

    for (int k = 0; k < num_neighbours; ++k) {
      int neigh_row_ind = row_ind + neighbours[k][0];
      if (neigh_row_ind < 0 || neigh_row_ind >= game->config.rows) continue;

      int neigh_col_ind = col_ind + neighbours[k][1];
      // נבדוק שהוא לא משמאל או מימין ללוח
      if (neigh_col_ind < 0 || neigh_col_ind >= game->config.cols) continue;

      // נבדוק שהוא לא מוקש
      if (game->hidden_board[neigh_row_ind][neigh_col_ind] != -1) {
        // נוסיף אחד למנין המוקשים השכנים שלו (זוכרים שאתחלנו אותו לאפס בהתחלה?
        // חשוב חשוב)
        game->hidden_board[neigh_row_ind][neigh_col_ind]++;
      }
    }
  }

  free(indices);
}

void SetColor(struct Game *game, int i, int j, bool red) {
  if (game->pos.i == i && game->pos.j == j) {
    if (red)
      attrset(COLOR_PAIR(3));
    else
      attrset(COLOR_PAIR(1));
  } else {
    attrset(COLOR_PAIR(2));
  }
}

void PrintCellValue(struct Game *game, int i, int j, bool red,
                    bool board_changed) {
  if (board_changed) SetColor(game, i, j, red);
  printw(" ");
  SetColor(game, i, j, red);

  // נבדוק שהתא הזה כבר חשוף למשתמש
  if (game->is_revealed_board[i][j]) {
    if (game->hidden_board[i][j] == -1) {
      // אם הוא מוקש נסמנו בהתאם
      printw("*");
    } else {
      // אם הוא לא מוקש נדפיס למשתמש/ת את ערכו
      printw("%d", game->hidden_board[i][j]);
    }
  } else {
    if (game->is_flagged_board[i][j]) {
      printw("f");
    } else {
      // אם הוא לא חשוף למשתמש/ת, נשאיר אותו ריק
      printw(" ");
    }
  }
  attroff(COLOR_PAIR(1));
  if (board_changed) SetColor(game, i, j, red);
  printw(" ");
  attroff(COLOR_PAIR(1));
}

void PrintHorizontalLine(struct Game *game) {
  for (int j = 0; j < game->config.cols; ++j) {
    printw("----");
  }
  printw("-\n");
}

void DrawBoard(struct Game *game, bool red, bool board_changed) {
  clear();  // clear screen

  int num_flags = 0;
  for (int i = 0; i < game->config.rows; ++i) {
    for (int j = 0; j < game->config.cols; ++j) {
      num_flags += game->is_flagged_board[i][j];
    }
  }

  printw("Mine Sweeper, rows: %d, cols: %d, mines: %d, flags: %d\n",
         game->config.rows, game->config.cols, game->config.mines, num_flags);

  for (int i = 0; i < game->config.rows; ++i) {
    PrintHorizontalLine(game);

    for (int j = 0; j < game->config.cols; ++j) {
      printw("|");
      PrintCellValue(game, i, j, red, board_changed);
    }
    printw("|\n");
  }
  PrintHorizontalLine(game);
}

void reveal_cell(struct Game *game, int i, int j) {
  Cell *cell = game->gtk_cells[i][j];
  char s[64] = {0};

  snprintf(s, sizeof(s), "gtk_example/images/Minesweeper_%d.svg",
           (int)game->hidden_board[i][j]);
  
  gtk_image_set_from_file(cell->image, s);
}

void reveal_pos_cell(struct Game *game) {
  reveal_cell(game, game->pos.i, game->pos.j);
}

void flag_pos_cell(struct Game *game) {
  Cell *cell = game->gtk_cells[game->pos.i][game->pos.j];
  gtk_image_set_from_file(cell->image, "gtk_example/images/Minesweeper_flag.svg");
}

void reveal_green_mine(struct Game *game, int i, int j) {
  if (game->hidden_board[i][j] == -1) {
    Cell *cell = game->gtk_cells[i][j];
    gtk_image_set_from_file(cell->image, "gtk_example/images/Minesweeper_-1_green.svg");
  }
}

void reveal_red_mine(struct Game *game) {
  Cell *cell = game->gtk_cells[game->pos.i][game->pos.j];
  gtk_image_set_from_file(cell->image, "gtk_example/images/Minesweeper_-1_red.svg");
}

void RevealZeroes(struct Game *game) {
  LIST_HEAD(listhead, entry)
  head;
  struct entry {
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
  if (show) reveal_pos_cell(game);

  char *ptr = game->changed_cells;
  *ptr++ = game->pos.i;
  *ptr++ = game->pos.j;
  *ptr++ = game->hidden_board[game->pos.i][game->pos.j] + '0';

  while (head.lh_first != NULL) {
    int curr_row_ind = head.lh_first->row_ind;
    int curr_col_ind = head.lh_first->col_ind;

    np = head.lh_first;
    LIST_REMOVE(head.lh_first, entries);
    free(np);

    for (int k = 0; k < num_neighbours; ++k) {
      int neigh_row_ind = curr_row_ind + neighbours[k][0];
      if (neigh_row_ind < 0 || neigh_row_ind >= game->config.rows) continue;

      int neigh_col_ind = curr_col_ind + neighbours[k][1];
      if (neigh_col_ind < 0 || neigh_col_ind >= game->config.cols) continue;

      if (game->is_revealed_board[neigh_row_ind][neigh_col_ind]) continue;

      game->is_revealed_board[neigh_row_ind][neigh_col_ind] = true;
      
      *ptr++ = neigh_row_ind;
      *ptr++ = neigh_col_ind;
      *ptr++ = game->hidden_board[neigh_row_ind][neigh_col_ind] + '0';
      
      if (show) {
        reveal_cell(game, neigh_row_ind, neigh_col_ind);

        // DrawBoard(game, false, false);
        // refresh();
        usleep(100000);
      }

      if (game->hidden_board[neigh_row_ind][neigh_col_ind] != 0) continue;

      np = (struct entry *)malloc(
          sizeof(struct entry)); /* Insert at the head. */
      np->row_ind = neigh_row_ind;
      np->col_ind = neigh_col_ind;
      LIST_INSERT_HEAD(&head, np, entries);
    }
  }

  game->changed_cells_len = (ptr - game->changed_cells) / sizeof(struct Cell);
}

bool RevealLocation(struct Game *game) {
  // printf("pos %d %d, %d\n", (int)game->pos.i, (int)game->pos.j,
  // (int)game->hidden_board[game->pos.i][game->pos.j]);
  switch (game->hidden_board[game->pos.i][game->pos.j]) {
    case 0:
      RevealZeroes(game);
      return true;
    case -1:
      game->is_revealed_board[game->pos.i][game->pos.j] = true;
      game->changed_cells[0] = game->pos.i;
      game->changed_cells[1] = game->pos.j;
      game->changed_cells[2] = '*';
      game->changed_cells_len = 1;
      if (show) {
        reveal_red_mine(game);

        // DrawBoard(game, true, true);
        // printw("\n\nBOOOOOOOOOM!!!! GAME OVER!\n");
        // refresh();
        sleep(1);
      }
      return false;
    default:
      game->is_revealed_board[game->pos.i][game->pos.j] = true;
      if (show) reveal_pos_cell(game);
      game->changed_cells[0] = game->pos.i;
      game->changed_cells[1] = game->pos.j;
      game->changed_cells[2] =
          game->hidden_board[game->pos.i][game->pos.j] + '0';
      game->changed_cells_len = 1;
      return true;
  }
}

bool CheckWin(struct Game *game) {
  int sum_unrevealed = game->config.rows * game->config.cols;
  for (int i = 0; i < game->config.rows; ++i) {
    for (int j = 0; j < game->config.cols; ++j) {
      sum_unrevealed -= game->is_revealed_board[i][j];
    }
  }

  if (sum_unrevealed > game->config.mines) return false;

  int sum_flags = 0;
  for (int i = 0; i < game->config.rows; ++i) {
    for (int j = 0; j < game->config.cols; ++j) {
      sum_flags += game->is_flagged_board[i][j];
    }
  }

  if (sum_flags != game->config.mines) return false;

  for (int i = 0; i < game->config.rows; ++i) {
    for (int j = 0; j < game->config.cols; ++j) {
      game->is_revealed_board[i][j] = true;
      if (show) {
        reveal_cell(game, i, j);
        reveal_green_mine(game, i, j);
      }
    }
  }

  if (show) {
    // DrawBoard(game, false, false);

    // printw("\nYOU WON!!!\n");
    // refresh();
    sleep(1);
  }
  return true;
}

static void handle_button_click(Cell *cell, GtkButton *button) {
  // cell->image = (GtkImage *)gtk_image_new_from_file(
  //     "gtk_example/images/Minesweeper_1.svg");
  // gtk_button_set_image(button, GTK_WIDGET(cell->image));
  // g_warning("clicked x=%d, y=%d\n", cell->x, cell->y);
}

void *init_display_gtk_func(void *args) {
  struct Game *game = (struct Game *)args;

  GtkTable *table;
  int x;
  int y;
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic push
  table = (GtkTable *)gtk_table_new(game->config.cols, game->config.rows, TRUE);
#pragma GCC diagnostic pop
  for (x = 0; x < game->config.cols; ++x)
    for (y = 0; y < game->config.rows; ++y) {
      GtkButton *button;
      Cell *cell;
      cell = g_malloc0(sizeof(Cell));
      cell->x = x;
      cell->y = y;
      cell->image = (GtkImage *)gtk_image_new_from_file(
          "gtk_example/images/Minesweeper_unopened_square.svg");
      gtk_widget_show(GTK_WIDGET(cell->image));
      button = (GtkButton *)gtk_button_new();
      game->gtk_buttons[y][x] = button;
      gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(cell->image));
      gtk_widget_show(GTK_WIDGET(button));
      g_object_set_data(G_OBJECT(button), "cell", cell);
      game->gtk_cells[y][x] = cell;
      gtk_table_attach(table, GTK_WIDGET(button), x, x + 1, y, y + 1,
                       GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
      // g_signal_connect_swapped(G_OBJECT(button), "clicked",
      //                          G_CALLBACK(handle_button_click), cell);
    }
  gtk_widget_show(GTK_WIDGET(table));
  game->display_window = (GtkWindow *)gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(G_OBJECT(game->display_window), "delete-event", G_CALLBACK(gtk_main_quit),
                   NULL);
  gtk_container_add(GTK_CONTAINER(game->display_window), GTK_WIDGET(table));

  // gtk_window_set_default_size(GTK_WINDOW(game->display_window), 400, 100);
  // gtk_window_set_resizable (GTK_WINDOW(game->display_window), FALSE);

  gtk_widget_show(GTK_WIDGET(game->display_window));
  gtk_main();
}

void init_display_gtk(struct Game *game) {
  game->gtk_cells =
      (Cell ***)malloc(game->config.rows * sizeof(*game->gtk_cells));
  for (int i = 0; i < game->config.rows; ++i)
    game->gtk_cells[i] =
        (Cell **)malloc(game->config.cols * sizeof(**game->gtk_cells));

  game->gtk_buttons =
      (GtkButton ***)malloc(game->config.rows * sizeof(*game->gtk_buttons));
  for (int i = 0; i < game->config.rows; ++i)
    game->gtk_buttons[i] =
        (GtkButton **)malloc(game->config.cols * sizeof(**game->gtk_buttons));

  pthread_create(&game->display_thread, NULL, init_display_gtk_func,
                 (void *)game);
}

void Init(struct Game *game) {
  game->hidden_board =
      (int8_t **)malloc(game->config.rows * sizeof(*game->hidden_board));
  for (int i = 0; i < game->config.rows; ++i)
    game->hidden_board[i] = (int8_t *)calloc(game->config.cols, 1);

  game->is_flagged_board =
      (bool **)malloc(game->config.rows * sizeof(*game->is_flagged_board));
  for (int i = 0; i < game->config.rows; ++i)
    game->is_flagged_board[i] = (bool *)calloc(game->config.cols, 1);

  game->is_revealed_board =
      (bool **)malloc(game->config.rows * sizeof(*game->is_revealed_board));
  for (int i = 0; i < game->config.rows; ++i)
    game->is_revealed_board[i] = (bool *)calloc(game->config.cols, 1);

  int num_cells = game->config.rows * game->config.cols;
  game->changed_cells = (char *)malloc(num_cells * sizeof(struct Cell));
  game->changed_cells_len = 0;

  memset(&game->pos, 0, sizeof(game->pos));

  if (show) init_display_gtk(game);
}

void deinit_display_gtk(struct Game *game) {
  for (int i = 0; i < game->config.rows; ++i) free(game->gtk_buttons[i]);
  free(game->gtk_buttons);

  for (int i = 0; i < game->config.rows; ++i) free(game->gtk_cells[i]);
  free(game->gtk_cells);

  gtk_window_close(game->display_window);
  gtk_main_quit();

  pthread_join(game->display_thread, NULL);
}

void DeInit(struct Game *game) {
  for (int i = 0; i < game->config.rows; ++i) free(game->hidden_board[i]);
  free(game->hidden_board);

  for (int i = 0; i < game->config.rows; ++i) free(game->is_flagged_board[i]);
  free(game->is_flagged_board);

  for (int i = 0; i < game->config.rows; ++i) free(game->is_revealed_board[i]);
  free(game->is_revealed_board);

  free(game->changed_cells);

  if (show) deinit_display_gtk(game);
}

void write_revealed_board(struct Game *game, int sock) {
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

  int cell_size = game->changed_cells_len * sizeof(struct Cell);
  int tot_size = cell_size + sizeof(game->pos);
  char *mem = (char *)malloc(tot_size);
  char *ptr = mem;
  if (cell_size) {
    memcpy(ptr, game->changed_cells, cell_size);
    ptr += cell_size;
  }
  memcpy(ptr, &game->pos, sizeof(game->pos));

  send_message(sock, 0, mem, tot_size);

  free(mem);
}

bool GetConfigFromSock(struct Game *game, int sock, bool *should_continue) {
  char msg[sizeof(game->config)];
  int8_t msg_type;
  if (!get_message(sock, &msg_type, msg, NULL)) {
    usleep(1);
    *should_continue = true;
    return false;
  }

  if (msg_type == 1 && msg[0] == 'q') return true;

  memcpy(&game->config, msg, sizeof(game->config));

  return false;
}

void parse_game_from_file(const char *game_file, struct Game *game,
                          int **indices) {
  FILE *f = fopen(game_file, "r");

  size_t len = 0;
  ssize_t read;

  if (f == NULL) exit(EXIT_FAILURE);

  int rows, cols;
  read = fscanf(f, "%dx%d", &rows, &cols);
  if (read != 2 || rows <= 0 || rows > 127 || cols <= 0 || cols > 127) {
    printf("failed to read config\n");
    exit(-1);
  }

  game->config.rows = rows;
  game->config.cols = cols;

  long curr = ftell(f);

  game->config.mines = 0;
  int i, j;
  while ((read = fscanf(f, "%d,%d", &i, &j)) == 2) {
    game->config.mines++;
  }

  *indices = (int *)malloc(game->config.mines * sizeof(*indices));

  fseek(f, curr, SEEK_SET);

  int k = 0;
  while ((read = fscanf(f, "%d,%d", &i, &j)) == 2) {
    if (i < 0 || i > 127 || j < 0 || j > 127) exit(-1);
    (*indices)[k++] = i * game->config.cols + j;
  }

  fclose(f);
}

bool GetConfigFromUser(struct Game *game) {
  printw(
      "enter level (1/2/3/4)\n1: Easy\n2: Intermediate\n3: Hard\n4: Custom\n");
  bool should_exit = false;
  for (;;) {
    int key = getch();
    bool should_continue = false;
    switch (key) {
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
        scanw("%c", &game->config.rows);
        if (game->config.rows > INT8_MAX) should_continue = true;
        printw("enter num cols: ");
        scanw("%c", &game->config.cols);
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
    if (!should_continue) break;
  }

  return should_exit;
}

int GetConfig(int sock, struct Game *game, const char *game_file_path,
              int **indices) {
  if (game_file_path) {
    parse_game_from_file(game_file_path, game, indices);
    return 0;
  }
  if (sock >= 0) {
    bool should_continue = false;
    if (GetConfigFromSock(game, sock, &should_continue)) return 1;
    if (should_continue) return 2;
  } else {
    if (GetConfigFromUser(game)) return 1;
    noecho();
  }
  return 0;
}

int run_one_game(int sock, const char *game_file_path) {
  struct Game game;

  int *indices = NULL;
  int config_ret = GetConfig(sock, &game, game_file_path, &indices);
  switch (config_ret) {
    case 1:
      return -1;
    case 2:
      return 2;
  }

  Init(&game);
  write_revealed_board(&game, sock);

  bool should_exit = false;

  bool first_move = true;

  bool board_changed = false;

  for (int iter = 0;; ++iter) {
    // printf("iter %d\n", iter);
    if (show) {
      // DrawBoard(&game, false, board_changed);

      // // נבקש קלט מהמשתמש/ת
      // printw("\nquit anytime with \"q\"\n\n");
      // printw("use 'w'=up, 'd'=right, 'x'=down, 'a'=left to move\n");
      // printw("use space bar to reveal\n");
      // printw("use `f` to flag an existing mine\n");
      // refresh();
      usleep(100000);
    }

    char c;
    int8_t msg_type;
    if (sock >= 0) {
      if (!get_message(sock, &msg_type, &c, NULL)) {
        usleep(1);
        continue;
      }
    } else {
      c = getch();
    }

    // printf("c '%c'\n", c);

    board_changed = false;
    bool lose = false;
    int curr;
    switch (c) {
      case 'q':
        should_exit = true;
        break;
      case 'w':  // up
        curr = game.pos.i - 1;
        if (curr >= 0) game.pos.i = curr;
        break;
      case 'x':  // down
        curr = game.pos.i + 1;
        if (curr < game.config.rows) game.pos.i = curr;
        break;
      case 'a':  // left
        curr = game.pos.j - 1;
        if (curr >= 0) game.pos.j = curr;
        break;
      case 'd':  // right
        curr = game.pos.j + 1;
        if (curr < game.config.cols) game.pos.j = curr;
        break;
      case ' ':
        if (first_move) {
          PlaceMines(&game, indices);
          first_move = false;
        }
        lose = !RevealLocation(&game);
        board_changed = true;
        break;
      case 'f':
        game.is_flagged_board[game.pos.i][game.pos.j] =
            !game.is_flagged_board[game.pos.i][game.pos.j];
        if (show) flag_pos_cell(&game);
        game.changed_cells[0] = game.pos.i;
        game.changed_cells[1] = game.pos.j;
        game.changed_cells[2] =
            game.is_flagged_board[game.pos.i][game.pos.j] ? 'f' : ' ';
        game.changed_cells_len = 1;
        board_changed = true;
        break;
    }

    if (should_exit) break;

    bool win = CheckWin(&game);
    if (lose || win) {
      send_message(sock, 2, &win, -1);
      break;
    }

    if (board_changed) write_revealed_board(&game, sock);
  }

  DeInit(&game);

  if (should_exit) return -1;

  return 0;
}

void init_curses() {
#ifdef USE_NCURSES
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
#endif  // USE_NCURSES
}

void run_game(int sock, const char *game_file_path) {
  int ch;

  if (show) init_curses();

  // srand(time(NULL));

  for (int g = 0;; ++g) {
    // if (g > 368) show = true;

    // printf("g %d\n", g);

    int ret = run_one_game(sock, game_file_path);
    // printf("ret %d\n", ret);
    if (ret < -1) continue;
    if (ret < 0) break;
  }

  if (show) endwin();
}

void CreateSocket(int *server_fd, int *new_socket) {
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  // Creating socket file descriptor
  if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Attaching socket to the port
  if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Binding the socket to the address
  if (bind(*server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Listening for incoming connections
  if (listen(*server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  printf("Waiting for connections...\n");

  if ((*new_socket = accept(*server_fd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[]) {
  bool should_create_socket = false;
  for (int i = 1; i < argc; ++i) {
    should_create_socket = !strcmp(argv[i], "socket");
    show = !strcmp(argv[i], "show");
  }
 
  if (show)
    gtk_init(NULL, NULL);

  char *game_file_path = NULL;
  if (argc > 3) game_file_path = strdup(argv[3]);

  int server_fd, new_socket = -1;
  if (should_create_socket) CreateSocket(&server_fd, &new_socket);

  run_game(new_socket, game_file_path);

  if (should_create_socket) {
    // Close the socket
    close(new_socket);
    close(server_fd);
  }

  return 0;
}