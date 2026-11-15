F1 TEAM SIMULATOR - BACKEND (C++)

HOW TO COMPILE (run these inside the backend folder)

  Windows (MinGW g++):
    g++ -std=c++17 main.cpp -o f1server.exe -lws2_32 -lwsock32 -D_WIN32_WINNT=0x0A00
    f1server.exe

  Linux / macOS / WSL:
    g++ -std=c++17 -pthread main.cpp -o f1server
    ./f1server

Then open http://localhost:8080 in your browser.
Always start the server from inside the backend folder (it serves ../frontend).

FILES
  main.cpp        web server + API endpoints (builds JSON by hand)
  Game.h          owns everything: player, 10 AI teams, market, buying, racing
  Race.h          simulate(): one score per team, sorted = finishing order
                  (all tuning numbers are constants at the top)
  Team.h          a team = driver + car + mechanic crew + pit crew + money
  Car.h           composition: a car has 4 parts
  CarPart.h       abstract base + Engine, PowerUnit, FrontWing, RearWing
  Driver.h  MechanicCrew.h  PitCrew.h  Tyre.h  Circuit.h   data classes
  httplib.h       third-party single-file web server library (cpp-httplib 0.18.0)

API
  GET  /team                      player's team, money, car parts, car stats
  GET  /market?type=driver|mechanic|pit
  POST /buy?type=driver|mechanic|pit&id=N
  POST /upgrade?part=0..3         0 Engine, 1 Power Unit, 2 Front Wing, 3 Rear Wing
  POST /set-name?name=...
  GET  /tracks
  GET  /tyres?track=N             5 tyres with a fit hint for that track
  POST /race?track=N&tyre=N       runs the race, returns results + new team state
  POST /new-game
