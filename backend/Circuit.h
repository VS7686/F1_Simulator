
#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <string>
#include <vector>
using namespace std;

class Circuit {
public:
    string name;          
    string country;       
    string description;   

    int    laps;          
    double lengthKm;      

    int airTemp;          
    int trackTemp;       
    int humidity;         

    int abrasiveness;     // how hard the track is on tyres (0-100)
    int downforceDemand;  // how much the track needs wings/downforce (0-100)
    int powerDemand;      // how much the track needs engine power (0-100)
    int rainChance;       // chance of rain in percent (0-100)

    //just stores all the values
    Circuit(string n, string c, string desc, int l, double len,
            int air, int track, int hum,
            int abr, int down, int pow, int rain) {
        name = n;
        country = c;
        description = desc;
        laps = l;
        lengthKm = len;
        airTemp = air;
        trackTemp = track;
        humidity = hum;
        abrasiveness = abr;
        downforceDemand = down;
        powerDemand = pow;
        rainChance = rain;
    }

    // Builds the list of the 5 circuits used in the game.
    // (static = we can call Circuit::createAll() without making an object)
    static vector<Circuit> createAll() {
        vector<Circuit> list;
        //                 name          country    description                                      laps length air track hum abr down pow rain
        list.push_back(Circuit("Monza",       "Italy",     "Long straights. Engine power matters most.",     53, 5.79, 28, 40, 45, 40,  20, 95,  5));
        list.push_back(Circuit("Monaco",      "Monaco",    "Tight streets. Wings and downforce matter most.", 78, 3.34, 22, 32, 60, 25, 100, 15, 10));
        list.push_back(Circuit("Silverstone", "UK",        "Fast corners that destroy tyres. Cool weather.",  52, 5.89, 18, 26, 65, 85,  70, 60, 30));
        list.push_back(Circuit("Singapore",   "Singapore", "Hot, humid night race. Lots of downforce.",       62, 4.94, 31, 38, 85, 60,  85, 40, 20));
        list.push_back(Circuit("Spa",         "Belgium",   "Long lap, cold and rainy. Wet tyres likely.",     44, 7.00, 14, 18, 80, 50,  50, 75, 60));
        return list;
    }
};

#endif
