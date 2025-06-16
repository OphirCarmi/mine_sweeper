#include <gtk/gtk.h>

#define BOARD_WIDTH 9
#define BOARD_HEIGHT 9

struct _Cell {
  int x;
  int y;
  GtkImage* image;
};
typedef struct _Cell Cell;

static void handle_button_click(Cell* cell, GtkButton* button) {
  g_warning("clicked x=%d, y=%d\n", cell->x, cell->y);
}

int main(int argc, char* argv[]) {
  Cell* cells[BOARD_WIDTH][BOARD_HEIGHT];
  GtkWindow* window;
  GtkTable* table;
  int x;
  int y;
  gtk_init(&argc, &argv);
  table = (GtkTable*)gtk_table_new(BOARD_WIDTH, BOARD_HEIGHT, TRUE);
  for (x = 0; x < BOARD_WIDTH; ++x)
    for (y = 0; y < BOARD_HEIGHT; ++y) {
      GtkButton* button;
      Cell* cell;
      cell = g_malloc0(sizeof(Cell));
      cell->x = x;
      cell->y = y;
      cell->image = (GtkImage*)gtk_image_new_from_file(
          "/home/ophir/Downloads/Minesweeper_1.svg");
      gtk_widget_show(GTK_WIDGET(cell->image));
      button = (GtkButton*)gtk_button_new();
      gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(cell->image));
      gtk_widget_show(GTK_WIDGET(button));
      g_object_set_data(G_OBJECT(button), "cell", cell);
      cells[x][y] = cell;
      gtk_table_attach(table, GTK_WIDGET(button), x, x + 1, y, y + 1,
                       GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
      g_signal_connect_swapped(G_OBJECT(button), "clicked",
                               G_CALLBACK(handle_button_click), cell);
    }
  gtk_widget_show(GTK_WIDGET(table));
  window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(gtk_main_quit),
                   NULL);
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(table));
  gtk_widget_show(GTK_WIDGET(window));
  gtk_main();
  return (0);
}