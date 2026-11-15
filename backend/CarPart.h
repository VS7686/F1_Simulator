// ============================================================
// CarPart.h
// INHERITANCE + POLYMORPHISM example.
//
//   CarPart  (abstract base class)
//     |-- Engine
//     |-- PowerUnit
//     |-- FrontWing
//     |-- RearWing
//
// Every part has a LEVEL from 1 to 5 (like Hill Climb Racing:
// you pay money to upgrade a part one level at a time).
// Each part type overrides getContribution() because each part
// reacts to the circuit in a different way.
// ============================================================
#ifndef CARPART_H
#define CARPART_H

#include <string>
#include "Circuit.h"
using namespace std;

const int MAX_PART_LEVEL = 5;        // highest level a part can reach
const int UPGRADE_COST_PER_LEVEL = 30000;  // upgrading from level N costs N * 30000

// ---------------- Base class ----------------
class CarPart {
protected:                 // protected = visible to child classes only
    string name;
    int level;             // 1 .. MAX_PART_LEVEL

public:
    CarPart(string n) {
        name = n;
        level = 1;         // every part starts at level 1
    }

    // A virtual destructor is needed for any class with virtual functions
    virtual ~CarPart() {}

    // PURE VIRTUAL functions (= 0): the base class has no code for them,
    // every child class MUST write its own version.
    virtual double getContribution(const Circuit& c) const = 0;  // 0-100 score on a track
    virtual string getDescription() const = 0;                   // short text for the UI

    // ---- normal functions shared by all parts ----
    string getName() const { return name; }
    int getLevel() const { return level; }
    bool isMaxed() const { return level >= MAX_PART_LEVEL; }

    // Raw strength of the part from its level alone: 20, 40, 60, 80, 100
    double getRaw() const { return level * 20.0; }

    // Price of the NEXT upgrade (level 1 -> 2 costs 30000, 2 -> 3 costs 60000 ...)
    int getUpgradeCost() const { return UPGRADE_COST_PER_LEVEL * level; }

    // The only way to raise the level (encapsulation)
    void upgrade() {
        if (level < MAX_PART_LEVEL) level++;
    }

    // Used to build the AI teams with pre-set levels
    void setLevel(int l) {
        if (l < 1) l = 1;
        if (l > MAX_PART_LEVEL) l = MAX_PART_LEVEL;
        level = l;
    }
};

// ---------------- Engine ----------------
// Matters MORE on tracks with long straights (high powerDemand).
class Engine : public CarPart {
public:
    Engine() : CarPart("Engine") {}

    double getContribution(const Circuit& c) const {
        // multiplier goes from 0.5 (no power needed) to 1.0 (lots of power needed)
        return getRaw() * (0.5 + c.powerDemand / 200.0);
    }
    string getDescription() const { return "Raw power. Best on fast tracks like Monza."; }
};

// ---------------- Power Unit ----------------
// The hybrid system. Helps a little on EVERY track (does not depend on the circuit).
class PowerUnit : public CarPart {
public:
    PowerUnit() : CarPart("Power Unit") {}

    double getContribution(const Circuit& c) const {
        return getRaw() * 0.85;   // same on every track
    }
    string getDescription() const { return "Hybrid boost. Helps on every track."; }
};

// ---------------- Front Wing ----------------
// Matters MORE on twisty tracks (high downforceDemand).
class FrontWing : public CarPart {
public:
    FrontWing() : CarPart("Front Wing") {}

    double getContribution(const Circuit& c) const {
        return getRaw() * (0.5 + c.downforceDemand / 200.0);
    }
    string getDescription() const { return "Grip in corners. Best at Monaco."; }
};

// ---------------- Rear Wing ----------------
// Same idea as the front wing.
class RearWing : public CarPart {
public:
    RearWing() : CarPart("Rear Wing") {}

    double getContribution(const Circuit& c) const {
        return getRaw() * (0.5 + c.downforceDemand / 200.0);
    }
    string getDescription() const { return "Stability and downforce."; }
};

#endif
