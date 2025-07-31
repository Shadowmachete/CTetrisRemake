#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <conio.h>

#ifdef _WIN32
#define clrscr() system("cls")
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

void free_grid(char **board, int height);
void sleep_ms(int milliseconds);
char **init_board();
char **board_for_display(char **board);
char **block_queue_for_display(block **block_queue);
void display_board_and_block_queue(char **board, block **block_queue);
block *generate_block();
shape_tiles get_tiles(block *curr_block, int rotation);
void update_board(char *(**board), block *curr_block, bool remove);
bool is_valid(char **board, block *curr_block);
bool is_obstructed(char **board, block *curr_block);
void place(char *(**board), block *curr_block);
void clear_rows(char *(**board), int *score);
void swap(block **curr_block, block **held_block);
void display_held(block *held_block);
void display(char **board, block *held_block, int sleep_amount, int *score, block **block_queue);

int main(int argc, char *argv[]) {
  char **board = init_board();

  srand( time(NULL) );

  block **block_queue = malloc(4 * sizeof(block *));
  for (int i = 0; i < 4; i++) {
    block_queue[i] = generate_block();
  }
  block *curr_block = NULL;
  block *held_block = NULL;
  int score = 0;
  int i = 0;
  bool game_over = false;
  while (!game_over) {
    if (!curr_block) {
      curr_block = block_queue[0];
      for (int i = 0; i < 3; i++) {
        block_queue[i] = block_queue[i + 1];
      }
      block_queue[3] = generate_block();
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
        display(board, held_block, sleep_amount, &score, block_queue);
        continue;
      } else if (KB_code == 'c') {
        swap(&curr_block, &held_block);
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
    display(board, held_block, sleep_amount, &score, block_queue);
  }

  if (game_over) {
    clrscr();
    if (held_block != NULL) {
      display_held(held_block);
    }
    display_board_and_block_queue(board, block_queue);
    printf("Game over!\nYour score was %d", score);
  }

  free_grid(board, HEIGHT);
  free(curr_block);
  free(held_block);
  for (int i = 0; i < 4; i++) {
    free(block_queue[i]);
  }
  free(block_queue);

  return 0;
}

block *generate_block() {
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
  shape_tiles shaped_tiles = get_tiles(curr_block, -1);
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

shape_tiles get_tiles(block *curr_block, int rotation) {
  if (rotation == -1) {
    rotation = curr_block->rotation;
  }

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
  shape_tiles shaped_tiles = get_tiles(curr_block, -1);
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
  shape_tiles shaped_tiles = get_tiles(curr_block, -1);
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
  shape_tiles shaped_tiles = get_tiles(curr_block, -1);
  pos *tiles = shaped_tiles.tiles;
  pos p = curr_block->p;
  int r, c;
  for (int i = 0; i < 4; i++) {
    r = p.row + tiles[i].row, c = p.col + tiles[i].col;
    (*board)[r][c] = '#';
  }
}

void clear_rows(char *(**board), int *score) {
  bool filled;
  for (int i = 0; i < HEIGHT; i++) {
    filled = true;
    for (int j = 0; j < WIDTH; j++) {
      if ((*board)[i][j] != '#') {
        filled = false;
      }
    }
    if (filled) {
      (*score)++;
      for (int i2 = i; i2 > 3; i2--) {
        for (int j = 0; j < WIDTH; j++) {
          (*board)[i2][j] = (*board)[i2 - 1][j];
        }
      }
    }
  }
}

void swap(block **curr_block, block **held_block) {
  if (*held_block == NULL) {
    *held_block = *curr_block;
    *curr_block = generate_block();
    (*curr_block)->p = (*held_block)->p;
  } else {
    block *temp = *curr_block;
    (*held_block)->p = temp->p;
    *curr_block = *held_block;
    *held_block = temp;
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
  char **result = malloc((HEIGHT + 2) * sizeof(char *));
  char buffer[WIDTH + 2];
  for (int i = 0; i < WIDTH + 2; i++) {
    buffer[i] = '-';
  }
  result[0] = malloc((WIDTH + 2) * sizeof(char));
  strcpy(result[0], buffer);

  for (int i = 1; i < HEIGHT + 1; i++) {
    result[i] = malloc((WIDTH + 2) * sizeof(char));
    buffer[0] = '|';
    for (int j = 0; j < WIDTH; j++) {
      buffer[j + 1] = board[i - 1][j];
    }
    buffer[WIDTH + 1] = '|';
    strcpy(result[i], buffer);
  }
  // buffer[0] = '\\';
  for (int i = 0; i < WIDTH + 2; i++) {
    buffer[i] = '-';
  }
  // buffer[WIDTH + 1] = '/';
  result[HEIGHT + 1] = malloc((WIDTH + 2) * sizeof(char));
  strcpy(result[HEIGHT + 1], buffer);


  return result;
}

char **block_queue_for_display(block **block_queue) {
  int height = 13;
  char **result = malloc(height * sizeof(char *));
  char buffer[WIDTH + 1];
  for (int i = 0; i < height; i++) {
    result[i] = malloc((WIDTH + 1) * sizeof(char));
  }
  for (int i = 0; i < WIDTH + 1; i++) {
    buffer[i] = '-';
  }
  strcpy(result[0], buffer);

  for (int i = 0; i < 4; i++) {
    shape_tiles shaped_tiles = get_tiles(block_queue[i], 0);
    pos *tiles = shaped_tiles.tiles;
    pos p0 = { 1 + tiles[0].row, WIDTH / 2 + tiles[0].col };
    pos p1 = { 1 + tiles[1].row, WIDTH / 2 + tiles[1].col };
    pos p2 = { 1 + tiles[2].row, WIDTH / 2 + tiles[2].col };
    pos p3 = { 1 + tiles[3].row, WIDTH / 2 + tiles[3].col };

    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < WIDTH; k++) {
        if ((j == p0.row && k == p0.col) || (j == p1.row && k == p1.col) || (j == p2.row && k == p2.col) || (j == p3.row && k == p3.col)) {
          buffer[k] = '#';
        } else {
          buffer[k] = ' ';
        }
      }
      buffer[WIDTH] = '|';
      strcpy(result[i * 3 + j + 1], buffer);
      // printf("%s\n", result[i * 3 + j]);
    }
    if (i < 3) {
      for (int k = 0; k < WIDTH; k++) {
        buffer[k] = ' ';
      }
      buffer[WIDTH] = '|';
      strcpy(result[(i + 1) * 3], buffer);
      // printf("%s\n", result[(i + 1) * 3]);
    }
  }

  for (int i = 0; i < WIDTH + 1; i++) {
    buffer[i] = '-';
  }
  strcpy(result[height - 1], buffer);

  // for (int i = 0; i < height + 1; i++) {
  //   for (int j = 0; j < WIDTH + 1; j++) {
  //     printf("%c", result[i][j]);
  //   }
  //   printf("\n");
  // }
  return result;
}

void display_board_and_block_queue(char **board, block **block_queue) {
  char **board_display = board_for_display(board);
  char **block_queue_display = block_queue_for_display(block_queue);
  for (int i = 0; i < HEIGHT + 2; i++) {
    for (int j = 0; j < WIDTH + 2; j++) {
      printf("%c", board_display[i][j]);
    }
    if (i < 13) {
      for (int j = 0; j < WIDTH + 1; j++) {
        printf("%c", block_queue_display[i][j]);
      }
    }
    printf("\n");
  }
  free_grid(board_display, HEIGHT + 2);
  free_grid(block_queue_display, 13);
}

void display_held(block *held_block) {
  for (int i = 0; i < WIDTH + 2; i++) {
    printf("-");
  }
  printf("\n");

  shape_tiles shaped_tiles = get_tiles(held_block, 0);
  pos *tiles = shaped_tiles.tiles;
  pos p0 = { 1 + tiles[0].row, WIDTH / 2 + tiles[0].col };
  pos p1 = { 1 + tiles[1].row, WIDTH / 2 + tiles[1].col };
  pos p2 = { 1 + tiles[2].row, WIDTH / 2 + tiles[2].col };
  pos p3 = { 1 + tiles[3].row, WIDTH / 2 + tiles[3].col };

  for (int i = 0; i < 2; i++) {
    printf("|");
    for (int j = 0; j < WIDTH; j++) {
      if ((i == p0.row && j == p0.col) || (i == p1.row && j == p1.col) || (i == p2.row && j == p2.col) || (i == p3.row && j == p3.col)) {
        printf("#");
      } else {
        printf(" ");
      }
    }
    printf("|\n");
  }
}

void display(char **board, block *held_block, int sleep_amount, int *score, block **block_queue) {
  clear_rows(&board, score);
  if (held_block != NULL) {
    display_held(held_block);
  }
  display_board_and_block_queue(board, block_queue);
  printf("Score: %d", *score);
  sleep_ms(sleep_amount);
}

void free_grid(char **board, const int height) {
  for (int i = 0; i < height; i++) {
    free(board[i]);
  }
  free(board);
}

