/*
 * battleplanets
 *
 * A two player game. Each player shoots projectiles at the
 * other player's planet. The projectiles follow a path
 * determined by gravity.
 *
 * C.P.L.U.S.P.L.U.S
 *
 */

#include <iostream>
#include <ncurses.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include "Body.h"

using namespace std;

// Helper functions for the main game program
void wait(int);                                             // Allows pausing for animations
Body * checkcollision(const Missile*, Body *);              // Checks every body on the screen. Returns a pointer to the body collided with
void setupinterface(int, int);                              // Prints the main interface components of the game using ncurses functions
int inputparam(char [], char [], int, int);                 // The main user control function. Returns a value based on the keypress
void arrangeplanets(Body *, int, int, int);                 // Creates a random arrangement of planets so that none of them overlap, etc.
bool checkSides(Missile*, int, int);                        // Returns true if the projectile has reached the edge of the screen
Body * fireproj(Missile*, Body * head, bool, int, int);     // Packages up some other functions that combine to move the projectile across the screen
void printscore(int *, int, int);                           // Prints player scores on the screen and updates them
char* itoa(int, char*, int);                                // Used in printscore, converts an int to a char array

int main() {
  // This block of code initiates some relevant features of
  // ncurses.
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  int nlines, ncols;
  getmaxyx(stdscr, nlines, ncols);        // gets the dimensions of the terminal window

  // Creates a random group of planets to display to the screen
  int area = nlines * ncols;
  int num = area / 700;                // Number of planets. Scales to terminal size.
  Body planets[num];
  arrangeplanets(planets, num, nlines, ncols);
  Body * head = &planets[0];

  // Link the Bodies. Necessary for proper functioning of
  // the physics engine
  for (int i = 0; i < num - 2; ++i) {
    planets[i].setnext(&planets[i + 1]);
  }

  //Initialize player 1 and the score of each to 0. 
  bool player = 0;
  int ctrl = 1;
  int score[2] = {0, 0};
  // Main program loop
  while (ctrl) {
    if (ctrl == 2) {
      for (int i = 0; i < num; ++i) {       // Erase the current set of planets
        planets[i].erasebody();
      }
      arrangeplanets(planets, num, nlines, ncols);  // Gets a new planet arrangement, prints it, links the list
      head = &planets[0];
      for (int i = 0; i < num - 2; ++i) {
        planets[i].setnext(&planets[i + 1]);
      }
    }
    setupinterface(nlines, ncols);        // Repair UI elements that have been damaged
    for (int i = 0; i < num; ++i) {       // Repair planets that have been damaged
      planets[i].printbody();
    }
    wmove(stdscr, planets[0].gety(), planets[0].getx());    // Mark the player's planet
    player ? addch('1') : addch('1' | A_STANDOUT);          // Highlight if current player
    wmove(stdscr, planets[1].gety(), planets[1].getx());    // Mark the target planet
    player ? addch('2' | A_STANDOUT) : addch('2');          // Highlight if current player
    printscore(score, nlines, ncols);
    wrefresh(stdscr);

    Body * collided = NULL;
    // Get user inputs for angle and speed
    char vbuf[3];
    char vthetabuf[3];
    curs_set(1);
    ctrl = inputparam(vbuf, vthetabuf, nlines, ncols);            // Get user input for the projectile
    curs_set(0);
    if (ctrl == 3) {                                              // 'Fire' signal recieved
      double v1 = atof(vbuf);
      double vtheta = atof(vthetabuf);
      Body * start = (player ? &planets[1] : &planets[0]);        // The missile starts at the current player's planet
      Missile * missile1 = new Missile(start, v1, vtheta, 9);
      collided = fireproj(missile1, head, player, nlines, ncols); // If the missile collided with something, return the pointer to that object
      if (collided == (player ? &planets[0] : &planets[1])) {     // Did they hit the other player's planet?
        ++score[player];
      }
      player = !player;                                           // Switch players after every launch
      delete missile1;
    }
 }

  endwin();

  return 0;
}

//structure of wait program taken from ubuntuforums.org user
//OVERPOWER8
void wait(int mil) {
  clock_t endwait;
  endwait = clock() + mil * CLOCKS_PER_SEC / 1000;
  while (clock() < endwait) {}
}

Body * checkcollision(const Missile* proj, Body * head) {         // Checks whether the projectile is within a certain distance of the planet center
  Body * iterator = head;                                         // Checks the whole linked list
  while (iterator) {
    int rad = iterator->getsize() / 2;                            // body.size is the diameter, in number of lines; divide to get the radius
    double projx = proj->getx();
    double projy = proj->gety();
    double bodyx = iterator->getx();
    double bodyy = iterator->gety();
    double xdiff = abs(projx - bodyx) / 2;                        // chars are twice as tall as they are wide;
    double ydiff = abs(projy - bodyy);                            // dividing by two evens this up in the distance calculation.
    double dist = sqrt(pow(xdiff,2) + pow(ydiff, 2));
    if (dist < rad) {
      break;
    }
    iterator = iterator->getnext();
  }
  // Returns a pointer to the body collided with
  return iterator;                                               // If there hasn't been a collision, the null pointer is returned.
}

void setupinterface(int nlines, int ncols) {
  // Set up game interface
  // First, a header
  char title[] = "BATTLE PLANETS BETA -- BY C.P.L.U.S.P.L.U.S";
  wmove(stdscr, 1, ((ncols / 2) - (strlen(title) / 2)));         // center the title
  addstr(title);
  wmove(stdscr, 2, 0);
  hline('-', ncols);                                             // horizontal line

  wmove(stdscr, nlines - 4, 0);
  hline('-', ncols);
  char promptangle[] = "Enter an angle (0 - 360 degrees): ";        // Prompts to be displayed across the bottom of the screen
  char promptspeed[] = "Enter an initial speed for the projectile (0 - 10): ";
  char helptext[] = "<i>: Input a number    <f>: Fire projectile    <n>: New planet system    <q>: Quit";
  wmove(stdscr, nlines - 1, (ncols / 2) - (strlen(helptext) / 2));
  addstr(helptext);
  int space = 5;                                                                               // The space between the two prompts
  int promptcursor = (ncols / 2) - ((strlen(promptangle) + strlen(promptspeed) + space) / 2);  // The prompt will be centered at the bottom of the screen
  wmove(stdscr, nlines - 3, promptcursor);
  addstr(promptangle);
  wmove(stdscr, nlines - 3, promptcursor + strlen(promptangle) + space);   // Move the cursor to the beginning of the next string
  addstr(promptspeed);
  wmove(stdscr, nlines - 3, promptcursor + strlen(promptangle));           // Move cursor to first input position
  wrefresh(stdscr);
}

int inputparam(char vbuf[3], char vthetabuf[3], int nlines, int ncols) {
  // Get user input
  bool userinput = FALSE;
  int pos = 0;
  char promptangle[] = "Enter an angle (0 - 360 degrees): ";
  char promptspeed[] = "Enter an initial speed for the projectile (0 - 10): ";
  int space = 5;                                                                               // The space between the two prompts
  int promptcursor = (ncols / 2) - ((strlen(promptangle) + strlen(promptspeed) + space) / 2);  // The prompt will be centered at the bottom of the screen
 
  // Erase any pervious input
  wmove(stdscr, nlines - 3, promptcursor + strlen(promptangle));
  addstr("     ");
  wmove(stdscr, nlines - 3, promptcursor + strlen(promptangle) + strlen(promptspeed) + space);
  addstr("     ");

  // This next section powers all the user control
  // characters
  int ch;
  wmove(stdscr, nlines - 3, promptcursor + strlen(promptangle));
  wrefresh(stdscr);
  while(!userinput) {
    ch = getch();
    switch (ch) {
      case 'i':                                                                                  // 'i' for input. Inspired by vim.
        if (pos == 0) {
          echo();
          getstr(vthetabuf);
          noecho();
          wmove(stdscr, nlines - 3, promptcursor + strlen(promptangle) + strlen(promptspeed) + space);
          pos = 1;
        }
        else if (pos == 1) {
          echo();
          getstr(vbuf);
          noecho();
          wmove(stdscr, nlines - 3, promptcursor + strlen(promptangle));
          pos = 0;
        }
        break;
      case 'f':                                                                                 // 'f' for fire
        userinput = TRUE;
        return 3;
        break;
      case KEY_LEFT:
        pos = 0;
        wmove(stdscr, nlines - 3, promptcursor + strlen(promptangle));
        break;
      case KEY_RIGHT:
        pos = 1;
        wmove(stdscr, nlines - 3, promptcursor + strlen(promptangle) + strlen(promptspeed) + space);
        break;
      case 'q':
        return 0;
        break;
      case 'n':
        return 2;
        break;
    }
    wrefresh(stdscr);
  }
  return 1;
}

// Arranges the selected number of planets on the screen, at
// pseudo-random locations. Ensures they do not overlap or
// go off the edge of the screen.
void arrangeplanets(Body * planets, int num, int nlines, int ncols) {
  srand(time(NULL));
  int minlines = 3;                           // Bounaries take into account the UI elements
  int maxlines = nlines - 4;
  int linesrange = maxlines - minlines;
  int mincols = 0;
  int maxcols = ncols;
  int colsrange = maxcols - mincols;
  int sizes[5] = {3, 4, 5, 6, 9};
  int s[num];             // Stores the sizes of the bodes;
  double x[num], y[num];  // Stores the locations of the bodies;
  s[0] = 9;
  s[1] = 9;                         // There must be at least two planets of size 9
  for (int i = 2; i < num; ++i) {   // The rest of the sizes may be random (one of the five possible sizes)
    s[i] = rand() % 5;
    s[i] = sizes[s[i]];             // Must be one of the possible sizes
  }

  // Place the first planet in a random location on the screen; can't go off the screen
  // x values are double y values for the same distance, hence the differing algorithms
  x[0] = rand() % (colsrange - 2 * s[0]) + (mincols + s[0]);   
  y[0] = rand() % (linesrange - s[0]) + (minlines + s[0] / 2);
  for (int i = 1; i < num; ++i) {
  bool clear = FALSE;             // Flag is true if the new planet doesn't overlap with any of the previous ones
    while (!clear) {
      x[i] = rand() % (colsrange - 2 * s[i]) + (mincols + s[i]); 
      y[i] = rand() % (linesrange - s[i]) + (minlines + s[i] / 2);
      clear = TRUE;
      for (int j = i - 1; j >= 0; --j) {
        double dist = sqrt(pow((x[i] - x[j]) / 2, 2) + pow(y[i] - y[j], 2));    // Halve the x number to get distance
        double mindist = (double)s[i] / 2 + (double)s[j] / 2;
        if (dist < mindist) {
          clear = FALSE;
        }
      }
    }
  }

  // Construct the planets
  for (int i = 0; i < num; ++i) {
    planets[i] = Planet(x[i], y[i], s[i]);
  }
}

// Returns true if the missile goes out of bounds
bool checkSides(Missile* Projectile, int cols, int lines) {
  int mx = Projectile->getx();
  int my = Projectile->gety();
  if ((mx < (cols-2)) && (mx >1)) {
    if ((my < (lines-3)) && (my > 2)) {
      return false;
    }
    else {
      return true;
    }
  }
  else {
    return true;
  }
}

// Abstracts away the functions needed to fire the missile.
// Makes the loop in the main function simpler and easier to
// debug.
Body * fireproj(Missile* missile1, Body * head, bool player, int nlines, int ncols) {
  Body * collided = NULL;
  while (!collided) {
    bool sidecoll = false;
    double missileforce[2];
    missile1->getforce(head, missileforce);             // Part of the Body class. Calculates the force from all the other bodies' gravity
    missile1->setvelocity(missileforce);                // Part of the Body class. Sets velocity using dv = F/m dt
    missile1->movebody();                               // Part of the Body class. Moves the body according to its velocity
    collided = checkcollision(missile1, head);
    missile1->printbody();
    wait(100);
    missile1->erasebody();
    sidecoll = checkSides(missile1, ncols, nlines);
    if (sidecoll == true) {
      break;
      }
    }
  return collided;
}

// Uses ncurses.h to print the players' scores. Also uses an
// itoa function found online, below
void printscore(int score[2], int nlines, int ncols) {
  char score1[] = "Player 1 Score: ";
  char score1val[5];
  itoa(score[0], score1val, 10);
  char score2[] = "Player 2 Score: ";
  char score2val[5];
  itoa(score[1], score2val, 10);
  wmove(stdscr, 1, 5);                                  // Prints on the upper left of the screen
  addstr(score1);
  wmove(stdscr, 1, 5 + strlen(score1));
  addstr(score1val);
  wmove(stdscr, 1, ncols - 7 - strlen(score2));         // Prints on the upper right of the screen
  addstr(score2);
  wmove(stdscr, 1, ncols - 7);
  addstr(score2val);
}

/* itoa and reverse code taken from geeksforgeeks.org */
void reverse(char str[], int length) { 
  int start = 0; 
  int end = length -1; 
  while (start < end) { 
    swap(*(str+start), *(str+end)); 
      start++; 
      end--; 
  } 
} 

//Implementation of itoa() 
char* itoa(int num, char* str, int base) { 
  int i = 0; 
  bool isNegative = false; 
  /* Handle 0 explicitely, otherwise empty
  string is printed for 0 */
  if (num == 0) { 
    str[i++] = '0'; 
    str[i] = '\0'; 
    return str;
  }
  // In standard itoa(), negative numbers are handled only
  // with base 10. Otherwise numbers are considered unsigned. 
  if (num < 0 && base == 10) { 
    isNegative = true; 
    num = -num; 
  }
  // Process individual digits 
  while (num != 0) { 
    int rem = num % base; 
    str[i++] = (rem > 9)? (rem-10)+ 'a' : rem + '0'; 
    num = num/base; 
  }
  // If number is negative, append '-' 
  if (isNegative) 
    str[i++] = '-'; 

  str[i] = '\0'; // Append string terminator 
  
  // Reverse the string 
  reverse(str, i); 
  return str; 
} 
