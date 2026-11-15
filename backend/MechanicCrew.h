// ============================================================
// MechanicCrew.h
// The mechanics who build and set up the car.
// Bought from the market. Stats are 1-100.
//   reliability : lowers the chance the car breaks down (DNF)
//   setupSkill  : gives a small bonus to the car's race performance
// ============================================================
#ifndef MECHANICCREW_H
#define MECHANICCREW_H

#include <string>
using namespace std;

class MechanicCrew {
private:
    string name;
    int price;
    int reliability;
    int setupSkill;

public:
    // Default constructor
    MechanicCrew() {
        name = "";
        price = 0;
        reliability = 0;
        setupSkill = 0;
    }

    MechanicCrew(string n, int p, int rel, int setup) {
        name = n;
        price = p;
        reliability = rel;
        setupSkill = setup;
    }

    string getName() const { return name; }
    int getPrice() const { return price; }
    int getReliability() const { return reliability; }
    int getSetupSkill() const { return setupSkill; }
};

#endif
