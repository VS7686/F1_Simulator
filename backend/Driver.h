// ============================================================
// Driver.h
// A Driver is someone you can hire from the market.
// Every stat is a number from 1 to 100 (higher = better).
// The stats are private (encapsulation): the outside world can
// only READ them through the getter functions.
// ============================================================
#ifndef DRIVER_H
#define DRIVER_H

#include <string>
using namespace std;

class Driver {
private:
    string name;
    int price;           // cost to hire in the market
    int pace;            // raw speed
    int consistency;     // makes fewer mistakes (less random results)
    int aggression;      // overtakes more, but crashes more often
    int tyreManagement;  // looks after the tyres (better tyre score)
    int wetSkill;        // skill driving in the rain

public:
    // Default constructor: creates an empty driver
    Driver() {
        name = "";
        price = 0;
        pace = 0;
        consistency = 0;
        aggression = 0;
        tyreManagement = 0;
        wetSkill = 0;
    }

    // Constructor: sets all the stats
    Driver(string n, int p, int pc, int co, int ag, int tm, int ws) {
        name = n;
        price = p;
        pace = pc;
        consistency = co;
        aggression = ag;
        tyreManagement = tm;
        wetSkill = ws;
    }

    // ---- Getters (read-only access to private data) ----
    string getName() const { return name; }
    int getPrice() const { return price; }
    int getPace() const { return pace; }
    int getConsistency() const { return consistency; }
    int getAggression() const { return aggression; }
    int getTyreManagement() const { return tyreManagement; }
    int getWetSkill() const { return wetSkill; }

    // One overall number (0-100) that sums up the driver.
    // Pace counts the most, the rest are smaller parts.
    double getRating() const {
        return pace * 0.5 + consistency * 0.2 + aggression * 0.1
             + tyreManagement * 0.1 + wetSkill * 0.1;
    }
};

#endif
