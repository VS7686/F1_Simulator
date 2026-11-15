// ============================================================
// Race.h
// The race is NOT played lap by lap. Instead, simulate() gives
// every team a single "race score" and then sorts the teams by it.
// Highest score = 1st place.
//
// All the numbers that control the result are the named
// constants below, so they are easy to find and tune.
// ============================================================
#ifndef RACE_H
#define RACE_H

#include <string>
#include <vector>
#include <cstdlib>    // rand()
#include <algorithm>  // sort()
#include "Team.h"
#include "Tyre.h"
#include "Circuit.h"
using namespace std;

// ---------- Tuning numbers (change these to rebalance the game) ----------
const double WEIGHT_DRIVER = 0.35;     // how much the driver counts in the score
const double WEIGHT_CAR    = 0.35;     // how much the car counts
const double WEIGHT_TYRE   = 0.30;     // how much the tyre choice counts
const double SETUP_BONUS_MAX = 5.0;    // best possible bonus from the mechanic crew
const double NOISE_BASE      = 3.0;    // size of the random luck (+/- points)
const double BASE_DNF_CHANCE = 12.0;   // chance (in %) of breaking down, before reductions
const int    AI_BEST_TYRE_CHANCE = 60; // % of the time an AI team picks the best-fitting tyre

// Prize money by finishing position (index 0 = 1st place ... index 10 = 11th)
const int PRIZE_MONEY[11] = {
    200000, 160000, 130000, 105000, 85000, 65000,
    50000, 35000, 25000, 15000, 10000
};

// ---------- One row of the results table ----------
struct RaceResult {
    int position;          // 1 = winner
    string teamName;
    string driverName;
    string tyreName;
    int pitStops;
    bool dnf;              // true = did not finish (broke down)
    double score;          // the race score (higher is better)
    int prize;             // prize money won
    bool isPlayer;         // true for the player's row
};

class Race {
private:
    // Random number between -1.0 and +1.0
    static double randomPlusMinusOne() {
        return (rand() % 2001 - 1000) / 1000.0;
    }

    // AI teams choose a tyre with some randomness:
    // most of the time they pick the best-fitting tyre, otherwise a random one.
    static int chooseAiTyre(const vector<Tyre>& tyres, const Circuit& c) {
        if (rand() % 100 < AI_BEST_TYRE_CHANCE) {
            // find the tyre with the highest fit
            int best = 0;
            for (int i = 1; i < (int)tyres.size(); i++) {
                if (tyres[i].getTrackFit(c) > tyres[best].getTrackFit(c)) best = i;
            }
            return best;
        }
        return rand() % tyres.size();   // random tyre
    }

    // Used by sort(): finishers come before DNFs, then higher score first
    static bool isBetter(const RaceResult& a, const RaceResult& b) {
        if (a.dnf != b.dnf) return !a.dnf;   // the one that did NOT crash is better
        return a.score > b.score;
    }

public:
    // Runs the whole race and returns the results table (sorted, 1st place first).
    // 'teams' holds pointers to all 11 teams (the player and the 10 AI teams).
    static vector<RaceResult> simulate(vector<Team*>& teams, const Circuit& c) {
        vector<Tyre> tyres = Tyre::createAll();
        vector<RaceResult> results;

        for (int i = 0; i < (int)teams.size(); i++) {
            Team* t = teams[i];

            // 1. AI teams pick their tyre (the player already chose theirs)
            if (!t->isPlayer) {
                t->tyreChoice = chooseAiTyre(tyres, c);
            }
            const Tyre& tyre = tyres[t->tyreChoice];

            // 2. The three main factors, each roughly 0-100
            double driverFactor = t->driver.getRating();
            double carFactor = t->car.getOverallRating(c);
            // A driver with good tyre management gets more out of the tyres
            double tyreFactor = tyre.getTrackFit(c)
                              * (0.8 + 0.2 * t->driver.getTyreManagement() / 100.0);

            double score = WEIGHT_DRIVER * driverFactor
                         + WEIGHT_CAR * carFactor
                         + WEIGHT_TYRE * tyreFactor;

            // 3. Mechanic crew gives a small bonus for a well set-up car
            score += t->mechanics.getSetupSkill() / 100.0 * SETUP_BONUS_MAX;

            // 4. Pit stops cost points. The pit crew decides how many per stop.
            int stops = tyre.getPitStops(c);
            score -= stops * t->pitCrew.getTimeLostPerStop();

            // 5. Rain: a driver with good wet skill gains in the rain,
            //    a bad one loses. (wetSkill 50 = neutral)
            double rain = c.rainChance / 100.0;
            score += (t->driver.getWetSkill() - 50) / 50.0 * 4.0 * rain;

            // 6. Random luck. Consistent drivers have less luck (good or bad).
            double noiseSize = NOISE_BASE * (1.5 - t->driver.getConsistency() / 100.0);
            score += randomPlusMinusOne() * noiseSize;

            // 7. Breakdown chance (in percent).
            //    Reliable mechanics + reliable car lower it, an aggressive driver raises it.
            double dnfChance = BASE_DNF_CHANCE
                             - t->mechanics.getReliability() * 0.06
                             - t->car.getReliabilityStat() * 0.04
                             + t->driver.getAggression() * 0.04;
            if (dnfChance < 1.0) dnfChance = 1.0;
            bool dnf = (rand() % 1000) < dnfChance * 10;

            // 8. Fill in this team's row
            RaceResult r;
            r.position = 0;                 // set after sorting
            r.teamName = t->name;
            r.driverName = t->driver.getName();
            r.tyreName = tyre.getName();
            r.pitStops = stops;
            r.dnf = dnf;
            r.score = score;
            r.prize = 0;                    // set after sorting
            r.isPlayer = t->isPlayer;
            results.push_back(r);
        }

        // Sort: best first, DNFs at the back
        sort(results.begin(), results.end(), isBetter);

        // Give out positions and prize money
        for (int i = 0; i < (int)results.size(); i++) {
            results[i].position = i + 1;
            // A team that broke down gets no prize money
            if (!results[i].dnf && i < 11) results[i].prize = PRIZE_MONEY[i];
        }
        return results;
    }
};

#endif
