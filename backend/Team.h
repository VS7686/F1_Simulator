// ============================================================
// Team.h
// A Team is the player OR one of the 10 AI opponents.
// It "has" a driver, a car, a mechanic crew and a pit crew
// (composition). The money (budget) is private, so it can only
// change through spend() and earn().
// ============================================================
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
    bool isPlayer;          // true for the player, false for AI teams
    Driver driver;
    Car car;
    MechanicCrew mechanics;
    PitCrew pitCrew;
    int tyreChoice;         // index of the chosen tyre (0 = Soft ... 4 = Wet)

    // Default constructor
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
