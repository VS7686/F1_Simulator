
#ifndef PITCREW_H
#define PITCREW_H

#include <string>
using namespace std;

class PitCrew {
private:
    string name;
    int price;
    int speed;
    int consistency;

public:
    // Default constructor
    PitCrew() {
        name = "";
        price = 0;
        speed = 0;
        consistency = 0;
    }

    PitCrew(string n, int p, int sp, int co) {
        name = n;
        price = p;
        speed = sp;
        consistency = co;
    }

    string getName() const { return name; }
    int getPrice() const { return price; }
    int getSpeed() const { return speed; }
    int getConsistency() const { return consistency; }

    // How many score points one pit stop costs this crew (between 1 and 3).
    
    double getTimeLostPerStop() const {
        return 1.0 + 1.2 * (100 - speed) / 100.0 + 0.8 * (100 - consistency) / 100.0;
    }
};

#endif
