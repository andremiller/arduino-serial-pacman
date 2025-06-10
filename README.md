# arduino-serial-pacman
Pacman running on an Arduino using serial VT100 as a display
Inspired by https://github.com/dadecoza/ArduinoTermTetris

I tried to replicate the gameplay of the original arcade game, including:
  - Grid layout
  - Ghost AI logic (including the original bugs with Pinky and Inky
  - Ghost pathing mode switch between Chase and Scatter and timing
  - Ghost and Pacman speed
  - Can press the button before reaching an intersection to pre-empt turning
  
Things that are now working:
- Ghosts that you eat can enter their home and respawn
- Eating all the dots increases the level count
- Dying causes you to lose a life and eventually game over
- The tunnel transports you to the other side
- High score is saved on EEPROM

Upload the sketch to an Arduino (tested with UNO, but should work on other Arduino's too.
On an UNO: Sketch uses 11494 bytes (35%) of program storage space. Maximum is 32256 bytes.
Global variables use 1374 bytes (67%) of dynamic memory, leaving 674 bytes for local variables. Maximum is 2048 bytes.

Use a terminal emulator such as PuTTY to connect at 9600 BAUD
Use keys 'wasd' to move around

# "Screenshot"

    S C O R E :                       H I G H : 25460
    /=====================================================================\
    | . . . . . . . . . . . . . . . . | | . . . . . . . . . . . . . . . . |
    | . /-------\ . /-------------\ . | | . /-------------\ . /-------\ . |
    | O \-------/ . \-------------/ . \-/ . \-------------/ . \-------/ O |
    | . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . |
    | . -=======- . /---\ . -=====================- . /---\ . -=======- . |
    | . . . . . . . |   | . . . . . . | | . . . . . . |   | . . . . . . . |
    \===========\ . |   |=========-   \-/   -=========|   | . /===========/
                | . |   |                             |   | . |
    ============/ . \---/   /=========   =========\   \---/ . \============
                  .         |                     |         .
    ============\ . /---\   \=====================/   /---\ . /============
                | . |   |    P R E S S   A   K E Y    |   | . |
    /===========/ . \---/   -=====================-   \---/ . \===========\
    | . . . . . . . . . . . . . . . . | | . . . . . . . . . . . . . . . . |
    | . -=======\ . -=============- . \-/ . -=============- . /=======- . |
    | O . . . | | . . . . . . . . . .     . . . . . . . . . . | | . . . O |
    |=====- . \-/ . /---\ . -=====================- . /---\ . \-/ . -=====|
    | . . . . . . . |   | . . . . . . | | . . . . . . |   | . . . . . . . |
    | . -=========================- . \-/ . -=========================- . |
    | . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . |
    \=====================================================================/
    L I V E S : 3                     L E V E L :  1






