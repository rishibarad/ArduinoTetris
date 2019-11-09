#include <gamma.h>
#include <RGBmatrixPanel.h>
#include <Adafruit_GFX.h>

/*
    TETRIS
    BY: TEAM ITACHI (Rishi Barad, Nilay Patel, Thejas Raghava, Maureen Wu)

    USING ARDUINO MEGA fOR THE ADDITIONAL PROGRAM STORAGE SPACE!!
    4 button setup.
*/

// define the wiring of the LED screen
const uint8_t CLK  = 11;
const uint8_t LAT = 10;
const uint8_t OE = 9;
const uint8_t A = A0;
const uint8_t B = A1;
const uint8_t C = A2;

// define the wiring of the inputs
const uint8_t BUTTON = A3;


// a global variable that represents the LED screen
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

// global variables
int boardWidth = 16;
int boardHeight = 18;
const int boardArea = 288;

unsigned char board[boardArea];


String tetromino[7];






// the following functions are for printing messages
void print_game_name();
//void print_board();
void print_number(int x, int y, int num);
void print_level(int level);
void print_lines(int lines);
void print_game_over();

class Color {
  public:
    int red;
    int green;
    int blue;
    Color() {
      red = 0;
      green = 0;
      blue = 0;
    }
    Color(int r, int g, int b) {
      red = r;
      green = g;
      blue = b;
    }
    uint16_t to_333() const {
      return matrix.Color333(red, green, blue);
    }
};

const Color BLACK(0, 0, 0);
const Color RED(4, 0, 0);
const Color ORANGE(6, 1, 0);
const Color YELLOW(4, 4, 0);
const Color GREEN(0, 4, 0);
const Color BLUE(0, 0, 4);
const Color PURPLE(1, 0, 2);
const Color WHITE(4, 4, 4);
const Color LIME(2, 4, 0);
const Color AQUA(0, 4, 4);




class Game {

  public:

    Game() {
      lines = 0;
      level = 1;
      rotation = 0;
      time = 0;
      gameOver = false;
      buttonHold = false;
      gameOverCtr = 0;

      x = 7;
      y = 15;

      //sets tetrominos 4x4 in 1D array
      tetromino[0].concat("0000011001100000"); //O Yellow
      tetromino[1].concat("0000222200000000"); //I Aqua
      tetromino[2].concat("0300033000300000"); //S Green
      tetromino[3].concat("0040044004000000"); //Z Red
      tetromino[4].concat("0500050005500000"); //L Orange
      tetromino[5].concat("0060006006600000"); //J Blue
      tetromino[6].concat("0070077000700000"); //T Purple
    }


    void setupGame() {
      print_game_name();
      delay(3000);
      matrix.fillScreen(BLACK.to_333());
      delay(500);

      //initializes randomizer array
      //for (int i = 0; i < 7; i++) {
        //randomArr[i] = 7;
      //}

      // board array buffer with printed boundaries
      // used normal x and y
      // inverted x and y only in matrix printing
      for (int i = 0; i < boardWidth; i++) {
        for (int j = 0; j < boardHeight; j++) {
          int arrayPos = j * boardWidth + i;
          if ((arrayPos >= 2 && arrayPos <= 13) || (arrayPos > 15 && i == boardWidth - 3) || (arrayPos > 15 && i == boardWidth - 14)) {
            board[arrayPos] = 8;
            matrix.drawPixel(j, i, WHITE.to_333()); //flipped x and y
          } else {
            board[arrayPos] = 0;
            matrix.drawPixel(j, i, BLACK.to_333());
          }
        }
      }

      //prints lines and levels
      print_level(level);
      print_lines(lines);

      // set millis
      time = millis();
      lastPress = 0;


    }


    void update(int button_value) {
      if (!gameOver) {
        if (millis() - lastPress > 200 ) {
          if (button_value > 1021) { //left
            if (collision_checker(currentTetromino, rotation, x - 1, y)) {
              x -= 1;
            }
          } else if (button_value > 1000) { //fall
            while (collision_checker(currentTetromino, rotation, x, y - 1)) {
              y -= 1;
            }
          } else if (button_value > 900) { //right
            if (collision_checker(currentTetromino, rotation, x + 1, y)) {
              x += 1;
            }
          } else if (button_value > 500) { //rotate
            if (collision_checker(currentTetromino, rotation + 1, x, y)) {
              rotation += 1;
              if (rotation > 3) {
                rotation = 0;
              }
            }
          }
          //in case button remains pressed (for easier movement)
          if (button_value > 500) {
            if (buttonHold) {
              lastPress = millis() - 100;
            } else {
              lastPress = millis();
            }
            buttonHold = true; // this allows the next loop to recognize whether button was already pressed
          } else {
            buttonHold = false;
            lastPress = 0;
          }
        }

        //compute drop speed based on level
        dropSpeed = get_drop_speed();

        if ((millis() - time) > dropSpeed) {

          if (collision_checker(currentTetromino, rotation, x, y - 1)) {
            y -= 1;
            time = millis();
          } else { //IF PIECE CANNOT FALL

            // lock piece in place
            for (int i = 0; i < 4; i++) {
              for (int j = 0; j < 4; j++) {
                if (tetromino[currentTetromino][rotate_piece(i, j, rotation)] != '0') {
                  board[(y + j) * boardWidth + (x + i)] = currentTetromino + 1;
                }
              }
            }

            //line completion and update
            lineCtr = 0;
            for (int j = 1; j < boardHeight; j++) {
              for (int i = 3; i < 13; i++) {
                if (board[j * boardWidth + i] != 0) {
                  lineCtr++;
                  if (lineCtr == 10) {
                    for (int k = j + 1; k < boardHeight; k++) {
                      for (int i = 3; i < 13; i++) {
                        board[(k - 1) * boardWidth + i] = board[k * boardWidth + i];
                      }
                    }
                    lines++;
                    lineCtr = 0;
                  }
                } else {
                  lineCtr = 0;
                  break;
                }
              }
            }

            //quickly blacken rectangle so numbers dont overlap
            matrix.fillRect(19, 5, 31, 15, BLACK.to_333());

            //print lines
            print_lines(lines);
            // compute level
            level = 1 + (lines / 5);
            /*if (lines < 45) {
              level = 1 + (lines / 5);
            } else {
              level = 10;
            }*/
            //print level
            print_level(level);

            //get new piece. set time = millis(). re intitialize lastPress
            x = 7;
            y = 15;
            rotation = 0;
            currentTetromino = get_random_piece();
            //test for game over
            if (!collision_checker(currentTetromino, rotation, x, y)) {
              gameOver = true;
            }
          }
        }

        //Draw board
        for (int i = 0; i < boardWidth; i++) {
          for (int j = 0; j < boardHeight; j++) {
            int arrayPos = j * boardWidth + i;

            if (board[arrayPos] == 0) {
              matrix.drawPixel(j, i, BLACK.to_333());
            } else if (board[arrayPos] == 1) {
              matrix.drawPixel(j, i, YELLOW.to_333());
            } else if (board[arrayPos] == 2) {
              matrix.drawPixel(j, i, AQUA.to_333());
            } else if (board[arrayPos] == 3) {
              matrix.drawPixel(j, i, GREEN.to_333());
            } else if (board[arrayPos] == 4) {
              matrix.drawPixel(j, i, RED.to_333());
            } else if (board[arrayPos] == 5) {
              matrix.drawPixel(j, i, ORANGE.to_333());
            } else if (board[arrayPos] == 6) {
              matrix.drawPixel(j, i, BLUE.to_333());
            } else if (board[arrayPos] == 7) {
              matrix.drawPixel(j, i, PURPLE.to_333());
            } else if (board[arrayPos] == 8) {
              matrix.drawPixel(j, i, WHITE.to_333());
            }
          }
        }

        //Draw piece
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 4; j++) {
            if (tetromino[currentTetromino][rotate_piece(i, j, rotation)] != '0') {
              if (tetromino[currentTetromino][rotate_piece(i, j, rotation)] == '1') {
                matrix.drawPixel(y + j, x + i, YELLOW.to_333());
              } else if (tetromino[currentTetromino][rotate_piece(i, j, rotation)] == '2') {
                matrix.drawPixel(y + j, x + i, AQUA.to_333());
              } else if (tetromino[currentTetromino][rotate_piece(i, j, rotation)] == '3') {
                matrix.drawPixel(y + j, x + i, GREEN.to_333());
              } else if (tetromino[currentTetromino][rotate_piece(i, j, rotation)] == '4') {
                matrix.drawPixel(y + j, x + i, RED.to_333());
              } else if (tetromino[currentTetromino][rotate_piece(i, j, rotation)] == '5') {
                matrix.drawPixel(y + j, x + i, ORANGE.to_333());
              } else if (tetromino[currentTetromino][rotate_piece(i, j, rotation)] == '6') {
                matrix.drawPixel(y + j, x + i, BLUE.to_333());
              } else if (tetromino[currentTetromino][rotate_piece(i, j, rotation)] == '7') {
                matrix.drawPixel(y + j, x + i, PURPLE.to_333());
              }
            }
          }
        }
      } else {
        if (gameOverCtr == 0) {
          matrix.fillRect(0, 0, 32 - 11, 16, BLACK.to_333());
          gameOverCtr++;
          delay(100);
        }
        print_game_over();
        print_level(level);
        print_lines(lines);
      }
    }



  private:
    int x; // x position of tetromino (top left)
    int y; // y position of tetromino (top left)
    int level; //level increases every 10 lines cleared. level will influence drop speed
    int randomArr[7]; //randomizer array to randomize tetromino without repitition
    int lines; //every line cleared
    int rotation;
    int dropSpeed; //decreases as level increases
    unsigned long time;
    unsigned long lastPress; //for holding down button
    unsigned long nextStep; //for next auto movement
    int lineCtr;
    bool lineCompleted;
    bool gameOver;
    bool buttonHold;
    int currentTetromino;
    int gameOverCtr;

    int get_random_piece() {
      /*const int VALUES = 7;
      int ctr = 0;
      int ctr2 = 0;
      int val;
      bool valChecker = false;

      //check to see if array is full
      for (int i = 0; i < VALUES; i++) {
        if (randomArr[i] != 7) {
          ctr++;
        }
      }

      //array is full, re-initialize array. re-randomize the random generator
      if (ctr == 7) {
        for (int i = 0; i < VALUES; i++) {
          randomArr[i] = 7;
        }
        randomSeed(analogRead(12));
      }

      while (!valChecker) {
        val = random(0, 6);
        valChecker = true;
        //check to see if value is in array
        for (int i = 0; i < VALUES; i++) {
          if (randomArr[i] == val) {
            valChecker = false;
          }
        }
      }
      //value is not in array, and must be added to array
      for (int i = 0; i < VALUES; i++) {
        if (randomArr[i] == 7 && ctr2 == 0) {
          randomArr[i] = val;
          ctr2++;
        }
      }
      ctr = 0;
      ctr2 = 0;
      valChecker = false;
      return val;*/
      return random(0,6);
    }

    int get_drop_speed() {
      //speed in milliseconds
      if (level == 1) {
        return 500 * 2;
      //} else if (level == 2) {
      //  return 450 * 2;
      } else if (level == 2) {
        return 400 * 2;
      //} else if (level == 4) {
      //  return 350 * 2;
      } else if (level == 3) {
        return 300 * 2;
      } else if (level == 4) {
        return 250 * 2;
      } else if (level == 5) {
        return 200 * 2;
      } else if (level == 6) {
        return 150 * 2;
      } else if (level == 7) {
        return 100 * 2;
      } else {
        return 50 * 2;
      }
    }

    int rotate_piece(int xPos, int yPos, int r) {
      //using 1D array for tetromino, with algorithms to rotate the index
      r = r % 4; // in case rotation function in game is buggy
      int rVal;
      //rotating 4x4 matrix
      if (r == 0) {
        rVal = (yPos * 4) + xPos;
      } else if (r == 1) {
        rVal = yPos + 12 - (xPos * 4);
      } else if (r == 2) {
        rVal = 15 - xPos - (yPos * 4);
      } else if (r == 3) {
        rVal = 3 - yPos + (xPos * 4);
      }
      return rVal;
    }

    bool collision_checker(int piece, int rotationVal, int xPos, int yPos) {


      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {

          int newX = xPos + i;
          int newY = yPos + j;
          int tempPiece = rotate_piece(i, j, rotationVal);
          int tempBoard = (newY * boardWidth) + newX;

          if ((newX >= 0) && (newX < boardWidth)) {
            if ((newY >= 0) && (newY < boardHeight)) {
              //collision check
              if ((tetromino[piece][tempPiece] != '0') && (board[tempBoard] != 0)) {
                return false;
              }
            }
          }
        }
      }
      return true;
    }

};
// a global variable that represents the game Tetris
Game game;

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON, INPUT);
  matrix.begin();
  matrix.fillScreen(BLACK.to_333());

  // have to use randomSeed to make the random() different for each execution of the sketch
  randomSeed(analogRead(12));
  game.setupGame();
}

void loop() {
  int button_value = analogRead(BUTTON);
  //Serial.println(button_value);

  game.update(button_value);
  //delay(500);
}



void print_game_name() {
  //Draw T
  matrix.drawPixel(30, 0, RED.to_333());
  matrix.drawPixel(30, 1, RED.to_333());
  matrix.drawPixel(30, 2, RED.to_333());
  matrix.drawPixel(29, 1, RED.to_333());
  matrix.drawPixel(28, 1, RED.to_333());
  matrix.drawPixel(27, 1, RED.to_333());
  matrix.drawPixel(26, 1, RED.to_333());
  //Draw E
  matrix.drawPixel(25, 2, ORANGE.to_333());
  matrix.drawPixel(25, 3, ORANGE.to_333());
  matrix.drawPixel(25, 4, ORANGE.to_333());
  matrix.drawPixel(24, 2, ORANGE.to_333());
  matrix.drawPixel(23, 2, ORANGE.to_333());
  matrix.drawPixel(23, 3, ORANGE.to_333());
  matrix.drawPixel(23, 4, ORANGE.to_333());
  matrix.drawPixel(22, 2, ORANGE.to_333());
  matrix.drawPixel(21, 2, ORANGE.to_333());
  matrix.drawPixel(21, 3, ORANGE.to_333());
  matrix.drawPixel(21, 4, ORANGE.to_333());
  //Draw T
  matrix.drawPixel(20, 5, YELLOW.to_333());
  matrix.drawPixel(20, 6, YELLOW.to_333());
  matrix.drawPixel(20, 7, YELLOW.to_333());
  matrix.drawPixel(19, 6, YELLOW.to_333());
  matrix.drawPixel(18, 6, YELLOW.to_333());
  matrix.drawPixel(17, 6, YELLOW.to_333());
  matrix.drawPixel(16, 6, YELLOW.to_333());
  //Draw R
  matrix.drawPixel(15, 7, GREEN.to_333());
  matrix.drawPixel(15, 8, GREEN.to_333());
  matrix.drawPixel(15, 9, GREEN.to_333());
  matrix.drawPixel(14, 7, GREEN.to_333());
  matrix.drawPixel(14, 9, GREEN.to_333());
  matrix.drawPixel(13, 7, GREEN.to_333());
  matrix.drawPixel(13, 8, GREEN.to_333());
  matrix.drawPixel(12, 7, GREEN.to_333());
  matrix.drawPixel(12, 8, GREEN.to_333());
  matrix.drawPixel(11, 7, GREEN.to_333());
  matrix.drawPixel(11, 9, GREEN.to_333());
  //Draw I
  matrix.drawPixel(10, 10, BLUE.to_333());
  matrix.drawPixel(10, 11, BLUE.to_333());
  matrix.drawPixel(10, 12, BLUE.to_333());
  matrix.drawPixel(9, 11, BLUE.to_333());
  matrix.drawPixel(8, 11, BLUE.to_333());
  matrix.drawPixel(7, 11, BLUE.to_333());
  matrix.drawPixel(6, 10, BLUE.to_333());
  matrix.drawPixel(6, 11, BLUE.to_333());
  matrix.drawPixel(6, 12, BLUE.to_333());
  //Draw S
  matrix.drawPixel(5, 13, PURPLE.to_333());
  matrix.drawPixel(5, 14, PURPLE.to_333());
  matrix.drawPixel(5, 15, PURPLE.to_333());
  matrix.drawPixel(4, 13, PURPLE.to_333());
  matrix.drawPixel(3, 13, PURPLE.to_333());
  matrix.drawPixel(3, 14, PURPLE.to_333());
  matrix.drawPixel(3, 15, PURPLE.to_333());
  matrix.drawPixel(2, 15, PURPLE.to_333());
  matrix.drawPixel(1, 13, PURPLE.to_333());
  matrix.drawPixel(1, 14, PURPLE.to_333());
  matrix.drawPixel(1, 15, PURPLE.to_333());
}


void print_number(int x, int y, int num) {
  //print 0
  if (num == 0) {
    matrix.drawPixel(x, y, ORANGE.to_333());
    matrix.drawPixel(x, y + 1, ORANGE.to_333());
    matrix.drawPixel(x, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 1, y, ORANGE.to_333());
    matrix.drawPixel(x + 2, y, ORANGE.to_333());
    matrix.drawPixel(x + 3, y, ORANGE.to_333());
    matrix.drawPixel(x + 4, y, ORANGE.to_333());
    matrix.drawPixel(x + 1, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 3, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 1, ORANGE.to_333());
  }
  //print 1
  else if (num == 1) {
    matrix.drawPixel(x, y, ORANGE.to_333());
    matrix.drawPixel(x, y + 1, ORANGE.to_333());
    matrix.drawPixel(x, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 1, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 3, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 4, y, ORANGE.to_333());
  }
  //print 2
  else if (num == 2) {
    matrix.drawPixel(x, y, ORANGE.to_333());
    matrix.drawPixel(x, y + 1, ORANGE.to_333());
    matrix.drawPixel(x, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 1, y, ORANGE.to_333());
    matrix.drawPixel(x + 2, y, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 3, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 4, y, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 2, ORANGE.to_333());
  }
  //print 3
  else if (num == 3) {
    matrix.drawPixel(x, y, ORANGE.to_333());
    matrix.drawPixel(x, y + 1, ORANGE.to_333());
    matrix.drawPixel(x, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 1, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 2, y, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 3, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 4, y, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 2, ORANGE.to_333());

  }
  //print 4
  else if (num == 4) {
    matrix.drawPixel(x, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 1, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 2, y, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 3, y, ORANGE.to_333());
    matrix.drawPixel(x + 3, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 4, y, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 2, ORANGE.to_333());

  }
  //print 5
  else if (num == 5) {
    matrix.drawPixel(x, y, ORANGE.to_333());
    matrix.drawPixel(x, y + 1, ORANGE.to_333());
    matrix.drawPixel(x, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 1, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 2, y, ORANGE.to_333());
    matrix.drawPixel(x + 3, y, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 4, y, ORANGE.to_333());

  }
  //print 6
  else if (num == 6) {
    matrix.drawPixel(x, y, ORANGE.to_333());
    matrix.drawPixel(x, y + 1, ORANGE.to_333());
    matrix.drawPixel(x, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 1, y, ORANGE.to_333());
    matrix.drawPixel(x + 1, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 2, y, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 3, y, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 4, y, ORANGE.to_333());

  }
  //print 7
  else if (num == 7) {
    matrix.drawPixel(x, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 1, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 3, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 4, y, ORANGE.to_333());

  }
  //print 8
  else if (num == 8) {
    matrix.drawPixel(x, y, ORANGE.to_333());
    matrix.drawPixel(x, y + 1, ORANGE.to_333());
    matrix.drawPixel(x, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 1, y, ORANGE.to_333());
    matrix.drawPixel(x + 1, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 2, y, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 3, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 3, y, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 4, y, ORANGE.to_333());

  }
  //print 9
  else if (num == 9) {
    matrix.drawPixel(x, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 1, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 2, y, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 2, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 3, y, ORANGE.to_333());
    matrix.drawPixel(x + 3, y + 2, ORANGE.to_333());
    matrix.drawPixel(x + 4, y, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 1, ORANGE.to_333());
    matrix.drawPixel(x + 4, y + 2, ORANGE.to_333());
  }
}

void print_level(int level) {
  matrix.drawPixel(6 + 20, 0, AQUA.to_333());
  matrix.drawPixel(7 + 20, 0, AQUA.to_333());
  matrix.drawPixel(8 + 20, 0, AQUA.to_333());
  matrix.drawPixel(9 + 20, 0, AQUA.to_333());
  matrix.drawPixel(6 + 20, 2, BLUE.to_333());
  matrix.drawPixel(7 + 20, 1, BLUE.to_333());
  matrix.drawPixel(8 + 20, 1, BLUE.to_333());
  matrix.drawPixel(7 + 20, 3, BLUE.to_333());
  matrix.drawPixel(8 + 20, 3, BLUE.to_333());

  int num1;
  int num2;
  int num3;
  // use modulo 10
  if (level < 10) {
    print_number(6 + 20, 5, 0);
    print_number(6 + 20, 9, 0);
    print_number(6 + 20, 13, level);
  } else if (level < 100) {
    num2 = level / 10;
    num3 = level % 10;

    print_number(6 + 20, 5, 0);
    print_number(6 + 20, 9, num2);
    print_number(6 + 20, 13, num3);
  } else {
    num1 = level / 100;
    num2 = (level - (num1 * 100)) / 10;
    num3 = level % 10;

    print_number(6 + 20, 5, num1);
    print_number(6 + 20, 9, num2);
    print_number(6 + 20, 13, num3);
  }
}

void print_lines(int lines) {
  matrix.drawPixel(0 + 20, 0, AQUA.to_333());
  matrix.drawPixel(1 + 20, 0, AQUA.to_333());
  matrix.drawPixel(2 + 20, 0, AQUA.to_333());
  matrix.drawPixel(3 + 20, 0, AQUA.to_333());
  matrix.drawPixel(2 + 20, 2, BLUE.to_333());
  matrix.drawPixel(1 + 20, 1, BLUE.to_333());
  matrix.drawPixel(0 + 20, 1, BLUE.to_333());
  matrix.drawPixel(1 + 20, 3, BLUE.to_333());
  matrix.drawPixel(0 + 20, 3, BLUE.to_333());

  int num1;
  int num2;
  int num3;
  // use modulo 10
  if (lines < 10) {
    print_number(20, 5, 0);
    print_number(20, 9, 0);
    print_number(20, 13, lines);
  } else if (lines < 100) {
    num2 = lines / 10;
    num3 = lines % 10;

    print_number(20, 5, 0);
    print_number(20, 9, num2);
    print_number(20, 13, num3);
  } else {
    num1 = lines / 100;
    num2 = (lines - (num1 * 100)) / 10;
    num3 = lines % 10;

    print_number(20, 5, num1);
    print_number(20, 9, num2);
    print_number(20, 13, num3);
  }
}

// displays "GAME OVER"
void print_game_over() {
  //print GAME
  //print G
  matrix.drawPixel(26 - 11, 0, RED.to_333());
  matrix.drawPixel(26 - 11, 1, RED.to_333());
  matrix.drawPixel(26 - 11, 2, RED.to_333());
  matrix.drawPixel(26 - 11, 3, RED.to_333());
  matrix.drawPixel(25 - 11, 0, RED.to_333());
  matrix.drawPixel(24 - 11, 0, RED.to_333());
  matrix.drawPixel(24 - 11, 1, RED.to_333());
  matrix.drawPixel(24 - 11, 2, RED.to_333());
  matrix.drawPixel(24 - 11, 3, RED.to_333());
  matrix.drawPixel(23 - 11, 0, RED.to_333());
  matrix.drawPixel(23 - 11, 3, RED.to_333());
  matrix.drawPixel(22 - 11, 0, RED.to_333());
  matrix.drawPixel(22 - 11, 1, RED.to_333());
  matrix.drawPixel(22 - 11, 2, RED.to_333());
  matrix.drawPixel(22 - 11, 3, RED.to_333());
  //print A
  matrix.drawPixel(26 - 11, 5, WHITE.to_333());
  matrix.drawPixel(26 - 11, 6, WHITE.to_333());
  matrix.drawPixel(25 - 11, 4, WHITE.to_333());
  matrix.drawPixel(25 - 11, 7, WHITE.to_333());
  matrix.drawPixel(24 - 11, 4, WHITE.to_333());
  matrix.drawPixel(24 - 11, 5, WHITE.to_333());
  matrix.drawPixel(24 - 11, 6, WHITE.to_333());
  matrix.drawPixel(24 - 11, 7, WHITE.to_333());
  matrix.drawPixel(23 - 11, 4, WHITE.to_333());
  matrix.drawPixel(23 - 11, 7, WHITE.to_333());
  matrix.drawPixel(22 - 11, 4, WHITE.to_333());
  matrix.drawPixel(22 - 11, 7, WHITE.to_333());
  //print M
  matrix.drawPixel(26 - 11, 8, RED.to_333());
  matrix.drawPixel(26 - 11, 12, RED.to_333());
  matrix.drawPixel(25 - 11, 8, RED.to_333());
  matrix.drawPixel(25 - 11, 9, RED.to_333());
  matrix.drawPixel(25 - 11, 11, RED.to_333());
  matrix.drawPixel(25 - 11, 12, RED.to_333());
  matrix.drawPixel(24 - 11, 8, RED.to_333());
  matrix.drawPixel(24 - 11, 10, RED.to_333());
  matrix.drawPixel(24 - 11, 12, RED.to_333());
  matrix.drawPixel(23 - 11, 8, RED.to_333());
  matrix.drawPixel(23 - 11, 12, RED.to_333());
  matrix.drawPixel(22 - 11, 8, RED.to_333());
  matrix.drawPixel(22 - 11, 12, RED.to_333());
  //print E
  matrix.drawPixel(26 - 11, 13, WHITE.to_333());
  matrix.drawPixel(26 - 11, 14, WHITE.to_333());
  matrix.drawPixel(26 - 11, 15, WHITE.to_333());
  matrix.drawPixel(25 - 11, 13, WHITE.to_333());
  matrix.drawPixel(24 - 11, 13, WHITE.to_333());
  matrix.drawPixel(24 - 11, 14, WHITE.to_333());
  matrix.drawPixel(24 - 11, 15, WHITE.to_333());
  matrix.drawPixel(23 - 11, 13, WHITE.to_333());
  matrix.drawPixel(22 - 11, 13, WHITE.to_333());
  matrix.drawPixel(22 - 11, 14, WHITE.to_333());
  matrix.drawPixel(22 - 11, 15, WHITE.to_333());

  //print OVER
  //print O
  matrix.drawPixel(20 - 11, 2, WHITE.to_333());
  matrix.drawPixel(20 - 11, 3, WHITE.to_333());
  matrix.drawPixel(20 - 11, 4, WHITE.to_333());
  matrix.drawPixel(19 - 11, 2, WHITE.to_333());
  matrix.drawPixel(19 - 11, 4, WHITE.to_333());
  matrix.drawPixel(18 - 11, 2, WHITE.to_333());
  matrix.drawPixel(18 - 11, 4, WHITE.to_333());
  matrix.drawPixel(17 - 11, 2, WHITE.to_333());
  matrix.drawPixel(17 - 11, 4, WHITE.to_333());
  matrix.drawPixel(16 - 11, 2, WHITE.to_333());
  matrix.drawPixel(16 - 11, 3, WHITE.to_333());
  matrix.drawPixel(16 - 11, 4, WHITE.to_333());
  //print V
  matrix.drawPixel(20 - 11, 5, RED.to_333());
  matrix.drawPixel(20 - 11, 7, RED.to_333());
  matrix.drawPixel(19 - 11, 5, RED.to_333());
  matrix.drawPixel(19 - 11, 7, RED.to_333());
  matrix.drawPixel(18 - 11, 5, RED.to_333());
  matrix.drawPixel(18 - 11, 7, RED.to_333());
  matrix.drawPixel(17 - 11, 5, RED.to_333());
  matrix.drawPixel(17 - 11, 7, RED.to_333());
  matrix.drawPixel(16 - 11, 6, RED.to_333());
  //print E
  matrix.drawPixel(20 - 11, 8, WHITE.to_333());
  matrix.drawPixel(20 - 11, 9, WHITE.to_333());
  matrix.drawPixel(20 - 11, 10, WHITE.to_333());
  matrix.drawPixel(19 - 11, 8, WHITE.to_333());
  matrix.drawPixel(18 - 11, 8, WHITE.to_333());
  matrix.drawPixel(18 - 11, 9, WHITE.to_333());
  matrix.drawPixel(18 - 11, 10, WHITE.to_333());
  matrix.drawPixel(17 - 11, 8, WHITE.to_333());
  matrix.drawPixel(16 - 11, 8, WHITE.to_333());
  matrix.drawPixel(16 - 11, 9, WHITE.to_333());
  matrix.drawPixel(16 - 11, 10, WHITE.to_333());
  //print R
  matrix.drawPixel(20 - 11, 11, RED.to_333());
  matrix.drawPixel(20 - 11, 12, RED.to_333());
  matrix.drawPixel(20 - 11, 13, RED.to_333());
  matrix.drawPixel(19 - 11, 11, RED.to_333());
  matrix.drawPixel(19 - 11, 13, RED.to_333());
  matrix.drawPixel(18 - 11, 11, RED.to_333());
  matrix.drawPixel(18 - 11, 12, RED.to_333());
  matrix.drawPixel(17 - 11, 11, RED.to_333());
  matrix.drawPixel(17 - 11, 12, RED.to_333());
  matrix.drawPixel(16 - 11, 11, RED.to_333());
  matrix.drawPixel(16 - 11, 13, RED.to_333());
}
