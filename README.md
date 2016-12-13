# arduino-serial-pacman
Pacman running on an Arduino using serial VT100 as a display

I tried to replicate the gameplay of the original arcade game, including:
  - Grid layout
  - Ghost AI logic (including the original bugs with Pinky and Inky
  - Ghost pathing mode switch between Chase and Scatter and timing
  - Ghost and Pacman speed
  - Can press the button before reaching an intersection to pre-empt turning
  
This is still a work in progress, and right now at least the following major things are not working:
- Eating Energizers does nothing, so you can't eat the ghosts and they don't go into frightened mode
- Eating all the dots does not advance the level, so you can't win the game
- Dying does not cause you to lose a life, so you can't lose the game
- The tunnel does not transport you to the other side

Upload the sketch to an Arduino (tested with UNO, but should work on other Arduino's too.
The current version uses about 11KB of program space (out of a max of 32KB on the UNO) and 1.4KB of RAM (out of a max of 2KB on the UNO)

Use a terminal emulator such as PuTTY to connect at 9600 BAUD
Use keys 'wasd' to move around

# "Screenshot"

          1 U P       H I G H   S C O R E
               0                    0
    
    /=====================================================\
    | . . . . . . . . . . . . \ / . . . . . . . . . . . . |
    | . /-----\ . /-------\ . | | . /-------\ . /-----\ . |
    | O |     | . |       | . | | . |       | . |     | O |
    | . \-----/ . \-------/ . \ / . \-------/ . \-----/ . |
    | . . . . . . . . . . . . . . . . . . . . . . . . . . |
    | . /-----\ . /-\ . /-------------\ . /-\ . /-----\ . |
    | . \-----/ . | | . \-----\ /-----/ . | | . \-----/ . |
    | . . . . . . | | . . . . | | . . . . | | . . . . . . |
    \=========\ . | \-----\   | |   /-----/ | . /=========/
              | . | /-----/   \-/   \-----\ | . |
              | . | |         0           | | . |
              | . | |   /=====---=====\   | | . |
    ==========/ . \-/   |             |   \-/ . \==========
                .       |             |       .
    ==========\ . /-\   |             |   /-\ . /==========
              | . | |   \=============/   | | . |
              | . | | G E T   R E A D Y ! | | . |
              | . | |   /-------------\   | | . |
    /=========/ . \-/   \-----\ /-----/   \-/ . \=========\
    | . . . . . . . . . . . . | | . . . . . . . . . . . . |
    | . /-----\ . /-------\ . | | . /-------\ . /-----\ . |
    | . \---\ | . \-------/ . \-/ . \-------/ . | /---/ . |
    | O . . | | . . . . . . .  @  . . . . . . . | | . . O |
    |---\ . | | . /-\ . /-------------\ . /-\ . | | . /---|
    |---/ . \-/ . | | . \-----\ /-----/ . | | . \-/ . \---|
    | . . . . . . | | . . . . | | . . . . | | . . . . . . |
    | . /---------/ \-----\ . | | . /-----/ \---------\ . |
    | . \-----------------/ . \-/ . \-----------------/ . |
    | . . . . . . . . . . . . . . . . . . . . . . . . . . |
    \=====================================================/






