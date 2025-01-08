#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <conio.h>

#ifdef _WIN32
#define clrscr() system("cls")
#elif defined(__linux__)
#define clrscr() system("clear")
#else
#error Platform not supported
#endif

#define KB_UP 72
#define KB_DOWN 80
#define KB_LEFT 75
#define KB_RIGHT 77

typedef struct {
  int row;
  int col;
} pos;

typedef struct {
  pos tiles[4];
} shape_tiles;

typedef struct {
  int shape;
  pos p;
  int rotation;
  int num_rotations;
} block;

// shapes
// ##  ## ##
// ## ##   ##
//
// #
// # #   #
// # #   # ###
// # ## ##  #

const shape_tiles O[1] = {
  { .tiles = { {0, 0}, {0, -1}, {-1, 0}, {-1, -1} } }
};

const shape_tiles I[4] = {
  { .tiles = { {0, 0}, {0, -1}, {0, -2}, {0, 1} } },
  { .tiles = { {0, 0}, {-1, 0}, {1, 0}, {2, 0} } },
  { .tiles = { {1, 0}, {1, 1}, {1, -1}, {1, -2} } },
  { .tiles = { {0, -1}, {-1, -1}, {1, -1}, {2, -1} } }
};

const shape_tiles T[4] = {
  { .tiles = { {0, 0}, {0, -1}, {0, -2}, {-1, -1} } },
  { .tiles = { {0, 0}, {0, -1}, {-1, -1}, {1, -1} } },
  { .tiles = { {0, 0}, {0, -1}, {1, -1}, {0, -2} } },
  { .tiles = { {0, -1}, {1, -1}, {-1, -1}, {0, -2} } }
};

const shape_tiles Z[2] = {
  { .tiles = { {0, 0}, {0, -1}, {-1, -1}, {-1, -2} } },
  { .tiles = { {0, 0}, {-1, 0}, {0, -1}, {1, -1} } }
};

const shape_tiles S[2] = {
  { .tiles = { {-1, 0}, {0, -1}, {-1, -1}, {0, -2} } },
  { .tiles = { {0, 0}, {-1, -1}, {0, -1}, {1, 0} } }
};

const shape_tiles L[4] = {
  { .tiles = { {0, 0}, {-1, 0}, {0, -1}, {0, -2} } },
  { .tiles = { {1, 0}, {0, -1}, {-1, -1}, {1, -1} } },
  { .tiles = { {0, 0}, {0, -1}, {0, -2}, {1, -2} } },
  { .tiles = { {0, -1}, {1, -1}, {-1, -1}, {-1, -2} } }
};

const shape_tiles J[4] = {
  { .tiles = { {0, 0}, {0, -1}, {0, -2}, {-1, -2} } },
  { .tiles = { {-1, 0}, {-1, -1}, {0, -1}, {1, -1} } },
  { .tiles = { {0, 0}, {1, 0}, {0, -1}, {0, -2} } },
  { .tiles = { {0, -1}, {-1, -1}, {1, -1}, {1, -2} } }
};

const int HEIGHT = 23;
const int WIDTH = 10;
int KB_code = 0;
int sleep_amount = 200;
pos block_spawn = {1, 5};

void free_board(char **board, int height);
void sleep_ms(int milliseconds);
char **init_board();
char **board_for_display();
void display_board();
void free_board();
block *generate_block();
shape_tiles get_tiles(block *curr_block);
void update_board(char *(**board), block *curr_block, bool remove);
bool is_valid(char **board, block *curr_block);
bool is_obstructed(char **board, block *curr_block);
void place(char *(**board), block *curr_block);
void clear_rows(char *(**board));

int main(int argc, char *argv[]) {
  char **board = init_board();

  srand( time(NULL) );

  block *curr_block = NULL;
  int i = 0;
  bool game_over = false;
  while (!game_over) {
    if (!curr_block) {
      curr_block = generate_block();
      if (!is_valid(board, curr_block) || is_obstructed(board, curr_block)) {
        place(&board, curr_block);
        game_over = true;
        break;
      }
    }
    clrscr();
    update_board(&board, curr_block, true);
    if (kbhit() != 0) {
      KB_code = getch();
      if (KB_code == 224) {
        KB_code = getch();
        switch (KB_code) {
          case KB_LEFT:
            curr_block->p.col--;
            if (!is_valid(board, curr_block)) {
              curr_block->p.col++;
            }
            break;
          case KB_RIGHT:
            curr_block->p.col++;
            if (!is_valid(board, curr_block)) {
              curr_block->p.col--;
            }
            break;
          case KB_UP:
            if (curr_block->shape != 0) {
              int original_rotation = curr_block->rotation;
              curr_block->rotation = (original_rotation + 1) % curr_block->num_rotations;
              if (!is_valid(board, curr_block)) {
                curr_block->rotation = original_rotation;
              }
            }
            break;
          case KB_DOWN:
            curr_block->p.row++;
            if (!is_valid(board, curr_block)) {
              curr_block->p.row--;
            }
            break;
        }
      } else if (KB_code == ' ') {
        while (is_valid(board, curr_block)) {
          curr_block->p.row++;
        }
        curr_block->p.row--;
        place(&board, curr_block);
        free(curr_block);
        curr_block = NULL;
        clear_rows(&board);
        display_board(board);
        sleep_ms(sleep_amount);
        continue;
      } else if (KB_code == 'q') {
        printf("Quitting...\n");
        break;
      }
    }

    i = (i + 1) % 10;
    if (i % 5 == 4) {
      curr_block->p.row++;
      if (!is_valid(board, curr_block)) {
        curr_block->p.row--;
      }
    }
    if (i == 9) {
      if (is_obstructed(board, curr_block)) {
        place(&board, curr_block);
        free(curr_block);
        curr_block = NULL;
      }
    }

    if (curr_block) {
      update_board(&board, curr_block, false);
    }
    clear_rows(&board);
    display_board(board);
    sleep_ms(sleep_amount);
  }

  if (game_over) {
    clrscr();
    display_board(board);
    printf("Game over!");
  }

  free_board(board, HEIGHT);
  free(curr_block);

  return 0;
}

block *generate_block(char **board) {
  block *new_block = malloc(sizeof(block));
  new_block->p = block_spawn;
  int shape = rand() % 7;
  new_block->shape = shape;
  new_block->rotation = 0;
  int num_rotations;
  switch (shape) {
    case 0:
      num_rotations = 1;
      break;
    case 3:
      num_rotations = 2;
      break;
    case 4:
      num_rotations = 2;
      break;
    default:
      num_rotations = 4;
      break;
  }
  new_block->num_rotations = num_rotations;
  return new_block;
}

void update_board(char *(**board), block *curr_block, bool remove) {
  pos p = curr_block->p;
  shape_tiles shaped_tiles = get_tiles(curr_block);
  pos *tiles = shaped_tiles.tiles;
  int r, c;
  for (int i = 0; i < 4; i++) {
    r = p.row + tiles[i].row, c = p.col + tiles[i].col;
    if (remove) {
      if (r < 3) {
        (*board)[r][c] = ' ';
      } else {
        (*board)[r][c] = '.';
      }
    } else {
      (*board)[r][c] = '#';
    }
  }
}

shape_tiles get_tiles(block *curr_block) {
  int rotation = curr_block->rotation;
  int shape = curr_block->shape;
  shape_tiles tiles;
  switch (shape) {
    case 0:
      tiles = O[rotation];
      break;
    case 1:
      tiles = I[rotation];
      break;
    case 2:
      tiles = T[rotation];
      break;
    case 3:
      tiles = Z[rotation];
      break;
    case 4:
      tiles = S[rotation];
      break;
    case 5:
      tiles = L[rotation];
      break;
    case 6:
      tiles = J[rotation];
      break;
  }
  return tiles;
}

bool is_valid(char **board, block *curr_block) {
  shape_tiles shaped_tiles = get_tiles(curr_block);
  pos *tiles = shaped_tiles.tiles;
  pos p = curr_block->p;
  int r, c;
  for (int i = 0; i < 4; i++) {
    r = p.row + tiles[i].row, c = p.col + tiles[i].col;
    if (r < 0 || r >= HEIGHT || c < 0 || c >= WIDTH || board[r][c] == '#') {
      return false;
    }
  }
  return true;
}

bool is_obstructed(char **board, block *curr_block) {
  shape_tiles shaped_tiles = get_tiles(curr_block);
  pos *tiles = shaped_tiles.tiles;
  pos p = curr_block->p;
  int bottom_row_len = 0;
  pos *bottom_row = NULL;
  int r, c;
  bool nothing_below;
  for (int i = 0; i < 4; i++) {
    r = p.row + tiles[i].row, c = p.col + tiles[i].col;
    nothing_below = true;
    for (int j = 0; j < 4; j++) {
      if (i == j) {
        continue;
      }
      if (r + 1 == p.row + tiles[j].row && c == p.col + tiles[j].col) {
        nothing_below = false;
      }
    }
    if (nothing_below) {
      bottom_row = realloc(bottom_row, (bottom_row_len + 1) * sizeof(pos));
      bottom_row[bottom_row_len].row = r;
      bottom_row[bottom_row_len].col = c;
      bottom_row_len++;
    }
  }

  for (int i = 0; i < bottom_row_len; i++) {
    r = bottom_row[i].row, c = bottom_row[i].col;
    if (r + 1 == HEIGHT || board[r + 1][c] == '#') {
      return true;
    }
  }

  free(bottom_row);
  return false;
}

void place(char *(**board), block *curr_block) {
  shape_tiles shaped_tiles = get_tiles(curr_block);
  pos *tiles = shaped_tiles.tiles;
  pos p = curr_block->p;
  int r, c;
  for (int i = 0; i < 4; i++) {
    r = p.row + tiles[i].row, c = p.col + tiles[i].col;
    (*board)[r][c] = '#';
  }
}

void clear_rows(char *(**board)) {
  bool filled;
  for (int i = 0; i < HEIGHT; i++) {
    filled = true;
    for (int j = 0; j < WIDTH; j++) {
      if ((*board)[i][j] != '#') {
        filled = false;
      }
    }
    if (filled) {
      for (int i2 = i; i2 > 3; i2--) {
        for (int j = 0; j < WIDTH; j++) {
          (*board)[i2][j] = (*board)[i2 - 1][j];
        }
      }
    }
  }
}

void sleep_ms(int milliseconds) {
  clock_t start_time = clock();
  while (clock() < start_time + milliseconds);
}

char **init_board() {
  char **board = malloc(HEIGHT * sizeof(char *));
  for (int i = 0; i < HEIGHT ; i++) {
    board[i] = calloc(WIDTH, sizeof(char));
    for (int j = 0; j < WIDTH; j++) {
      if (i < 3) {
        board[i][j] = ' ';
      } else {
        board[i][j] = '.';
      }
    }
  }
  return board;
}

char **board_for_display(char **board) {
  char **result = malloc((HEIGHT + 1) * sizeof(char *));
  char buffer[WIDTH + 2];
  for (int i = 0; i < HEIGHT; i++) {
    result[i] = malloc((WIDTH + 2) * sizeof(char));
    buffer[0] = '|';
    for (int j = 0; j < WIDTH; j++) {
      buffer[j + 1] = board[i][j];
    }
    buffer[WIDTH + 1] = '|';
    strcpy(result[i], buffer);
  }
  // buffer[0] = '\\';
  for (int i = 0; i < WIDTH + 2; i++) {
    buffer[i] = '-';
  }
  // buffer[WIDTH + 1] = '/';
  result[HEIGHT] = malloc((WIDTH + 2) * sizeof(char));
  strcpy(result[HEIGHT], buffer);

  return result;
}

void display_board(char **board) {
  char **board_display = board_for_display(board);
  for (int i = 0; i < HEIGHT + 1; i++) {
    for (int j = 0; j < WIDTH + 2; j++) {
      printf("%c", board_display[i][j]);
    }
    printf("\n");
  }
  free_board(board_display, HEIGHT + 1);
}

void free_board(char **board, const int height) {
  for (int i = 0; i < height; i++) {
    free(board[i]);
  }
  free(board);
}

