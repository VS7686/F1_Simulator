

// COMPOSITION:
//   Car has 4 parts (Engine, PowerUnit, FrontWing, RearWing).
//   getPart(i) returns a CarPart pointer for polymorphism.

#ifndef CAR_H
#define CAR_H

#include <string>
#include "Circuit.h"
using namespace std;

const int MAX_PART_LEVEL = 5;              // highest level a part can reach
const int UPGRADE_COST_PER_LEVEL = 30000;  // upgrading from level N costs N * 30000

// Base class 
class CarPart {
protected:
    string name;
    int level;             // MAX_PART_LEVEL

public:
    CarPart(string n) {
        name = n;
        level = 1;         // every part starts at level 1
    }

    
    virtual ~CarPart() {}    // Virtual destructer 

    
    
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
        return getRaw() * (0.5 + c.powerDemand / 200.0);             //powerdemand defined for a circuit 
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

// ---------------- Car ----------------

class Car {
private:
    Engine engine;
    PowerUnit powerUnit;
    FrontWing frontWing;
    RearWing rearWing;

public:
    // Get a part by number: 0 = Engine, 1 = Power Unit, 2 = Front Wing, 3 = Rear Wing
    // Returns a pointer to the BASE class, but it really points to a child object. upcasting 
    CarPart* getPart(int i) {
        if (i == 0) return &engine;
        if (i == 1) return &powerUnit;
        if (i == 2) return &frontWing;
        return &rearWing;
    }
    // Same thing for a read-only car
    const CarPart* getPart(int i) const {
        if (i == 0) return &engine;
        if (i == 1) return &powerUnit;
        if (i == 2) return &frontWing;
        return &rearWing;
    }

    // Overall car rating (0-100) on a given track = average of the 4 parts.
    // Each part decides its own contribution (polymorphism).
    double getOverallRating(const Circuit& c) const {
        double total = 0;
        for (int i = 0; i < 4; i++) {
            total += getPart(i)->getContribution(c);
        }
        return total / 4.0;
    }

    // Summary stats (0-100) shown in the "current car stats" box 
    int getPowerStat() const { return engine.getLevel() * 20; }
    int getDownforceStat() const {
        return (frontWing.getLevel() + rearWing.getLevel()) * 10;
    }
    int getReliabilityStat() const {
        return (engine.getLevel() + powerUnit.getLevel()) * 10;
    }
    int getEfficiencyStat() const {
        return (powerUnit.getLevel() + rearWing.getLevel()) * 10;
    }

    // Used to build oponent cars: set all four levels at once
    void setLevels(int e, int pu, int fw, int rw) {
        engine.setLevel(e);
        powerUnit.setLevel(pu);
        frontWing.setLevel(fw);
        rearWing.setLevel(rw);
    }
};

#endif
