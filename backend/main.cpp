// ============================================================
// main.cpp
// Starts a small web server (using the single-file library
// httplib.h) and connects the web page to the Game class.
//
// The browser sends a request like  GET /team  or
// POST /upgrade?part=0  and we answer with JSON text.
// We build the JSON by hand with strings, so no JSON library is needed.
// ============================================================
#include "httplib.h"   // must stay FIRST (it includes the networking headers)

#include <iostream>
#include <string>
#include <cstdlib>     // srand, atoi
#include <ctime>       // time
#include "Game.h"
using namespace std;

// ------------------------------------------------------------
// Small helper functions for building JSON text
// ------------------------------------------------------------

// Turns a number into text
string num(double n) {
    // whole numbers print without decimals, others with 1 decimal
    if (n == (int)n) return to_string((int)n);
    string s = to_string(n);
    return s.substr(0, s.find('.') + 2);
}

// Puts a string in quotes and escapes " and \ so the JSON stays valid
string str(const string& s) {
    string out = "\"";
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == '"' || s[i] == '\\') out += '\\';
        if (s[i] == '\n') continue;
        out += s[i];
    }
    return out + "\"";
}

// true / false text
string jsonBool(bool b) { return b ? "true" : "false"; }

// Reads a number from the request, e.g. /upgrade?part=2  ->  getInt(req, "part") == 2
int getInt(const httplib::Request& req, const string& key) {
    if (!req.has_param(key)) return -1;
    return atoi(req.get_param_value(key).c_str());
}

// ------------------------------------------------------------
// Functions that turn game objects into JSON text
// ------------------------------------------------------------

string driverJson(const Driver& d, int id, const Game& g) {
    return "{\"id\":" + to_string(id)
         + ",\"name\":" + str(d.getName())
         + ",\"price\":" + to_string(d.getPrice())
         + ",\"pace\":" + to_string(d.getPace())
         + ",\"consistency\":" + to_string(d.getConsistency())
         + ",\"aggression\":" + to_string(d.getAggression())
         + ",\"tyreManagement\":" + to_string(d.getTyreManagement())
         + ",\"wetSkill\":" + to_string(d.getWetSkill())
         + ",\"rating\":" + num(d.getRating())
         + ",\"equipped\":" + jsonBool(d.getName() == g.player.driver.getName())
         + ",\"affordable\":" + jsonBool(g.player.canAfford(d.getPrice())) + "}";
}

string mechanicJson(const MechanicCrew& m, int id, const Game& g) {
    return "{\"id\":" + to_string(id)
         + ",\"name\":" + str(m.getName())
         + ",\"price\":" + to_string(m.getPrice())
         + ",\"reliability\":" + to_string(m.getReliability())
         + ",\"setupSkill\":" + to_string(m.getSetupSkill())
         + ",\"equipped\":" + jsonBool(m.getName() == g.player.mechanics.getName())
         + ",\"affordable\":" + jsonBool(g.player.canAfford(m.getPrice())) + "}";
}

string pitCrewJson(const PitCrew& p, int id, const Game& g) {
    return "{\"id\":" + to_string(id)
         + ",\"name\":" + str(p.getName())
         + ",\"price\":" + to_string(p.getPrice())
         + ",\"speed\":" + to_string(p.getSpeed())
         + ",\"consistency\":" + to_string(p.getConsistency())
         + ",\"equipped\":" + jsonBool(p.getName() == g.player.pitCrew.getName())
         + ",\"affordable\":" + jsonBool(g.player.canAfford(p.getPrice())) + "}";
}

// The whole player team: money, driver, crews, car parts and car stats
string teamJson(const Game& g) {
    const Team& t = g.player;

    string parts = "[";
    for (int i = 0; i < 4; i++) {
        const CarPart* p = t.car.getPart(i);
        if (i > 0) parts += ",";
        parts += "{\"index\":" + to_string(i)
               + ",\"name\":" + str(p->getName())
               + ",\"description\":" + str(p->getDescription())
               + ",\"level\":" + to_string(p->getLevel())
               + ",\"maxLevel\":" + to_string(MAX_PART_LEVEL)
               + ",\"maxed\":" + jsonBool(p->isMaxed())
               + ",\"upgradeCost\":" + to_string(p->getUpgradeCost())
               + ",\"affordable\":" + jsonBool(t.canAfford(p->getUpgradeCost())) + "}";
    }
    parts += "]";

    // Find the equipped items' ids so the page can mark them
    return "{\"name\":" + str(t.name)
         + ",\"budget\":" + to_string(t.getBudget())
         + ",\"raceNumber\":" + to_string(g.raceNumber)
         + ",\"driver\":" + driverJson(t.driver, -1, g)
         + ",\"mechanics\":" + mechanicJson(t.mechanics, -1, g)
         + ",\"pitCrew\":" + pitCrewJson(t.pitCrew, -1, g)
         + ",\"parts\":" + parts
         + ",\"carStats\":{\"power\":" + to_string(t.car.getPowerStat())
         + ",\"downforce\":" + to_string(t.car.getDownforceStat())
         + ",\"reliability\":" + to_string(t.car.getReliabilityStat())
         + ",\"efficiency\":" + to_string(t.car.getEfficiencyStat()) + "}}";
}

// The list of circuits
string tracksJson(const Game& g) {
    string out = "{\"tracks\":[";
    for (int i = 0; i < (int)g.circuits.size(); i++) {
        const Circuit& c = g.circuits[i];
        if (i > 0) out += ",";
        out += "{\"id\":" + to_string(i)
             + ",\"name\":" + str(c.name)
             + ",\"country\":" + str(c.country)
             + ",\"description\":" + str(c.description)
             + ",\"laps\":" + to_string(c.laps)
             + ",\"lengthKm\":" + num(c.lengthKm)
             + ",\"airTemp\":" + to_string(c.airTemp)
             + ",\"trackTemp\":" + to_string(c.trackTemp)
             + ",\"humidity\":" + to_string(c.humidity)
             + ",\"abrasiveness\":" + to_string(c.abrasiveness)
             + ",\"downforceDemand\":" + to_string(c.downforceDemand)
             + ",\"powerDemand\":" + to_string(c.powerDemand)
             + ",\"rainChance\":" + to_string(c.rainChance) + "}";
    }
    return out + "]}";
}

// The 5 tyres (stats only, no track-fit score)
string tyresJson(const Game& g, int trackIndex) {
    const Circuit& c = g.circuits[trackIndex];
    string out = "{\"tyres\":[";
    for (int i = 0; i < (int)g.tyres.size(); i++) {
        const Tyre& t = g.tyres[i];
        if (i > 0) out += ",";
        out += "{\"id\":" + to_string(i)
             + ",\"name\":" + str(t.getName())
             + ",\"dryGrip\":" + to_string(t.getDryGrip())
             + ",\"wetGrip\":" + to_string(t.getWetGrip())
             + ",\"wear\":" + to_string(t.getWear())
             + ",\"tempMin\":" + to_string(t.getTempMin())
             + ",\"tempMax\":" + to_string(t.getTempMax())
             + ",\"pitStops\":" + to_string(t.getPitStops(c)) + "}";
    }
    return out + "]}";
}

// The race results table
string resultsJson(const vector<RaceResult>& results) {
    string out = "[";
    for (int i = 0; i < (int)results.size(); i++) {
        const RaceResult& r = results[i];
        if (i > 0) out += ",";
        out += "{\"position\":" + to_string(r.position)
             + ",\"team\":" + str(r.teamName)
             + ",\"driver\":" + str(r.driverName)
             + ",\"tyre\":" + str(r.tyreName)
             + ",\"pitStops\":" + to_string(r.pitStops)
             + ",\"dnf\":" + jsonBool(r.dnf)
             + ",\"score\":" + num(r.score)
             + ",\"prize\":" + to_string(r.prize)
             + ",\"isPlayer\":" + jsonBool(r.isPlayer) + "}";
    }
    return out + "]";
}

// Reply used by every action (buy, upgrade, ...): ok + message + updated team
string actionJson(const string& error, const Game& g) {
    bool ok = (error == "");
    return "{\"ok\":" + jsonBool(ok)
         + ",\"message\":" + str(ok ? "Done!" : error)
         + ",\"team\":" + teamJson(g) + "}";
}

// ------------------------------------------------------------
// main: create the game and register the web addresses (endpoints)
// ------------------------------------------------------------
int main() {
    srand((unsigned)time(NULL));   // seed the random number generator

    Game game;                     // the whole game lives in this one object
    httplib::Server svr;

    // Handle only one request at a time, so we never need locks (mutexes).
    svr.new_task_queue = [] { return new httplib::ThreadPool(1); };

    // Allow the page to call us even if opened as a plain file
    svr.set_default_headers({{"Access-Control-Allow-Origin", "*"}});

    // Serve the website files (index.html, style.css, script.js) from ../frontend
    if (!svr.set_mount_point("/", "../frontend")) {
        cout << "Warning: could not find the ../frontend folder." << endl;
    }

    // ---- GET /team : everything about the player's team ----
    svr.Get("/team", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_content(teamJson(game), "application/json");
    });

    // ---- GET /market?type=driver|mechanic|pit : the cards for one market tab ----
    svr.Get("/market", [&](const httplib::Request& req, httplib::Response& res) {
        string type = req.get_param_value("type");
        string out = "{\"type\":" + str(type) + ",\"budget\":" + to_string(game.player.getBudget()) + ",\"items\":[";

        if (type == "driver") {
            for (int i = 0; i < NUM_DRIVERS; i++) {
                if (i > 0) out += ",";
                out += driverJson(game.driverShop[i], i, game);
            }
        } else if (type == "mechanic") {
            for (int i = 0; i < NUM_MECHANICS; i++) {
                if (i > 0) out += ",";
                out += mechanicJson(game.mechanicShop[i], i, game);
            }
        } else if (type == "pit") {
            for (int i = 0; i < NUM_PIT_CREWS; i++) {
                if (i > 0) out += ",";
                out += pitCrewJson(game.pitCrewShop[i], i, game);
            }
        }
        out += "]}";
        res.set_content(out, "application/json");
    });

    // ---- POST /buy?type=driver|mechanic|pit&id=N ----
    svr.Post("/buy", [&](const httplib::Request& req, httplib::Response& res) {
        string type = req.get_param_value("type");
        int id = getInt(req, "id");
        string error;
        if (type == "driver") error = game.buyDriver(id);
        else if (type == "mechanic") error = game.buyMechanic(id);
        else if (type == "pit") error = game.buyPitCrew(id);
        else error = "Unknown market type.";
        res.set_content(actionJson(error, game), "application/json");
    });

    // ---- POST /upgrade?part=0..3 ----
    svr.Post("/upgrade", [&](const httplib::Request& req, httplib::Response& res) {
        string error = game.upgradePart(getInt(req, "part"));
        res.set_content(actionJson(error, game), "application/json");
    });

    // ---- POST /set-name?name=... ----
    svr.Post("/set-name", [&](const httplib::Request& req, httplib::Response& res) {
        string error = game.setTeamName(req.get_param_value("name"));
        res.set_content(actionJson(error, game), "application/json");
    });

    // ---- GET /tracks : the 5 circuits ----
    svr.Get("/tracks", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_content(tracksJson(game), "application/json");
    });

    // ---- GET /tyres?track=N : the 5 tyres with a fit hint for that track ----
    svr.Get("/tyres", [&](const httplib::Request& req, httplib::Response& res) {
        int track = getInt(req, "track");
        if (track < 0 || track >= (int)game.circuits.size()) {
            res.set_content("{\"ok\":false,\"message\":\"Unknown track.\"}", "application/json");
            return;
        }
        res.set_content(tyresJson(game, track), "application/json");
    });

    // ---- POST /race?track=N&tyre=N : run the race and return the results ----
    svr.Post("/race", [&](const httplib::Request& req, httplib::Response& res) {
        vector<RaceResult> results;
        string error = game.runRace(getInt(req, "track"), getInt(req, "tyre"), results);
        if (error != "") {
            res.set_content(actionJson(error, game), "application/json");
            return;
        }
        // Find the player's row so the page can show "you earned ..."
        int playerPosition = 0, playerPrize = 0;
        for (int i = 0; i < (int)results.size(); i++) {
            if (results[i].isPlayer) {
                playerPosition = results[i].position;
                playerPrize = results[i].prize;
            }
        }
        string out = "{\"ok\":true,\"message\":\"Race finished!\""
                   + string(",\"playerPosition\":") + to_string(playerPosition)
                   + ",\"playerPrize\":" + to_string(playerPrize)
                   + ",\"results\":" + resultsJson(results)
                   + ",\"team\":" + teamJson(game) + "}";
        res.set_content(out, "application/json");
    });

    // ---- POST /new-game : start over ----
    svr.Post("/new-game", [&](const httplib::Request& req, httplib::Response& res) {
        game.newGame();
        res.set_content(actionJson("", game), "application/json");
    });

    cout << "F1 Team Simulator running!" << endl;
    cout << "Open http://localhost:8080 in your browser. Press Ctrl+C to stop." << endl;
    svr.listen("127.0.0.1", 8080);
    return 0;
}
