// ============================================================
// Game.h
// The Game class owns everything: the player's team, the 10 AI
// teams, the circuits, and the market (lists of drivers and crews).
// The web server (main.cpp) just calls these functions.
//
// Functions that can fail return a string:
//   "" (empty) = it worked,   otherwise = the error message to show.
// ============================================================
#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include "Team.h"
#include "Race.h"
#include "Circuit.h"
#include "Tyre.h"
using namespace std;

const int STARTING_BUDGET = 300000;
const int NUM_DRIVERS = 5;
const int NUM_MECHANICS = 5;
const int NUM_PIT_CREWS = 5;
const int NUM_AI_TEAMS = 10;

class Game {
public:
    // ---- the market (normal fixed-size arrays) ----
    Driver driverShop[NUM_DRIVERS];
    MechanicCrew mechanicShop[NUM_MECHANICS];
    PitCrew pitCrewShop[NUM_PIT_CREWS];

    // ---- the world ----
    vector<Circuit> circuits;
    vector<Tyre> tyres;
    Team aiTeams[NUM_AI_TEAMS]; // the 10 opponents
    Team player;                // the player's team
    int raceNumber;             // how many races have been completed

    // The constructor sets everything up for a fresh game
    Game() {
        newGame();
    }

    // Resets everything back to the start of the game
    void newGame() {
        circuits = Circuit::createAll();
        tyres = Tyre::createAll();
        fillShop();

        // The player starts with the first (free) item of each shop list
        player = Team("My Racing Team", true, STARTING_BUDGET,
                      driverShop[0], mechanicShop[0], pitCrewShop[0]);
        raceNumber = 0;

        createAiTeams();
    }

    // ---------------- Buying ----------------

    string buyDriver(int index) {
        if (index < 0 || index >= NUM_DRIVERS) return "Unknown driver.";
        const Driver& d = driverShop[index];
        if (d.getName() == player.driver.getName()) return "You already have this driver.";
        if (!player.canAfford(d.getPrice())) return "Not enough money.";
        player.spend(d.getPrice());
        player.driver = d;            // replaces the old driver (no refund)
        return "";
    }

    string buyMechanic(int index) {
        if (index < 0 || index >= NUM_MECHANICS) return "Unknown mechanic crew.";
        const MechanicCrew& m = mechanicShop[index];
        if (m.getName() == player.mechanics.getName()) return "You already have this crew.";
        if (!player.canAfford(m.getPrice())) return "Not enough money.";
        player.spend(m.getPrice());
        player.mechanics = m;
        return "";
    }

    string buyPitCrew(int index) {
        if (index < 0 || index >= NUM_PIT_CREWS) return "Unknown pit crew.";
        const PitCrew& p = pitCrewShop[index];
        if (p.getName() == player.pitCrew.getName()) return "You already have this crew.";
        if (!player.canAfford(p.getPrice())) return "Not enough money.";
        player.spend(p.getPrice());
        player.pitCrew = p;
        return "";
    }

    // Upgrade one car part by one level. partIndex: 0 Engine, 1 PU, 2 Front Wing, 3 Rear Wing
    string upgradePart(int partIndex) {
        if (partIndex < 0 || partIndex > 3) return "Unknown car part.";
        CarPart* part = player.car.getPart(partIndex);
        if (part->isMaxed()) return "This part is already at max level.";
        int cost = part->getUpgradeCost();
        if (!player.canAfford(cost)) return "Not enough money.";
        player.spend(cost);
        part->upgrade();
        return "";
    }

    string setTeamName(string newName) {
        if (newName.length() == 0) return "Team name cannot be empty.";
        if (newName.length() > 20) newName = newName.substr(0, 20);
        player.name = newName;
        return "";
    }

    // ---------------- Racing ----------------

    // Runs one race. The results are saved in 'results'.
    // The player's prize money is added to their budget.
    string runRace(int circuitIndex, int tyreIndex, vector<RaceResult>& results) {
        if (circuitIndex < 0 || circuitIndex >= (int)circuits.size()) return "Unknown track.";
        if (tyreIndex < 0 || tyreIndex >= (int)tyres.size()) return "Unknown tyre.";

        player.tyreChoice = tyreIndex;

        // Put all 11 teams in one list (pointers, so the race can change their tyre choice)
        vector<Team*> allTeams;
        allTeams.push_back(&player);
        for (int i = 0; i < NUM_AI_TEAMS; i++) {
            allTeams.push_back(&aiTeams[i]);
        }

        results = Race::simulate(allTeams, circuits[circuitIndex]);

        // Pay the player
        for (int i = 0; i < (int)results.size(); i++) {
            if (results[i].isPlayer) player.earn(results[i].prize);
        }
        raceNumber++;
        return "";
    }

private:
    // ---------------- Setting up the data ----------------

    // The market lists. The FIRST item of each list is the free starter.
    void fillShop() {
        //                            name    price  pace cons aggr tyre wet
        driverShop[0] = Driver("Lance Troll", 0,     40,  49,  50,  50,  50);
        driverShop[1] = Driver("Charles Wall",80000, 62,  60,  55,  58,  55);
        driverShop[2] = Driver("Yukimon",     160000,72,  70,  50,  70,  65);
        driverShop[3] = Driver("Sir Lew",     280000,92,  85,  89,  72,  85);
        driverShop[4] = Driver("Mad Max",     450000,95,  85,  99,  85,  98);

        //                                    name                 price  reliability setup
        mechanicShop[0] = MechanicCrew("Garage Hands",           0,    40,  40);
        mechanicShop[1] = MechanicCrew("Pit Lane Pros",      40000,    55,  55);
        mechanicShop[2] = MechanicCrew("Torque Team",        90000,    70,  65);
        mechanicShop[3] = MechanicCrew("Apex Works",        150000,    82,  80);
        mechanicShop[4] = MechanicCrew("Elite Engineering", 250000,    95,  92);

        //                              name                  price  speed consistency
        pitCrewShop[0] = PitCrew("Weekend Crew",             0,    40,  40);
        pitCrewShop[1] = PitCrew("Quick Fix Crew",       40000,    55,  55);
        pitCrewShop[2] = PitCrew("Rapid Wheels",         90000,    70,  68);
        pitCrewShop[3] = PitCrew("Lightning Crew",      150000,    82,  80);
        pitCrewShop[4] = PitCrew("Perfect Stop Crew",   250000,    95,  95);
    }

    // Builds one AI team. Levels: e = engine, pu = power unit, fw = front wing, rw = rear wing
    void addAiTeam(int index, string teamName, Driver d, int e, int pu, int fw, int rw,
                   int mechanicIndex, int pitCrewIndex) {
        Team t(teamName, false, 0, d, mechanicShop[mechanicIndex], pitCrewShop[pitCrewIndex]);
        t.car.setLevels(e, pu, fw, rw);
        aiTeams[index] = t;
    }

    // The 10 opponents. Their stats are fixed and never change.
    // They are listed from strongest to weakest.
    void createAiTeams() {
        //                       idx  team name         driver (name, price, pace, cons, aggr, tyre, wet)       E PU FW RW  mech pit
        addAiTeam(0, "Crimson Arrow",  Driver("Lars Eriksen",  0, 88, 84, 62, 78, 72),  5, 4, 5, 4,  4, 4);
        addAiTeam(1, "Silver Falcon",  Driver("Marco Bellini", 0, 84, 80, 70, 74, 66),  4, 5, 4, 4,  3, 4);
        addAiTeam(2, "Blue Meteor",    Driver("Kenji Mori",    0, 80, 82, 55, 80, 78),  4, 4, 4, 3,  4, 3);
        addAiTeam(3, "Orange Fox",     Driver("Daniel Ward",   0, 78, 72, 68, 70, 60),  3, 4, 3, 4,  3, 3);
        addAiTeam(4, "Green Viper",    Driver("Pablo Ortega",  0, 74, 70, 75, 64, 62),  3, 3, 4, 3,  2, 3);
        addAiTeam(5, "Black Stallion", Driver("Hugo Laurent",  0, 70, 72, 58, 72, 64),  3, 3, 3, 3,  2, 2);
        addAiTeam(6, "Yellow Hornet",  Driver("Ivan Petrov",   0, 66, 60, 72, 58, 55),  2, 3, 2, 3,  2, 1);
        addAiTeam(7, "White Wolf",     Driver("Sam Okafor",    0, 62, 64, 55, 62, 60),  2, 2, 3, 2,  1, 2);
        addAiTeam(8, "Purple Raven",   Driver("Luca Rossi",    0, 58, 55, 60, 52, 50),  2, 2, 2, 1,  1, 1);
        addAiTeam(9, "Teal Shark",     Driver("Arjun Mehta",   0, 54, 52, 50, 50, 48),  1, 2, 1, 2,  1, 0);
    }
};

#endif
