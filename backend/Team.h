
#ifndef TEAM_H
#define TEAM_H

#include <string>
#include "Driver.h"
#include "Car.h"
#include "MechanicCrew.h"
#include "PitCrew.h"
using namespace std;

class Team {
private:
    int budget;   // money the team has

public:
    string name;
    bool isPlayer;          // true for the player, false for oponnnent teams
    Driver driver;
    Car car;
    MechanicCrew mechanics;
    PitCrew pitCrew;
    int tyreChoice;         // index of the chosen tyre (0 = Soft - 4 = Wet)

    
    Team() {
        name = "";
        isPlayer = false;
        budget = 0;
        tyreChoice = 1;
    }

    Team(string n, bool player, int startBudget, Driver d, MechanicCrew m, PitCrew p)
        : driver(d), mechanics(m), pitCrew(p) {
        name = n;
        isPlayer = player;
        budget = startBudget;
        tyreChoice = 1;     // Medium by default
    }

    // ---- money handling ----
    int getBudget() const { return budget; }
    bool canAfford(int cost) const { return budget >= cost; }
    void spend(int cost) { budget -= cost; }
    void earn(int amount) { budget += amount; }
};

#endif
