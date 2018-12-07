#include<iostream>
#include<cmath>
#include<ncurses.h>
#include<cstring>
#include"Body.h"
#define PI 3.1415

// Generic construction function for initializing variables
void Body::Construct(double x0, double y0) {
  this->x = x0;
  this->y = y0;

  this->vtheta = 0;
  this->vmag = 0;

  this->nextBody = 0;
}

// Initialize the private variables
Asteroid::Asteroid(double x0, double y0) {
  this->Construct(x0, y0);

  this->size = 6;  // size of asteroid will be equal to 6 if not specified
  this->mass = this->size;
  this->nextBody = 0;  // pointer to next body automatically null
};

Asteroid::Asteroid(double x0, double y0, int s) {
  this->Construct(x0, y0);

  this->size = s;  // size can be 3, 4, 5, 6, or 9
  this->mass = this->size;
  this->nextBody = 0;  // pointer to next body automatically null
};

Planet::Planet(double x0, double y0) {
  this->Construct(x0, y0);

  this->size = 9; // size of planets will be 9 if not specified
  this->mass = this->size;
  this->nextBody = 0;  // pointer to next body automatically null
};

Planet::Planet(double x0, double y0, int s) {
  this->Construct(x0, y0);

  this->size = s;  // size can be 3, 4, 5, 6, or 9
  this->mass = this->size;
  this->nextBody = 0;  // pointer to next body automatically null
};



Missile::Missile(const Body * Origin, double v0, double vphi, double originrad) {

  vphi = vphi * PI / 180; // Convert vphi to radians

  this->vtheta = vphi;  // Starting velocity for bodies is 0 unless otherwise specified
  this->vmag = v0;

  int delx = ceil(originrad * cos(vphi));      // Set x relative to origin body since triangle
  int dely = ceil(originrad / 2 * sin(vphi));  // formed by hypotenuse of radius and angle vphi

  this->x = Origin->getx() + delx;         // (x, y) of body is input to initialization
  this->y = Origin->gety() + dely;     // Divide by two to even with x coordinate length


  this->mass = 3;
  this->nextBody = 0;  // pointer to next body automatically null

};



double Body::getx() const {
  return this->x;
}

double Body::gety() const {
  return this->y;
}

int Body::getsize() const {
  return this->size;
}

double Body::getmass() const {
  return this->mass;
}


// Set next body so that there is a chain of bodies to check
// This doesn't pass a const Body * due to error in converting "const Body *" to "Body *"
void Body::setnext(Body * nextOne) {
  this->nextBody = nextOne;
}

Body * Body::getnext() const {
  return this->nextBody;
}



// Obtain the force in "forceptr" acted on an object
// "head" is the head object in the program
// This function is recursive, going down the list of objects
// until it reaches the end
void Body::getforce(const Body * inputBody, double * forceptr) const {


  // Obtain the distance between this body and the one input
  double delx = (inputBody->x - this->x) / 2;   // distance units in the x-direction are half as long
  double dely = inputBody->y - this->y;
  double Ftheta = 0; // Initialize Ftheta



  // If both delx and dely are zero, no angle is used due to that
  // the objects occupy the same location
  if (delx || dely)
    Ftheta = atan(dely/delx); 

  // If delx is negative, alter Ftheta by PI since atan assumes
  // positive delx
  if (delx < 0) {
    Ftheta += PI;
  }

  double distsq = pow(delx, 2) + pow(dely, 2); // distance squared

  double Fmag = 0; // Initialize Fmag
  if (distsq) // Only set Fmag if distsq is nonzero since concurrent objects
    Fmag = (this->mass * inputBody->mass)/distsq; // don't set infinite force

  double Fx = Fmag * cos(Ftheta);
  double Fy = Fmag * sin(Ftheta);

  // If this is not the last body, move onto the next one
  // and increment the force components
  if (inputBody->nextBody) {
    double nextForce[2];
    getforce(inputBody->nextBody, nextForce);

    Fx += nextForce[0];
    Fy += nextForce[1];
  }

  // Values are passed out through the array being pointed to
  forceptr[0] = Fx;
  forceptr[1] = Fy;
}


// Set a new velocity for the Body based on the Force exerted on it
void Body::setvelocity(const double * force) {

  // Retrieve raw velocity data
  double oldvmag = this->vmag;
  double oldvtheta = this->vtheta;
  double m = this->mass;

  // Break velocity data into components
  double vx = oldvmag * cos(oldvtheta);
  double vy = oldvmag * sin(oldvtheta);

  // Increment the velocity data by the acceleration from input force
  vx += (force[0] / m);
  vy += (force[1] / m);

  // Set new velocity information
  this->vmag = sqrt(pow(vx, 2) + pow(vy, 2));
  double newvtheta = atan(vy / vx);

  // If vx is negative, rotate PI due to range of atan
  if (vx < 0)
    newvtheta += PI;

  this->vtheta = newvtheta;
}


// Move the body based on the current velocity
void Body::movebody() {

  // Retrieve raw velocity and position data
  double v = this->vmag;
  double phi = this->vtheta;
  double oldx = this->x;
  double oldy = this->y;

  // Obtain velocity components
  double vx = v * cos(phi);
  double vy = v * sin(phi);

  // Step up x and y using v components
  this->x = oldx + vx;
  this->y = oldy + vy / 2;      // Height of a char is twice the width
}



// Display the body, using ncurses.h functions
void Body::printbody() const {
  int lines = this->y; 
  int cols = this->x;
  int size = this->size;

  int width = 2 * size;

  char circle[5][9][20] =         // Hard-coded strings for the circles, to be printed by addstr
    {
      {"__\0",
       "/  \\\0",
       "\\__/\0",
       "\0",
       "\0",
       "\0",
       "\0",
       "\0",
       "\0"},
      {"___\0",
       "/   \\\0",
       "|     |\0",
       "\\___/\0",
       "\0",
       "\0",
       "\0",
       "\0",
       "\0"},
      {".-''-.\0",
       "/      \\\0",
       "|        |\0",
       "\\      /\0",
       "`-..-'\0",
       "\0",
       "\0",
       "\0",
       "\0"},
      {"____\0",
       ".'    `.\0",
       "/        \\\0",
       "|        |\0",
       "\\        /\0",
       "`.____.'\0",
       "\0",
       "\0",
       "\0"},
      {"_.-\"\"\"\"-._\0",
       ".'          `.\0",
       "/              \\\0",
       "|                |\0",
       "|                |\0",
       "|                |\0",
       "\\              /\0",
       "`._        _.'\0",
       "`-....-'\0"}
    };

  int num;                                // This variable chooses which of the five circles to print
  switch (size) {
    case 3:
      num = 0;
      break;
    case 4:
      num = 1;
      break;
    case 5:
      num = 2;
      break;
    case 6:
      num = 3;
      break;
    case 9:
      num = 4;
      break;
  }

  int x = cols - size;                // The input location is for the center of the circle. Find the top right corner for printing.
  int y = lines - (size / 2);
  for (int i = 0; i < size; ++i) {
    int whitespace = ((width - strlen(circle[num][i])) / 2);  // Instead of spaces before the strings, move the cursor. That way, no
    wmove(stdscr, y + i, x + whitespace);                     // old objects get overwritten.
    addstr(circle[num][i]);
  }

  wrefresh(stdscr);     // Refresh the screen to print the circle
}


// Prints over the selected Body with whitespace, using
// ncurses functions
void Body::erasebody() const {
  int lines = this->y;
  int cols = this->x;
  int size = this->size; 

  int width = 2 * size;           // char width is half char height

  char circle[5][9][20] =         // Whitespace strings exactly match the length of the circle strings above
    {
      {"  \0",
       "    \0",
       "    \0",
       "\0",
       "\0",
       "\0",
       "\0",
       "\0",
       "\0"},
      {"   \0",
       "     \0",
       "       \0",
       "      \0",
       "\0",
       "\0",
       "\0",
       "\0",
       "\0"},
      {"      \0",
       "        \0",
       "          \0",
       "        \0",
       "      \0",
       "\0",
       "\0",
       "\0",
       "\0"},

      {"    \0",
       "        \0",
       "          \0",
       "          \0",
       "          \0",
       "        \0",
       "\0",
       "\0",
       "\0"},

      {"          \0",
       "              \0",
       "                \0",
       "                  \0",
       "                  \0",
       "                  \0",
       "                \0",
       "              \0",
       "        \0"}
    };

  int num;
  switch (size) {         // Converts circle size to position in array
    case 3:
      num = 0;
      break;
    case 4:
      num = 1;
      break;
    case 5:
      num = 2;
      break;
    case 6:
      num = 3;
      break;
    case 9:
      num = 4;
      break;
  }

  int x = cols - size;
  int y = lines - (size / 2);
  for (int i = 0; i < size; ++i) {
    int whitespace = ((width - strlen(circle[num][i])) / 2);    // Moves the cursor instead of printing leading whitespace
    wmove(stdscr, y + i, x + whitespace);                       // Prints by moving the cursor and using addstr
    addstr(circle[num][i]);
  }

  wrefresh(stdscr);                                             // Actually updates the terminal screen
}


// Prints a missile. Just a single char, so simpler than
// above
void Missile::printbody() const {
  int lines = this->y;
  int cols = this->x;
  char projectile = '+';
  wmove(stdscr, lines, cols);
  addch(projectile);
  wrefresh(stdscr);
}

// Prints a single space over the missile location
void Missile::erasebody() const {
  int lines = this->y;
  int cols = this->x;
  char erase = ' ';
  wmove(stdscr,lines, cols);
  addch(erase);
  wrefresh(stdscr);
}

