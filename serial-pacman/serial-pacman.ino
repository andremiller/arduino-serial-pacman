#include <avr/pgmspace.h>

#define DIR_RIGHT 0
#define DIR_DOWN  1
#define DIR_LEFT  2
#define DIR_UP    3

// Character positions
byte pacman_x = 13;
byte pacman_y = 26;
byte pacman_direction = DIR_LEFT;
byte pacman_planned_direction = DIR_LEFT;
float pacman_speed = 0.8; // Percentage of base speed
int ghost_eat_counter = 0; // Number of ghosts eaten per energizer
unsigned long pacman_update_counter = 0; // How much time has elapsed since the laste position update for pacman
long player_score = 0;
long player_score_previous = -1;
int level = 1;
int dots_eaten = 0;
#define PLAYER_ALIVE 0
#define PLAYER_DEAD 1
byte player_status = PLAYER_ALIVE;

unsigned long scatter_chase_timer = 0;
unsigned long fright_timer = 0;

// The ghosts
#define BLINKY 0
#define PINKY  1
#define INKY   2
#define CLYDE  3

// Game modes
#define CHASE   0
#define SCATTER 1
#define FRIGHT  2

// Ghost status
#define ACTIVE 3 // Ghost's normal mode while running maze
#define EATEN  4 // Ghost has been eaten and is moving to home
#define WAIT   5 // Ghost is waiting at home

byte ghost_x[4];
byte ghost_y[4];
byte ghost_direction[4];
byte ghost_planned_direction[4];
float ghost_speed[4];
byte ghost_target_x[4];
byte ghost_target_y[4];
byte ghost_scatter_target_x[4];
byte ghost_scatter_target_y[4];
byte ghost_status[4];
unsigned long ghost_update_counter[4];
byte ghost_mode;
byte previous_ghost_mode = -1;
boolean ghost_reverse_pending[4] = {false, false, false, false};

#define BASE_SPEED 10 // How many tiles per second is the base speed of Pac-Man

unsigned long previousMillis = 0;

unsigned long TPS_update_counter = 0; // Used to update TPS counter (Ticks per second)
unsigned long TPS = 0; // How many game ticks there are per second (number of times per second through the main loop)

#define FIELD_BLOCKED   0
#define FIELD_EMPTY     1
#define FIELD_DOT       2
#define FIELD_ENERGIZER 3

const char game_field_init[36][28] PROGMEM = { // Y, X - number of lines, number of columns
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
  {0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0},
  {0, 3, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0},
  {0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0},
  {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
  {0, 2, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 2, 0},
  {0, 2, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 2, 0},
  {0, 2, 2, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 0},
  {0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0},
  {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
  {0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0},
  {0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0},
  {0, 3, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 2, 3, 0},
  {0, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0},
  {0, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0},
  {0, 2, 2, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 0},
  {0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0},
  {0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0},
  {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

char game_field[28][36]; // X, Y

const char game_background[36][55] PROGMEM = {
  R"(      1 U P       H I G H   S C O R E                  )",
  R"(           0                    0                      )",
  R"(                                                       )",
  R"(/=====================================================\)",
  R"(| . . . . . . . . . . . . \ / . . . . . . . . . . . . |)",
  R"(| . /-----\ . /-------\ . | | . /-------\ . /-----\ . |)",
  R"(| O |     | . |       | . | | . |       | . |     | O |)",
  R"(| . \-----/ . \-------/ . \ / . \-------/ . \-----/ . |)",
  R"(| . . . . . . . . . . . . . . . . . . . . . . . . . . |)",
  R"(| . /-----\ . /-\ . /-------------\ . /-\ . /-----\ . |)",
  R"(| . \-----/ . | | . \-----\ /-----/ . | | . \-----/ . |)",
  R"(| . . . . . . | | . . . . | | . . . . | | . . . . . . |)",
  R"(\=========\ . | \-----\   | |   /-----/ | . /=========/)",
  R"(          | . | /-----/   \-/   \-----\ | . |          )",
  R"(          | . | |                     | | . |          )",
  R"(          | . | |   /=====---=====\   | | . |          )",
  R"(==========/ . \-/   |             |   \-/ . \==========)",
  R"(            .       |             |       .            )",
  R"(==========\ . /-\   |             |   /-\ . /==========)",
  R"(          | . | |   \=============/   | | . |          )",
  R"(          | . | |                     | | . |          )",
  R"(          | . | |   /-------------\   | | . |          )",
  R"(/=========/ . \-/   \-----\ /-----/   \-/ . \=========\)",
  R"(| . . . . . . . . . . . . | | . . . . . . . . . . . . |)",
  R"(| . /-----\ . /-------\ . | | . /-------\ . /-----\ . |)",
  R"(| . \---\ | . \-------/ . \-/ . \-------/ . | /---/ . |)",
  R"(| O . . | | . . . . . . .     . . . . . . . | | . . O |)",
  R"(|---\ . | | . /-\ . /-------------\ . /-\ . | | . /---|)",
  R"(|---/ . \-/ . | | . \-----\ /-----/ . | | . \-/ . \---|)",
  R"(| . . . . . . | | . . . . | | . . . . | | . . . . . . |)",
  R"(| . /---------/ \-----\ . | | . /-----/ \---------\ . |)",
  R"(| . \-----------------/ . \-/ . \-----------------/ . |)",
  R"(| . . . . . . . . . . . . . . . . . . . . . . . . . . |)",
  R"(\=====================================================/)",
  R"(TPS:                                                   )",
  R"(                                                       )"
};

void draw_background() {
  for (int y = 0; y < 36; y++) {
    for (int x = 0; x < 55; x++) {
      char value = pgm_read_byte(&game_background[y][x]);
      Serial.print(value);
    }
    Serial.println("");
  }
}

void init_playfield() {
  // Copies initial playfield from PGMSPACE to RAM
  for (int y = 0; y < 36; y++) {
    for (int x = 0; x < 28; x++ ) {
      game_field[x][y] = pgm_read_byte(&game_field_init[y][x]);
    }
  }

  // Set initial position of pacman
  pacman_x = 13;
  pacman_y = 26;
}

void get_ready() {
  draw_set_pos2(18, 20);
  Serial.print("G E T   R E A D Y !");
  delay(2000);
  draw_set_pos2(18, 20);
  Serial.print("                   ");
}

void init_ghosts() {

  ghost_mode = SCATTER;

  // Red Ghost
  ghost_x[BLINKY] = 14;
  ghost_y[BLINKY] = 14;
  ghost_scatter_target_x[BLINKY] = 25; // Third block from top right
  ghost_scatter_target_y[BLINKY] = 0;
  ghost_target_x[BLINKY] = ghost_scatter_target_x[BLINKY];
  ghost_target_y[BLINKY] = ghost_scatter_target_y[BLINKY];
  ghost_direction[BLINKY] = DIR_LEFT;
  ghost_planned_direction[BLINKY] = DIR_LEFT;
  ghost_speed[BLINKY] = 0.75;
  ghost_update_counter[BLINKY] = 0;
  ghost_status[BLINKY] = ACTIVE;

  // Pink Ghost
  ghost_x[PINKY] = 14;
  ghost_y[PINKY] = 14;
  ghost_scatter_target_x[PINKY] = 2; // Third block from top left
  ghost_scatter_target_y[PINKY] = 0;
  ghost_target_x[PINKY] = ghost_scatter_target_x[PINKY];
  ghost_target_y[PINKY] = ghost_scatter_target_y[PINKY];
  ghost_direction[PINKY] = DIR_LEFT;
  ghost_planned_direction[PINKY] = DIR_LEFT;
  ghost_speed[PINKY] = 0.75;
  ghost_update_counter[PINKY] = 0;
  ghost_status[PINKY] = ACTIVE;

  // Blue Ghost
  ghost_x[INKY] = 14;
  ghost_y[INKY] = 14;
  ghost_scatter_target_x[INKY] = 27; // Bottom right
  ghost_scatter_target_y[INKY] = 35;
  ghost_target_x[INKY] = ghost_scatter_target_x[INKY];
  ghost_target_y[INKY] = ghost_scatter_target_y[INKY];
  ghost_direction[INKY] = DIR_LEFT;
  ghost_planned_direction[INKY] = DIR_LEFT;
  ghost_speed[INKY] = 0.75;
  ghost_update_counter[INKY] = 0;
  ghost_status[INKY] = ACTIVE;

  // Yellow Ghost
  ghost_x[CLYDE] = 14;
  ghost_y[CLYDE] = 14;
  ghost_scatter_target_x[CLYDE] = 0; // Bottom left
  ghost_scatter_target_y[CLYDE] = 35;
  ghost_target_x[CLYDE] = ghost_scatter_target_x[CLYDE];
  ghost_target_y[CLYDE] = ghost_scatter_target_y[CLYDE];
  ghost_direction[CLYDE] = DIR_LEFT;
  ghost_planned_direction[CLYDE] = DIR_LEFT;
  ghost_speed[CLYDE] = 0.75;
  ghost_update_counter[CLYDE] = 0;
  ghost_status[CLYDE] = ACTIVE;

}

void draw_clear_screen() {
  Serial.print("\x1B[2J");
}

void draw_score() {
  if (player_score != player_score_previous) {
    draw_set_pos2(6, 1);
    Serial.print(player_score);
    player_score_previous = player_score;
  }
}

void draw_set_pos(byte x, byte y) {
  Serial.print("\x1B[");
  Serial.print(y + 1);
  Serial.print(";");
  Serial.print(x * 2 + 1);
  Serial.print("H");
}

void draw_set_pos2(byte x, byte y) {
  Serial.print("\x1B[");
  Serial.print(y + 1);
  Serial.print(";");
  Serial.print(x + 1);
  Serial.print("H");
}

void setup() {
  Serial.begin(9600);
  init_playfield();
  init_ghosts();
  draw_clear_screen();
  Serial.print("\x1B[?25l"); // Hide cursor
  Serial.print("\x1B[1;1H"); // Home cursor
  draw_background();
  get_ready();
  previousMillis = millis();
}

boolean check_valid_move(byte x, byte y) {
  // Checks if the tile is a valid spot to move to
  if (game_field[x][y] == FIELD_BLOCKED) {
    return false;
  }
  return true;
}

float calc_distance(byte x1, byte y1, byte x2, byte y2) {
  float len_x = abs(x1 - x2);
  float len_y = abs(y1 - y2);
  return sqrt(sq(len_x) + sq(len_y));
}

void reverse_ghosts(void) {
  for (int i = 0; i < 4; i++) {
    ghost_reverse_pending[i] = true;
  }
}


void erase_ghost(byte ghost_number) {

  // Erase ghost at ghost_x[ghost_number], ghost_y[ghost_number]
  draw_set_pos(ghost_x[ghost_number], ghost_y[ghost_number]);
  switch (game_field[ghost_x[ghost_number]][ghost_y[ghost_number]]) {
    case FIELD_EMPTY:
      Serial.print(" ");
      break;
    case FIELD_DOT:
      Serial.print(".");
      break;
    case FIELD_ENERGIZER:
      Serial.print("O");
      break;
    default:
      Serial.print("X");
      break;
  }
  if (ghost_x[ghost_number] == pacman_x && ghost_y[ghost_number] == pacman_y) {
    draw_pacman();
  }
}

void move_ghost(byte ghost_number) {
  byte next_x;
  byte next_y;

  erase_ghost(ghost_number);

  // Move the ghost in the direction it is currently facing
  switch (ghost_direction[ghost_number]) {
    case DIR_UP :
      ghost_y[ghost_number]--;
      break;
    case DIR_LEFT :
      ghost_x[ghost_number]--;
      break;
    case DIR_DOWN :
      ghost_y[ghost_number]++;
      break;
    case DIR_RIGHT :
      ghost_x[ghost_number]++;
      break;
  }

  // Draw ghost at ghost_x[ghost_number], ghost_y[ghost_number]

  draw_set_pos(ghost_x[ghost_number], ghost_y[ghost_number]);

  if (ghost_status[ghost_number] == EATEN) {
    // Draw eaten ghosts differently
    Serial.print('"');
  }
  else if (ghost_mode == FRIGHT) {
    // Draw frightened ghosts differently
    Serial.print(char(ghost_number + 65));
  } else {
    Serial.print(ghost_number);
  }

  check_collision_ghost(ghost_number);

  /*
     draw_set_pos2(1, 36);
    switch (ghost_direction[ghost_number]) {
     case DIR_UP :
       Serial.print("UP   |");
       break;
     case DIR_LEFT :
       Serial.print("LEFT |");
       break;
     case DIR_DOWN :
       Serial.print("DOWN |");
       break;
     case DIR_RIGHT :
       Serial.print("RIGHT|");
       break;
    } */ // Debug

  // Now change direction to what was planned one move earlier, unless there is a direction reverse pending
  if (ghost_reverse_pending[ghost_number]) {
    // Move back to where we came from
    switch (ghost_direction[ghost_number]) {
      case DIR_UP: ghost_direction[ghost_number] = DIR_DOWN; break;
      case DIR_LEFT: ghost_direction[ghost_number] = DIR_RIGHT; break;
      case DIR_DOWN: ghost_direction[ghost_number] = DIR_UP; break;
      case DIR_RIGHT: ghost_direction[ghost_number] = DIR_LEFT; break;
    }
    ghost_reverse_pending[ghost_number] = false;
  } else {
    ghost_direction[ghost_number] = ghost_planned_direction[ghost_number];
  }

  draw_set_pos2(1 + ghost_number * 3, 36);
  Serial.print(ghost_direction[ghost_number]);

  /*
    switch (ghost_direction[ghost_number]) {
    case DIR_UP :
      Serial.print("UP    ");
      break;
    case DIR_LEFT :
      Serial.print("LEFT  ");
      break;
    case DIR_DOWN :
      Serial.print("DOWN  ");
      break;
    case DIR_RIGHT :
      Serial.print("RIGHT ");
      break;
    } */ // Debug

  // Plan one tile ahead
  // Check which direction is the best to turn to
  float weight_up = 0;
  float weight_left = 0;
  float weight_down = 0;
  float weight_right = 0;

  next_x = ghost_x[ghost_number];
  next_y = ghost_y[ghost_number];
  // Weigh down the reverse direction
  switch (ghost_direction[ghost_number]) {
    case DIR_UP :
      next_y--;
      weight_down += 500;
      break;
    case DIR_LEFT :
      next_x--;
      weight_right += 500;
      break;
    case DIR_DOWN :
      next_y++;
      weight_up += 500;
      break;
    case DIR_RIGHT :
      next_x++;
      weight_left += 500;
      break;
  }
  //Serial.print(next_x); // Debug
  //Serial.print(","); // Debug
  //Serial.print(next_y); // Debug
  //Serial.print("|"); // Debug

  // Weigh blocked moves with a weight of 1000
  weight_up += !check_valid_move(next_x, next_y - 1) * 1000;
  weight_left += !check_valid_move(next_x - 1, next_y) * 1000;
  weight_down += !check_valid_move(next_x, next_y + 1) * 1000;
  weight_right += !check_valid_move(next_x + 1, next_y) * 1000;

  int ghost_behaviour = ghost_mode; // Default to global ghost mode
  if (ghost_status[ghost_number] == EATEN || ghost_status[ghost_number] == WAIT) {
    ghost_behaviour = ghost_status[ghost_number];
  }

  // Set the ghost's target based on ghost type and current behaviour
  switch (ghost_behaviour) {
    case EATEN:
      // Ghost is heading home
      ghost_target_x[ghost_number] = 13;
      ghost_target_y[ghost_number] = 14;
      break;
    case WAIT:
      ghost_target_x[ghost_number] = 13;
      ghost_target_y[ghost_number] = 14;
      break;
    case SCATTER:
      // In scatter mode, all ghosts does the same thing, they try to get to their 'home' positions
      // exception for blinky.. who becomes Elroy
      ghost_target_x[ghost_number] = ghost_scatter_target_x[ghost_number];
      ghost_target_y[ghost_number] = ghost_scatter_target_y[ghost_number];
      break;
    case CHASE:
      // In chase mode each ghost has a different dynamic target
      switch (ghost_number) {
        case BLINKY:
          // Blinky goes straight for the player
          ghost_target_x[ghost_number] = pacman_x;
          ghost_target_y[ghost_number] = pacman_y;
          break;
        case PINKY:
          // Pinky targets the tile 4 spaces in front of Pacman
          ghost_target_x[ghost_number] = pacman_x;
          ghost_target_y[ghost_number] = pacman_y;
          switch (pacman_direction) {
            case DIR_LEFT:
              ghost_target_x[ghost_number] = pacman_x - 4;
              break;
            case DIR_RIGHT:
              ghost_target_x[ghost_number] = pacman_x + 4;
              break;
            case DIR_DOWN:
              ghost_target_y[ghost_number] = pacman_y + 4;
              break;
            case DIR_UP:
              ghost_target_y[ghost_number] = pacman_y - 4;
              ghost_target_x[ghost_number] = pacman_x - 4; // Replicating bug in original game
              break;
          }
          break;
        case INKY:
          int x_offset;
          int y_offset;
          // First get coordinates of space 2 tiles in front of pacman
          byte pac_t_x;
          byte pac_t_y;
          pac_t_x = pacman_x;
          pac_t_y = pacman_y;
          switch (pacman_direction) {
            case DIR_LEFT:
              pac_t_x = pacman_x - 2;
              break;
            case DIR_RIGHT:
              pac_t_x = pacman_x + 2;
              break;
            case DIR_DOWN:
              pac_t_y = pacman_y + 2;
              break;
            case DIR_UP:
              pac_t_y = pacman_y - 2;
              pac_t_x = pacman_x - 2; // Replicating bug in original game
              break;
          }
          // Get offset between Blinky and target spot
          x_offset = ghost_x[BLINKY] - pac_t_x;
          y_offset = ghost_y[BLINKY] - pac_t_y;

          // Add the offset again to get target
          ghost_target_x[ghost_number] = pac_t_x + x_offset;
          ghost_target_y[ghost_number] = pac_t_y + x_offset;

          break;
        case CLYDE:
          // Clyde sets his target to Pacman if he's farther than 8 tiles away, otherwise he targets his scatter tile
          if (calc_distance(ghost_x[ghost_number], ghost_y[ghost_number], pacman_x, pacman_y) > 8) {
            ghost_target_x[ghost_number] = pacman_x;
            ghost_target_y[ghost_number] = pacman_y;
          } else {
            ghost_target_x[ghost_number] = ghost_scatter_target_x[ghost_number];
            ghost_target_y[ghost_number] = ghost_scatter_target_y[ghost_number];
          }
          break;
      }
      break;
  }

  // Check limits for target

  if (ghost_target_x[ghost_number] < 0) ghost_target_x[ghost_number] = 0;
  if (ghost_target_x[ghost_number] > 27) ghost_target_x[ghost_number] = 27;
  if (ghost_target_y[ghost_number] < 0) ghost_target_x[ghost_number] = 0;
  if (ghost_target_y[ghost_number] > 35) ghost_target_x[ghost_number] = 35;

  // Add vector weights based on target
  weight_up    += calc_distance(ghost_target_x[ghost_number], ghost_target_y[ghost_number], next_x, next_y - 1);
  weight_left  += calc_distance(ghost_target_x[ghost_number], ghost_target_y[ghost_number], next_x - 1, next_y);
  weight_down  += calc_distance(ghost_target_x[ghost_number], ghost_target_y[ghost_number], next_x, next_y + 1);
  weight_right += calc_distance(ghost_target_x[ghost_number], ghost_target_y[ghost_number], next_x + 1, next_y);

  ghost_planned_direction[ghost_number] = DIR_UP;
  float current_weight = weight_up;

  if (weight_left < current_weight) {
    ghost_planned_direction[ghost_number] = DIR_LEFT;
    current_weight = weight_left;
  }
  if (weight_down < current_weight) {
    ghost_planned_direction[ghost_number] = DIR_DOWN;
    current_weight = weight_down;
  }
  if (weight_right < current_weight) {
    ghost_planned_direction[ghost_number] = DIR_RIGHT;
    current_weight = weight_right;
  }

  /*
    switch (ghost_planned_direction[ghost_number]) {
      case DIR_UP :
        Serial.print("UP    |");
        break;
      case DIR_LEFT :
        Serial.print("LEFT  |");
        break;
      case DIR_DOWN :
        Serial.print("DOWN  |");
        break;
      case DIR_RIGHT :
        Serial.print("RIGHT |");
        break;
    }
    Serial.print("[ ");
    Serial.print(weight_up);
    Serial.print(",");
    Serial.print(weight_left);
    Serial.print(",");
    Serial.print(weight_down);
    Serial.print(",");
    Serial.print(weight_right);
    Serial.print(" ]");
  */ // Debug

}

void check_collision_ghost(int ghost_number) {
  if (ghost_x[ghost_number] == pacman_x && ghost_y[ghost_number] == pacman_y) {
    // There was a collision
    if (ghost_mode == FRIGHT) {
      // Player ate ghost
      ghost_status[ghost_number] = EATEN;
      ghost_eat_counter++;
      player_score += 200 * ghost_eat_counter;
    } else if (ghost_status[ghost_number] != ACTIVE) {
      // Do nothing here, ghost has is eaten.
    } else {
      // Player died
      player_status = PLAYER_DEAD;
    }
  }
}

void check_collision_all () {
  for (int i = 0; i < 4; i++) {
    check_collision_ghost(i);
  }
}

void draw_pacman() {
  draw_set_pos(pacman_x, pacman_y);
  Serial.print("@");
}

void loop() {

  unsigned long currentMillis;
  unsigned long elapsedMillis;

  currentMillis = millis();
  elapsedMillis = currentMillis - previousMillis;
  previousMillis = currentMillis;


  // Read keyboard input
  while (Serial.available()) { // Read in all the keys, keep the latest one
    switch (Serial.read()) {
      case 'a' :
        pacman_planned_direction = DIR_LEFT;
        break;
      case 'd' :
        pacman_planned_direction = DIR_RIGHT;
        break;
      case 'w' :
        pacman_planned_direction = DIR_UP;
        break;
      case 's' :
        pacman_planned_direction = DIR_DOWN;
        break;
      case ' ' :
        reverse_ghosts(); // Debugging Ghost AI
        break;
    }
  }

  // Increment time since last pacman move
  pacman_update_counter += elapsedMillis;
  // Check if Pacman can move one tile
  float pacman_move_time = 1000.0 / (BASE_SPEED * pacman_speed);
  if (pacman_update_counter >= pacman_move_time) {

    // Yes Pacman can move based on his current speed
    pacman_update_counter -= pacman_move_time; // Subtract one tile's movement time

    byte new_pacman_x = pacman_x;
    byte new_pacman_y = pacman_y;

    // Check if we can switch to the requested (pending direction right now)

    switch (pacman_planned_direction) {
      case DIR_RIGHT :
        if (check_valid_move(pacman_x + 1, pacman_y)) {
          pacman_direction = pacman_planned_direction;
        }
        break;
      case DIR_LEFT :
        if (check_valid_move(pacman_x - 1, pacman_y)) {
          pacman_direction = pacman_planned_direction;
        }
        break;
      case DIR_UP :
        if (check_valid_move(pacman_x, pacman_y - 1)) {
          pacman_direction = pacman_planned_direction;
        }
        break;
      case DIR_DOWN :
        if (check_valid_move(pacman_x, pacman_y + 1)) {
          pacman_direction = pacman_planned_direction;
        }
        break;
    }

    // Check if we can continue moving in the current direction
    switch (pacman_direction) {
      case DIR_RIGHT :
        if (check_valid_move(pacman_x + 1, pacman_y)) {
          new_pacman_x = pacman_x + 1;
        }
        break;
      case DIR_LEFT :
        if (check_valid_move(pacman_x - 1, pacman_y)) {
          new_pacman_x = pacman_x - 1;
        }
        break;
      case DIR_UP :
        if (check_valid_move(pacman_x, pacman_y - 1)) {
          new_pacman_y = pacman_y - 1;
        }
        break;
      case DIR_DOWN :
        if (check_valid_move(pacman_x, pacman_y + 1)) {
          new_pacman_y = pacman_y + 1;
        }
        break;
    }
    if (new_pacman_x != pacman_x || new_pacman_y != pacman_y) {
      // Erase from old position
      draw_set_pos(pacman_x, pacman_y);
      Serial.print(" ");


      pacman_x = new_pacman_x;
      pacman_y = new_pacman_y;
      draw_pacman();

      // Handle various actions based on what is at the new location
      switch (game_field[pacman_x][pacman_y]) {
        case FIELD_EMPTY :
          if (ghost_mode == FRIGHT) {
            pacman_speed = 0.9;
          } else {
            pacman_speed = 0.8;
          }
          break;
        case FIELD_DOT :
          // Move slower while eating dots
          if (ghost_mode == FRIGHT) {
            pacman_speed = 0.79;
          } else {
            pacman_speed = 0.71;
          }
          game_field[pacman_x][pacman_y] = FIELD_EMPTY; // Eat the dot
          player_score += 10;
          dots_eaten++;
          break;
        case FIELD_ENERGIZER :
          //pacman_speed = 0.71;
          game_field[pacman_x][pacman_y] = FIELD_EMPTY; // Eat the energizer
          player_score += 50;
          fright_timer = 6000;
          ghost_eat_counter = 0;
          dots_eaten++;
          break;
      }
      check_collision_all(); // Check collision for all ghosts
    }
  }

  // Handle fright timer
  if (fright_timer > elapsedMillis) {
    fright_timer -= elapsedMillis;
  } else {
    fright_timer = 0;
  }

  // Set ghost's mode
  if (fright_timer > 0) {
    ghost_mode = FRIGHT;
  } else {
    scatter_chase_timer += elapsedMillis;
    if (scatter_chase_timer < 7000) {
      ghost_mode = SCATTER;
    } else if (scatter_chase_timer < 27000) {
      ghost_mode = CHASE;
    } else if (scatter_chase_timer < 34000) {
      ghost_mode = SCATTER;
    } else if (scatter_chase_timer < 54000) {
      ghost_mode = CHASE;
    } else if (scatter_chase_timer < 59000) {
      ghost_mode = SCATTER;
    } else if (scatter_chase_timer < 79000) {
      ghost_mode = CHASE;
    } else if (scatter_chase_timer < 84000) {
      ghost_mode = SCATTER;
    } else {
      ghost_mode = CHASE;
    }
  }

  if (ghost_mode != previous_ghost_mode) {
    // Ghost mode changed
    draw_set_pos2(20, 34);
    if (previous_ghost_mode != FRIGHT) {
      reverse_ghosts(); // Reverse direction when mode changes, unless changing out of fright
    }
    // Set ghost speed
    for (int i = 0; i < 4; i++) {
      if (ghost_mode == FRIGHT) {
        ghost_speed[i] = 0.5;
      } else {
        ghost_speed[i] = 0.75;
      }
    }
    switch (ghost_mode) {
      case CHASE   : Serial.print("CHASE  "); break;
      case SCATTER : Serial.print("SCATTER"); break;
      case FRIGHT  : Serial.print("FRIGHT "); break;
    }
    previous_ghost_mode = ghost_mode;
  }

  // Move the ghosts
  for (int i = 0; i < 4; i++) {
    ghost_update_counter[i] += elapsedMillis;
    float ghost_move_time = 1000.0 / (BASE_SPEED * ghost_speed[i]);
    if (ghost_update_counter[i] >= ghost_move_time) {
      ghost_update_counter[i] -= ghost_move_time;
      move_ghost(i);
    }
  }

  draw_score();

  // Handle death
  if (player_status == PLAYER_DEAD) {
    // Erase ghosts
    for (int i = 0; i < 4; i++) {
      erase_ghost(i);
    }
    draw_set_pos(pacman_x, pacman_y);
    Serial.print(" ");
    delay(100);
    draw_set_pos(pacman_x, pacman_y);
    Serial.print("X");
    delay(100);
    draw_set_pos(pacman_x, pacman_y);
    Serial.print(" ");
    delay(100);
    draw_set_pos(pacman_x, pacman_y);
    Serial.print("+");
    delay(100);
    draw_set_pos(pacman_x, pacman_y);
    Serial.print(" ");
    delay(100);
    draw_set_pos(pacman_x, pacman_y);
    Serial.print("-");
    delay(100);
    draw_set_pos(pacman_x, pacman_y);
    Serial.print(" ");
    pacman_x = 13;
    pacman_y = 26;
    pacman_direction = DIR_LEFT;
    pacman_planned_direction = DIR_LEFT;
    player_status = PLAYER_ALIVE;
    draw_set_pos(pacman_x, pacman_y);
    Serial.print("@");
    delay(100);
    draw_set_pos(pacman_x, pacman_y);
    Serial.print(" ");
    delay(100);
    draw_set_pos(pacman_x, pacman_y);
    Serial.print("@");
    delay(250);
    init_ghosts();
    get_ready();
    scatter_chase_timer = 0;
    currentMillis = millis();
    previousMillis = currentMillis;
  }

  TPS_update_counter += elapsedMillis;
  TPS++;
  if (TPS_update_counter >= 1000) {
    TPS_update_counter -= 1000;

    if (TPS_update_counter >= 1000) {
      TPS = -1;
    }
    draw_set_pos2(4, 34);
    Serial.print("[");
    Serial.print(TPS);
    Serial.print("]");
    draw_set_pos2(20, 35);
    Serial.print("[");
    Serial.print(scatter_chase_timer / 1000);
    Serial.print("]     ");
    TPS = 0;
  }


  //draw_set_pos2(11, 34);
  //Serial.print(pacman_x, 1);
  //Serial.print(" ");
  //draw_set_pos2(18, 34);
  //Serial.print(pacman_y, 1);
  //Serial.print(" ");
  //draw_set_pos2(25, 34);
  //Serial.print(pacman_direction);
  //draw_set_pos2(29, 34);
  //Serial.print(pacman_planned_direction);


  Serial.flush();
  delay(10);
  //draw_clear_screen();
}
