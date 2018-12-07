class Body {


  public:

    void Construct(double, double);

    // retrieve protected information
    double getx() const;
    double gety() const;
    int getsize() const;
    double getmass() const;

    // Manipulate "string" of bodies
    void setnext(Body *);
    Body * getnext() const;

    // Functions that set physics
    void getforce(const Body *, double *) const;
    void setvelocity(const double *);
    void movebody();

    // Display functions
    void printbody() const;
    void erasebody() const;


  protected:

    double vtheta; // Angle in degrees counterclockwise from x+ of velocity
    double vmag;  // Magnitude of velocity in units per frame

    int x;
    int y;
    int size;

    double mass;
    Body * nextBody;

};

class Asteroid : public Body {

  public:
    Asteroid(double, double);
    Asteroid(double, double, int);

};

class Planet : public Body {

  public:
    Planet(double, double);
    Planet(double, double, int);
};

class Missile : public Body {

  public:
    Missile(const Body *, double, double, double);

    void printbody() const;
    void erasebody() const;
};

