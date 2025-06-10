// Arduino PACMAN
// https://github.com/andremiller/arduino-serial-pacman
// 80x24 version

#include <avr/pgmspace.h>
#include <EEPROM.h>

#define DIR_RIGHT 0
#define DIR_DOWN  1
#define DIR_LEFT  2
#define DIR_UP    3

// EEPROM address for high score storage
#define EEPROM_HIGH_SCORE_ADDR 0

// Character positions
byte pacman_x = 19;
byte pacman_y = 17;
byte pacman_direction = DIR_LEFT;
byte pacman_planned_direction = DIR_LEFT;
float pacman_speed = 0.8; // Percentage of base speed
int ghost_eat_counter = 0; // Number of ghosts eaten per energizer
unsigned long pacman_update_counter = 0; // How much time has elapsed since the laste position update for pacman
long player_score = 0;
long player_score_previous = -1;
long player_high_score = 0;
long next_life_score = 10000; // Next score threshold for free life
int level = 1;
int dots_eaten = 0;
int total_dots = 0;
#define PLAYER_ALIVE 0
#define PLAYER_DEAD 1
byte player_status = PLAYER_ALIVE;
int lives = 3;

unsigned long scatter_chase_timer = 0;
unsigned long fright_timer = 0;

// The cast
#define PACMAN  -1
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
unsigned long ghost_wait_timer[4];
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

char game_field[38][24]; // X, Y

const char game_background[24][76] PROGMEM = {
  R"(    S C O R E :                       H I G H :                             )",
  R"(    /=====================================================================\ )",
  R"(    | . . . . . . . . . . . . . . . . | | . . . . . . . . . . . . . . . . | )",
  R"(    | . /-------\ . /-------------\ . | | . /-------------\ . /-------\ . | )",
  R"(    | O \-------/ . \-------------/ . \-/ . \-------------/ . \-------/ O | )",
  R"(    | . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . | )",
  R"(    | . -=======- . /---\ . -=====================- . /---\ . -=======- . | )",
  R"(    | . . . . . . . |   | . . . . . . | | . . . . . . |   | . . . . . . . | )",
  R"(    \===========\ . |   |=========-   \-/   -=========|   | . /===========/ )",
  R"(                | . |   |                             |   | . |             )",
  R"(    ============/ . \---/   /=========   =========\   \---/ . \============ )",
  R"(                  .         |                     |         .               )",
  R"(    ============\ . /---\   \=====================/   /---\ . /============ )",
  R"(                | . |   |                             |   | . |             )",
  R"(    /===========/ . \---/   -=====================-   \---/ . \===========\ )",
  R"(    | . . . . . . . . . . . . . . . . | | . . . . . . . . . . . . . . . . | )",
  R"(    | . -=======\ . -=============- . \-/ . -=============- . /=======- . | )",
  R"(    | O . . . | | . . . . . . . . . .     . . . . . . . . . . | | . . . O | )",
  R"(    |=====- . \-/ . /---\ . -=====================- . /---\ . \-/ . -=====| )",
  R"(    | . . . . . . . |   | . . . . . . | | . . . . . . |   | . . . . . . . | )",
  R"(    | . -=========================- . \-/ . -=========================- . | )",
  R"(    | . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . | )",
  R"(    \=====================================================================/ )",
  R"(    L I V E S :                       L E V E L :                           )",
};


void draw_background() {
  for (int y = 0; y < 24; y++) {
    draw_set_pos(0, y);
    for (int x = 0; x < 76; x++) {
      char value = pgm_read_byte(&game_background[y][x]);
      Serial.print(value);
    }
    //Serial.println("");
  }
  draw_set_pos2(51, 24);
  Serial.print(level);
  draw_high_score();
}

void init_playfield() {
  // Build playfield from background pattern using character mapping
  total_dots = 0;
  dots_eaten = 0;
  for (int y = 0; y < 24; y++) {
    for (int x = 0; x < 38; x++) {
      char bg_char = pgm_read_byte(&game_background[y][x * 2]);
      switch(bg_char) {
        case ' ': game_field[x][y] = FIELD_EMPTY; break;
        case '.': game_field[x][y] = FIELD_DOT; total_dots+=1; break;
        case 'O': game_field[x][y] = FIELD_ENERGIZER; total_dots+=1; break;
        default:  game_field[x][y] = FIELD_BLOCKED; break;
      }
    }
  }

  total_dots-=1; // Hack to remove the one we counted in SC"O"RE

  // Set initial position of pacman
  pacman_x = 19;
  pacman_y = 17;
}

void get_ready() {
  draw_set_pos2(30, 13);
  Serial.print("G E T   R E A D Y !");
  delay(2000);
  draw_set_pos2(30, 13);
  Serial.print("                   ");
}

void press_to_start() {
  draw_set_pos2(29, 13);
  Serial.print("P R E S S   A   K E Y");
  // Wait for input - properly block until a key is pressed
  while (Serial.available() <= 0) {
    // Keep waiting until data is available
  }
  Serial.read(); // Read and discard the character
  draw_set_pos2(29, 13);
  Serial.print("                     ");
}

void load_high_score() {
  EEPROM.get(EEPROM_HIGH_SCORE_ADDR, player_high_score);
  // If EEPROM is uninitialized, it will contain 0xFFFFFFFF (-1 as signed long)
  // Reset to 0 if invalid
  if (player_high_score < 0) {
    player_high_score = 0;
  }
}

void save_high_score() {
  EEPROM.put(EEPROM_HIGH_SCORE_ADDR, player_high_score);
}

void game_over() {
  draw_set_pos2(30, 11);
  Serial.print("G A M E     O V E R");
  delay(2000);
}


void init_ghosts() {

  ghost_mode = SCATTER;

  // Red Ghost
  ghost_x[BLINKY] = 20;
  ghost_y[BLINKY] = 10;
  ghost_scatter_target_x[BLINKY] = 34; // Third block from top right
  ghost_scatter_target_y[BLINKY] = 0;
  ghost_target_x[BLINKY] = ghost_scatter_target_x[BLINKY];
  ghost_target_y[BLINKY] = ghost_scatter_target_y[BLINKY];
  ghost_direction[BLINKY] = DIR_LEFT;
  ghost_planned_direction[BLINKY] = DIR_LEFT;
  ghost_speed[BLINKY] = 0.75;
  ghost_update_counter[BLINKY] = 0;
  ghost_status[BLINKY] = ACTIVE;
  ghost_wait_timer[BLINKY] = 0;

  // Pink Ghost
  ghost_x[PINKY] = 20;
  ghost_y[PINKY] = 10;
  ghost_scatter_target_x[PINKY] = 2; // Third block from top left
  ghost_scatter_target_y[PINKY] = 0;
  ghost_target_x[PINKY] = ghost_scatter_target_x[PINKY];
  ghost_target_y[PINKY] = ghost_scatter_target_y[PINKY];
  ghost_direction[PINKY] = DIR_LEFT;
  ghost_planned_direction[PINKY] = DIR_LEFT;
  ghost_speed[PINKY] = 0.75;
  ghost_update_counter[PINKY] = 0;
  ghost_status[PINKY] = ACTIVE;
  ghost_wait_timer[PINKY] = 0;

  // Blue Ghost
  ghost_x[INKY] = 20;
  ghost_y[INKY] = 10;
  ghost_scatter_target_x[INKY] = 37; // Bottom right
  ghost_scatter_target_y[INKY] = 23;
  ghost_target_x[INKY] = ghost_scatter_target_x[INKY];
  ghost_target_y[INKY] = ghost_scatter_target_y[INKY];
  ghost_direction[INKY] = DIR_LEFT;
  ghost_planned_direction[INKY] = DIR_LEFT;
  ghost_speed[INKY] = 0.75;
  ghost_update_counter[INKY] = 0;
  ghost_status[INKY] = ACTIVE;
  ghost_wait_timer[INKY] = 0;

  // Yellow Ghost
  ghost_x[CLYDE] = 20;
  ghost_y[CLYDE] = 10;
  ghost_scatter_target_x[CLYDE] = 0; // Bottom left
  ghost_scatter_target_y[CLYDE] = 23;
  ghost_target_x[CLYDE] = ghost_scatter_target_x[CLYDE];
  ghost_target_y[CLYDE] = ghost_scatter_target_y[CLYDE];
  ghost_direction[CLYDE] = DIR_LEFT;
  ghost_planned_direction[CLYDE] = DIR_LEFT;
  ghost_speed[CLYDE] = 0.75;
  ghost_update_counter[CLYDE] = 0;
  ghost_status[CLYDE] = ACTIVE;
  ghost_wait_timer[CLYDE] = 0;

}

void draw_clear_screen() {
  Serial.print("\x1B[2J");
}

void draw_high_score() {
  draw_set_pos2(48, 0);
  Serial.print(player_high_score);
}


void draw_score() {
  if (player_score != player_score_previous) {
    draw_set_pos2(16, 0);
    Serial.print(player_score);
    player_score_previous = player_score;
    if (player_score > player_high_score) {
      player_high_score = player_score;
      draw_high_score();
    }
    
    // Check for free life every 10,000 points
    if (player_score >= next_life_score) {
      lives++;
      next_life_score += 10000;
      draw_lives();
    }
  }
}

void draw_lives() {
  draw_set_pos2(16, 23);
  Serial.print(lives);
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
  load_high_score();
  init_playfield();
  init_ghosts();
  draw_clear_screen();
  Serial.print("\x1B[?25l"); // Hide cursor
  Serial.print("\x1B[1;1H"); // Home cursor
  draw_background();
  draw_lives();
  press_to_start();
  get_ready();
  previousMillis = millis();
}

boolean check_valid_move(byte x, byte y, int8_t who, int8_t direction) {
  // Checks if the tile is a valid spot to move to

  // Special case for house

  if (y==10 && (x == 19 || x == 20)) {
    if (who == PACMAN) {
      return false; // Pacman can't move into house
    }
    if (direction == DIR_LEFT || direction == DIR_RIGHT) {
      return false; // Can't move left or right in the door
    }
    if (ghost_status[who] == EATEN && direction == DIR_UP) {
      return false; // Eaten ghosts can't move out of house
    }
    if (ghost_status[who] != EATEN && direction == DIR_DOWN) {
      return false; // Normal ghosts can't move into house
    }
  }

  // Special case for portal, only PACMAN can pass through
  if (who != PACMAN && y==11 && (x == 8 || x == 31 )) {
    return false;
  }

  
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
    Serial.print("8");
    //Serial.print(char(ghost_number + 65));
  } else {
    //Serial.print(ghost_number);
    Serial.print("0");
  }

  check_collision_ghost(ghost_number);

  /*
     draw_set_pos2(1, 24);
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

  //draw_set_pos2(40 + ghost_number * 3, 23);
  //Serial.print(ghost_direction[ghost_number]);

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
  weight_up += !check_valid_move(next_x, next_y - 1, ghost_number, DIR_UP) * 1000;
  weight_left += !check_valid_move(next_x - 1, next_y, ghost_number, DIR_LEFT) * 1000;
  weight_down += !check_valid_move(next_x, next_y + 1, ghost_number, DIR_DOWN) * 1000;
  weight_right += !check_valid_move(next_x + 1, next_y, ghost_number, DIR_RIGHT) * 1000;

  int ghost_behaviour = ghost_mode; // Default to global ghost mode
  if (ghost_status[ghost_number] == EATEN || ghost_status[ghost_number] == WAIT) {
    ghost_behaviour = ghost_status[ghost_number];
  }

  // Set the ghost's target based on ghost type and current behaviour
  switch (ghost_behaviour) {
    case EATEN:
      // Ghost is heading home
      ghost_target_x[ghost_number] = 19;
      ghost_target_y[ghost_number] = 11;
      // Check if ghost is back home
      if (ghost_y[ghost_number]==11 && (ghost_x[ghost_number]>17 && ghost_x[ghost_number]<21)) {
        ghost_status[ghost_number]=WAIT;
        ghost_wait_timer[ghost_number] = 3000;
      }
      break;
    case WAIT:
      ghost_target_x[ghost_number] = 19;
      ghost_target_y[ghost_number] = 11;
      // Check if wait timer is over is done in main loop
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
  if (ghost_target_x[ghost_number] > 37) ghost_target_x[ghost_number] = 37;
  if (ghost_target_y[ghost_number] < 0) ghost_target_x[ghost_number] = 0;
  if (ghost_target_y[ghost_number] > 23) ghost_target_y[ghost_number] = 23;

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
    
    if (pacman_y == 11 && (pacman_x < 8 || pacman_x > 31) ) {
      // If we are in the portal tunnel, we can't change directions
    } else {
      switch (pacman_planned_direction) {
        case DIR_RIGHT :
          if (check_valid_move(pacman_x + 1, pacman_y, PACMAN, DIR_RIGHT)) {
            pacman_direction = pacman_planned_direction;
          }
          break;
        case DIR_LEFT :
          if (check_valid_move(pacman_x - 1, pacman_y, PACMAN, DIR_LEFT)) {
            pacman_direction = pacman_planned_direction;
          }
          break;
        case DIR_UP :
          if (check_valid_move(pacman_x, pacman_y - 1, PACMAN, DIR_UP)) {
            pacman_direction = pacman_planned_direction;
          }
          break;
        case DIR_DOWN :
          if (check_valid_move(pacman_x, pacman_y + 1, PACMAN, DIR_DOWN)) {
            pacman_direction = pacman_planned_direction;
          }
          break;
      }
    }

    // Check if we can continue moving in the current direction
    switch (pacman_direction) {
      case DIR_RIGHT :
        if (check_valid_move(pacman_x + 1, pacman_y, PACMAN, DIR_RIGHT)) {
          new_pacman_x = pacman_x + 1;
        }
        break;
      case DIR_LEFT :
        if (check_valid_move(pacman_x - 1, pacman_y, PACMAN, DIR_LEFT)) {
          new_pacman_x = pacman_x - 1;
        }
        break;
      case DIR_UP :
        if (check_valid_move(pacman_x, pacman_y - 1, PACMAN, DIR_UP)) {
          new_pacman_y = pacman_y - 1;
        }
        break;
      case DIR_DOWN :
        if (check_valid_move(pacman_x, pacman_y + 1, PACMAN, DIR_DOWN)) {
          new_pacman_y = pacman_y + 1;
        }
        break;
    }

    // Teleport check
    if (new_pacman_y == 11 && new_pacman_x < 3) {
      new_pacman_x = 37;
    }
    else if (new_pacman_y == 11 && new_pacman_x > 36) {
      new_pacman_x = 2;
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
    draw_set_pos2(30, 23);
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
    /*
    switch (ghost_mode) {
      case CHASE   : Serial.print("CHASE  "); break;
      case SCATTER : Serial.print("SCATTER"); break;
      case FRIGHT  : Serial.print("FRIGHT "); break;
    }
      */
    previous_ghost_mode = ghost_mode;
  }

  // Move the ghosts and update timers
  for (int i = 0; i < 4; i++) {
    // Handle wait timer
    if (ghost_wait_timer[i] > elapsedMillis) {
      ghost_wait_timer[i] -= elapsedMillis;
    } else {
      ghost_wait_timer[i] = 0;
      if (ghost_status[i] == WAIT) {
        ghost_status[i]=ACTIVE;
      }
    }
    // Move ghosts
    ghost_update_counter[i] += elapsedMillis;
    float ghost_move_time = 1000.0 / (BASE_SPEED * ghost_speed[i]);
    if (ghost_update_counter[i] >= ghost_move_time) {
      ghost_update_counter[i] -= ghost_move_time;
      move_ghost(i);
    }
  }

  draw_score();

  if (dots_eaten >= total_dots) {
    // Reset the board
    level+=1;
    init_playfield();
    init_ghosts();
    draw_clear_screen();
    draw_background();
    draw_lives();
    pacman_x = 19;
    pacman_y = 17;
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
    get_ready();
    scatter_chase_timer = 0;
    currentMillis = millis();
    previousMillis = currentMillis;
  }

  // Handle death
  if (player_status == PLAYER_DEAD) {
    // Subtract a life
    lives -=1;
    draw_lives();
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
    pacman_x = 19;
    pacman_y = 17;
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
    if (lives) {
      get_ready();
    }
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
    //draw_set_pos2(4, 23);
    //Serial.print("[");
    //Serial.print(TPS);
    //Serial.print("]");
    //draw_set_pos2(20, 23);
    //Serial.print("[");
    //Serial.print(scatter_chase_timer / 1000);
    //Serial.print("]     ");
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

  // Check for game over
  if (lives == 0) {
    save_high_score();
    level = 1;
    lives = 3;
    player_score = 0;
    next_life_score = 10000; // Reset life threshold
    game_over();
    press_to_start();
    init_playfield();
    init_ghosts();
    draw_clear_screen();
    Serial.print("\x1B[?25l"); // Hide cursor
    Serial.print("\x1B[1;1H"); // Home cursor
    draw_background();
    draw_lives();
    get_ready();

    previousMillis = millis();
  }
}
