#ifndef TYRE_H
#define TYRE_H

#include <string>
#include <vector>
#include "Circuit.h"
using namespace std;

// enum = a named list of choices. SOFT = 0, MEDIUM = 1, and so on.
enum Compound { SOFT, MEDIUM, HARD, INTERMEDIATE, WET };

class Tyre {
private:
    Compound compound;
    string name;
    int dryGrip;   // grip on a DRY track when the temperature is perfect (0-100)
    int wetGrip;   // grip on a WET track (0-100)
    int wear;      // how fast it wears out (higher = more pit stops)
    int tempMin;   // best track temperature range starts here (degrees C)
    int tempMax;   // and ends here

public:
    Tyre(Compound c, string n, int dg, int wg, int w, int tmin, int tmax) {
        compound = c;
        name = n;
        dryGrip = dg;
        wetGrip = wg;
        wear = w;
        tempMin = tmin;
        tempMax = tmax;
    }

    Compound getCompound() const { return compound; }
    string getName() const { return name; }
    int getDryGrip() const { return dryGrip; }
    int getWetGrip() const { return wetGrip; }
    int getWear() const { return wear; }
    int getTempMin() const { return tempMin; }
    int getTempMax() const { return tempMax; }

    // How well this tyre suits the track, from 0 to 100.
    double getTrackFit(const Circuit& c) const {
        // Step 1: temperature score. 100 inside the best range,
        // and we lose 4 points for every degree outside of it.
        double tempScore = 100;
        if (c.trackTemp < tempMin) tempScore = 100 - 4.0 * (tempMin - c.trackTemp);
        if (c.trackTemp > tempMax) tempScore = 100 - 4.0 * (c.trackTemp - tempMax);
        if (tempScore < 0) tempScore = 0;

        // Step 2: grip if the track were dry (temperature lowers it)
        double dryFit = dryGrip * tempScore / 100.0;

        // Step 3: mix the dry grip and wet grip using the rain chance.
        // 0% rain -> only dry grip counts, 100% rain -> only wet grip counts.
        double rain = c.rainChance / 100.0;
        return (1.0 - rain) * dryFit + rain * wetGrip;
    }

    // How many pit stops this tyre needs over the whole race.
    int getPitStops(const Circuit& c) const {
        // How many laps one set of tyres survives
        int lapsPerSet = 130000 / (wear * c.abrasiveness);
        if (lapsPerSet < 1) lapsPerSet = 1;
        // Number of sets needed minus 1 = number of stops
        int sets = (c.laps + lapsPerSet - 1) / lapsPerSet;   // rounds up
        return sets - 1;
    }

    // A short text hint for the player, e.g. "Great fit" or "Risky - track too hot"
    string getHint(const Circuit& c) const {
        double fit = getTrackFit(c);
        string label;
        if (fit >= 75) label = "Great fit";
        else if (fit >= 55) label = "Good fit";
        else if (fit >= 35) label = "Risky";
        else label = "Poor fit";

        // Add the main reason when it is not a great fit
        if (fit < 75) {
            if (c.rainChance >= 40 && compound != INTERMEDIATE && compound != WET)
                label += " - rain likely";
            else if (c.rainChance < 40 && (compound == INTERMEDIATE || compound == WET))
                label += " - track is mostly dry";
            else if (c.trackTemp < tempMin) label += " - track too cold";
            else if (c.trackTemp > tempMax) label += " - track too hot";
        }
        return label;
    }

    // Builds the list of the 5 compounds, in the order of the Compound enum.
    static vector<Tyre> createAll() {
        vector<Tyre> list;
        //                      compound      name            dry wet wear tMin tMax
        list.push_back(Tyre(SOFT,         "Soft",         100, 10, 100, 20, 35));
        list.push_back(Tyre(MEDIUM,       "Medium",        90, 10,  60, 28, 42));
        list.push_back(Tyre(HARD,         "Hard",          80, 10,  35, 38, 55));
        list.push_back(Tyre(INTERMEDIATE, "Intermediate",  40, 70,  70, 10, 30));
        list.push_back(Tyre(WET,          "Wet",           25, 90,  60, 10, 25));
        return list;
    }
};

#endif
