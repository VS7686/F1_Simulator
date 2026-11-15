// ============================================================
// Car.h
// COMPOSITION example: a Car "has" four parts.
// The car keeps the parts as normal member objects, and
// getPart(i) gives back a CarPart pointer so we can loop over
// them and call the right getContribution() for each
// (this is POLYMORPHISM in action).
// ============================================================
#ifndef CAR_H
#define CAR_H

#include "CarPart.h"
#include "Circuit.h"

class Car {
private:
    Engine engine;
    PowerUnit powerUnit;
    FrontWing frontWing;
    RearWing rearWing;

public:
    // Get a part by number: 0 = Engine, 1 = Power Unit, 2 = Front Wing, 3 = Rear Wing
    // Returns a pointer to the BASE class, but it really points to a child object.
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

    // ---- Summary stats (0-100) shown in the "current car stats" box ----
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

    // Used to build AI cars: set all four levels at once
    void setLevels(int e, int pu, int fw, int rw) {
        engine.setLevel(e);
        powerUnit.setLevel(pu);
        frontWing.setLevel(fw);
        rearWing.setLevel(rw);
    }
};

#endif
