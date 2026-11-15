// ============================================================
// script.js
// All the page logic. It talks to the C++ server with fetch().
// The page is ONE html file; "screens" are <section> blocks that
// we show/hide with show('home'), show('garage'), and so on.
//
// Sections in this file:
//   1. Setup + small helpers
//   2. Drawing: pixel car, tyre, track outlines
//   3. One render function per screen
//   4. Race animation
//   5. Start-up
// ============================================================
'use strict';

// ------------------------------------------------------------
// 1. SETUP + HELPERS
// ------------------------------------------------------------

// If the page is opened as a plain file, talk to the local server.
// If the server itself serves the page, a blank prefix works.
const API = (location.protocol === 'file:') ? 'http://localhost:8080' : '';

const $ = id => document.getElementById(id);                       // shortcut for getElementById
const money = n => '$' + Number(n).toLocaleString('en-US');        // 300000 -> $300,000

// Make text safe before putting it inside innerHTML (team names are typed by the player)
function esc(s) {
  return String(s).replace(/[&<>"']/g, c =>
    ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
}

// ---- Game state kept in the browser ----
let team = null;            // latest team info from the server
let tracks = [];            // the 5 circuits
let selectedTrack = null;   // circuit chosen for the next race
let selectedTyre = null;    // tyre index chosen for the next race
let marketTab = 'driver';   // which market tab is open
let raceData = null;        // result of the last race (from the server)
let anim = null;            // running animation (null when none)

// Ask the server for something. Returns the JSON, or null if the server is down.
async function api(path, method) {
  try {
    const res = await fetch(API + path, { method: method || 'GET' });
    const data = await res.json();
    $('offline').hidden = true;
    return data;
  } catch (e) {
    $('offline').hidden = false;
    return null;
  }
}

// Update our copy of the team and the money shown in the top bar
function setTeam(t) {
  team = t;
  $('money').textContent = money(t.budget);
}

async function refreshTeam() {
  const t = await api('/team');
  if (t) setTeam(t);
}

// Small pop-up message at the bottom
let toastTimer = null;
function toast(message, ok) {
  const el = $('toast');
  el.textContent = message;
  el.className = 'toast' + (ok ? '' : ' bad');
  el.hidden = false;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => { el.hidden = true; }, 2500);
}

// Handles the reply of buy / upgrade / set-name. Returns true if it worked.
function handleAction(reply, successText) {
  if (!reply) return false;
  if (reply.team) setTeam(reply.team);
  toast(reply.ok ? successText : reply.message, reply.ok);
  return reply.ok;
}

// A row of 10 blocks showing a stat from 0 to 100
function statBar(label, value) {
  const filled = Math.round(value / 10);
  const cls = value < 40 ? 'lo' : (value < 70 ? 'mid' : 'hi');
  let blocks = '';
  for (let i = 0; i < 10; i++) blocks += `<i class="${i < filled ? 'on ' + cls : ''}"></i>`;
  return `<div class="stat"><span class="lbl">${label}</span><span class="bar">${blocks}</span><span class="val">${value}</span></div>`;
}

function driverStats(d) {
  return statBar('PACE', d.pace) + statBar('CONSISTENCY', d.consistency) +
         statBar('AGGRESSION', d.aggression) + statBar('TYRE MGMT', d.tyreManagement) +
         statBar('WET SKILL', d.wetSkill);
}
function mechanicStats(m) { return statBar('RELIABILITY', m.reliability) + statBar('SETUP', m.setupSkill); }
function pitStats(p)      { return statBar('SPEED', p.speed) + statBar('CONSISTENCY', p.consistency); }
function carStats(c) {
  return statBar('POWER', c.power) + statBar('DOWNFORCE', c.downforce) +
         statBar('RELIABILITY', c.reliability) + statBar('EFFICIENCY', c.efficiency);
}

// ------------------------------------------------------------
// 2. DRAWING (everything is made of square pixels)
// ------------------------------------------------------------

// Make a colour darker (used for the shaded part of the car body)
function darken(hex, amount) {
  const n = parseInt(hex.slice(1), 16);
  const r = Math.max(0, (n >> 16) - amount);
  const g = Math.max(0, ((n >> 8) & 255) - amount);
  const b = Math.max(0, (n & 255) - amount);
  return '#' + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
}

// Draws a filled pixel-circle (used for wheels and tyres)
function pixelCircle(ctx, s, cx, cy, radius, color) {
  ctx.fillStyle = color;
  for (let y = -radius; y <= radius; y++) {
    for (let x = -radius; x <= radius; x++) {
      if (x * x + y * y <= radius * radius + 1) ctx.fillRect((cx + x) * s, (cy + y) * s, s, s);
    }
  }
}

// The F1 car sprite: a 40 x 16 pixel picture facing RIGHT, drawn from rectangles.
// 'color' is the team colour, 's' is the zoom (each sprite pixel becomes s x s screen pixels).
const spriteCache = {};
function carSprite(color, s) {
  const key = color + s;
  if (spriteCache[key]) return spriteCache[key];

  const c = document.createElement('canvas');
  c.width = 40 * s;
  c.height = 16 * s;
  const ctx = c.getContext('2d');
  ctx.imageSmoothingEnabled = false;
  const dark = darken(color, 70);
  const K = '#0b0b0d';        // black

  // r(x, y, w, h, colour) = draw a rectangle in sprite pixels
  const r = (x, y, w, h, col) => { ctx.fillStyle = col; ctx.fillRect(x * s, y * s, w * s, h * s); };

  // rear wing
  r(1, 2, 2, 9, dark);        // end plate
  r(0, 2, 8, 2, color);       // wing plane
  r(4, 4, 2, 7, K);           // wing support
  // floor
  r(6, 12, 28, 2, K);
  // engine cover + sidepod
  r(7, 7, 18, 5, color);
  r(7, 10, 18, 2, dark);
  r(13, 8, 9, 1, '#f2efe6');  // white stripe
  // air intake above the driver
  r(12, 4, 8, 3, color);
  r(18, 5, 2, 2, K);
  // cockpit, helmet, halo
  r(23, 6, 5, 2, K);
  r(24, 5, 2, 2, '#ffd400');
  r(23, 4, 5, 1, K);
  r(27, 5, 1, 3, K);
  // front of the car + nose
  r(24, 8, 6, 4, color);
  r(29, 9, 8, 3, color);
  r(36, 10, 3, 2, dark);
  // front wing
  r(34, 13, 6, 1, color);
  r(34, 14, 6, 1, K);
  r(39, 11, 1, 4, K);
  // wheels (round pixel circles: black tyre, grey rim, black hub)
  pixelCircle(ctx, s, 11, 11, 4, K);  pixelCircle(ctx, s, 11, 11, 2, '#9a9aa5');  pixelCircle(ctx, s, 11, 11, 0, K);
  pixelCircle(ctx, s, 31, 12, 3, K);  pixelCircle(ctx, s, 31, 12, 1, '#9a9aa5');

  spriteCache[key] = c;
  return c;
}

// Draw the car sprite onto a canvas that is already in the page
function drawCarOn(canvasId, color, scale) {
  const sprite = carSprite(color, scale);
  const cv = $(canvasId);
  cv.width = sprite.width;
  cv.height = sprite.height;
  const ctx = cv.getContext('2d');
  ctx.imageSmoothingEnabled = false;
  ctx.clearRect(0, 0, cv.width, cv.height);
  ctx.drawImage(sprite, 0, 0);
}

// A tyre icon: coloured sidewall ring, black rubber, grey hub
const TYRE_COLORS = ['#e10600', '#ffd400', '#f2efe6', '#3ddc4a', '#3a8dff'];  // Soft, Medium, Hard, Inter, Wet
function tyreIcon(index) {
  const c = document.createElement('canvas');
  c.width = c.height = 64;
  const ctx = c.getContext('2d');
  pixelCircle(ctx, 4, 8, 8, 7, '#000');
  pixelCircle(ctx, 4, 8, 8, 6, TYRE_COLORS[index]);
  pixelCircle(ctx, 4, 8, 8, 4, '#0b0b0d');
  pixelCircle(ctx, 4, 8, 8, 2, '#9a9aa5');
  c.style.width = '56px';
  c.style.height = '56px';
  return c;
}

// Track outlines: each track is a list of corner points on a 32 x 20 grid.
// We connect the points with square pixels (a simple line algorithm).
const TRACK_SHAPES = {
  'Monza':       [[4,14],[4,6],[9,3],[24,3],[28,6],[28,10],[22,10],[22,13],[26,13],[26,17],[10,17]],
  'Monaco':      [[3,16],[3,10],[8,10],[8,5],[14,5],[14,9],[20,9],[20,3],[28,3],[28,12],[22,12],[22,17],[10,17],[10,14],[3,16]],
  'Silverstone': [[5,14],[3,8],[8,3],[15,5],[22,3],[28,7],[26,12],[20,10],[18,15],[10,17]],
  'Singapore':   [[3,17],[3,11],[9,11],[9,5],[17,5],[17,10],[23,10],[23,3],[29,3],[29,17]],
  'Spa':         [[3,15],[6,7],[12,3],[18,5],[25,3],[29,9],[22,12],[24,17],[14,15],[8,17]]
};

function drawTrack(canvas, name) {
  const s = 9;                         // zoom
  canvas.width = 32 * s;
  canvas.height = 20 * s;
  const ctx = canvas.getContext('2d');
  ctx.imageSmoothingEnabled = false;
  const pts = TRACK_SHAPES[name] || TRACK_SHAPES['Monza'];

  // draw a short line between two points, one square pixel at a time
  const line = (x0, y0, x1, y1, col) => {
    ctx.fillStyle = col;
    const steps = Math.max(Math.abs(x1 - x0), Math.abs(y1 - y0));
    for (let i = 0; i <= steps; i++) {
      const x = Math.round(x0 + (x1 - x0) * (steps ? i / steps : 0));
      const y = Math.round(y0 + (y1 - y0) * (steps ? i / steps : 0));
      ctx.fillRect(x * s, y * s, s, s);
    }
  };
  // outer edge in dark grey, then the white racing line on top
  for (let i = 0; i < pts.length; i++) {
    const a = pts[i], b = pts[(i + 1) % pts.length];
    line(a[0], a[1], b[0], b[1], '#55555d');
  }
  for (let i = 0; i < pts.length; i++) {
    const a = pts[i], b = pts[(i + 1) % pts.length];
    line(a[0], a[1], b[0], b[1], '#f2efe6');
  }
  // red start/finish square on the first point
  ctx.fillStyle = '#e10600';
  ctx.fillRect(pts[0][0] * s - s / 2, pts[0][1] * s - s / 2, s * 2, s * 2);
}

// The pencil icon for the "edit team name" button (8 x 8 pixels)
function pencilSvg() {
  let rects = '';
  for (let i = 0; i < 8; i++) {
    const col = i === 0 ? '#ff9aa8' : (i < 6 ? '#e10600' : (i === 6 ? '#ffd400' : '#f2efe6'));
    rects += `<rect x="${7 - i}" y="${i}" width="1" height="1" fill="${col}"/>`;
    if (i < 7) rects += `<rect x="${6 - i}" y="${i}" width="1" height="1" fill="${col}"/>`;
  }
  return `<svg viewBox="0 0 8 8" width="24" height="24" shape-rendering="crispEdges">${rects}</svg>`;
}

// ------------------------------------------------------------
// 3. SCREENS
// ------------------------------------------------------------

// Show one screen and hide the others, then fill it with fresh data
async function show(name) {
  stopAnimation();
  document.querySelectorAll('.screen').forEach(s => { s.hidden = (s.id !== 'screen-' + name); });
  $('homeBtn').disabled = (name === 'race');   // can't leave in the middle of a race
  window.scrollTo(0, 0);

  if (name === 'home') await renderHome();
  if (name === 'garage') await renderGarage();
  if (name === 'market') await renderMarket();
  if (name === 'tracks') await renderTracks();
  if (name === 'tyres') await renderTyres();
  if (name === 'race') startRaceAnimation();
  if (name === 'results') renderResults();
}

// ---------- HOME ----------
function renderTeamName(editing) {
  const box = $('teamNameBox');
  if (!editing) {
    box.innerHTML = `<span class="teamname">${esc(team.name)}</span>
      <button class="icon-btn" id="editName" title="Edit team name" aria-label="Edit team name">${pencilSvg()}</button>`;
    $('editName').onclick = () => renderTeamName(true);
  } else {
    box.innerHTML = `<input id="nameInput" maxlength="20" value="${esc(team.name)}" aria-label="Team name">
      <button class="btn small" id="saveName">SAVE</button>`;
    const input = $('nameInput');
    input.focus();
    input.select();
    const save = async () => {
      const value = input.value.trim();
      if (value && value !== team.name) {
        handleAction(await api('/set-name?name=' + encodeURIComponent(value), 'POST'), 'TEAM NAME SAVED');
      }
      renderTeamName(false);
    };
    $('saveName').onclick = save;
    input.onkeydown = e => { if (e.key === 'Enter') save(); if (e.key === 'Escape') renderTeamName(false); };
  }
}

async function renderHome() {
  await refreshTeam();
  if (!team) return;
  renderTeamName(false);

  $('homeDriver').innerHTML = `<h3>CURRENT DRIVER</h3>
    <div class="pname">${esc(team.driver.name)}</div>
    <div class="rating">RATING ${team.driver.rating}</div>${driverStats(team.driver)}`;

  drawCarOn('homeCar', '#e10600', 7);

  $('homeCrews').innerHTML =
    `<div class="crew"><div class="name">MECHANICS: ${esc(team.mechanics.name)}</div>${mechanicStats(team.mechanics)}</div>
     <div class="crew"><div class="name">PIT CREW: ${esc(team.pitCrew.name)}</div>${pitStats(team.pitCrew)}</div>`;

  $('homeCarStats').innerHTML = `<h3>CURRENT CAR STATS</h3>${carStats(team.carStats)}`;
}

// ---------- GARAGE ----------
async function renderGarage() {
  await refreshTeam();
  if (!team) return;

  $('garageStats').innerHTML = `<h3>CURRENT CAR STATS</h3>${carStats(team.carStats)}`;
  drawCarOn('garageCar', '#e10600', 8);

  $('partButtons').innerHTML = team.parts.map(p => {
    let pips = '';
    for (let i = 1; i <= p.maxLevel; i++) pips += `<i class="${i <= p.level ? (p.maxed ? 'max' : 'on') : ''}"></i>`;
    const button = p.maxed
      ? `<button class="btn" disabled>MAX LEVEL</button>`
      : `<button class="btn red" data-part="${p.index}" ${p.affordable ? '' : 'disabled'}>UPGRADE ${money(p.upgradeCost)}</button>`;
    return `<div class="card">
      <div class="head"><span class="name">${esc(p.name)}</span><span>LV ${p.level}/${p.maxLevel}</span></div>
      <div class="pips">${pips}</div>
      <div class="desc">${esc(p.description)}</div>${button}</div>`;
  }).join('');

  document.querySelectorAll('[data-part]').forEach(btn => {
    btn.onclick = async () => {
      handleAction(await api('/upgrade?part=' + btn.dataset.part, 'POST'), 'PART UPGRADED!');
      renderGarage();
    };
  });
}

// ---------- MARKET ----------
async function renderMarket() {
  document.querySelectorAll('.tab').forEach(t => t.classList.toggle('active', t.dataset.tab === marketTab));
  const data = await api('/market?type=' + marketTab);
  if (!data) return;
  await refreshTeam();

  $('marketGrid').innerHTML = data.items.map(item => {
    let stats = '';
    if (marketTab === 'driver') stats = `<div class="rating">RATING ${item.rating}</div>` + driverStats(item);
    if (marketTab === 'mechanic') stats = mechanicStats(item);
    if (marketTab === 'pit') stats = pitStats(item);

    let button;
    if (item.equipped) button = `<button class="btn" disabled>EQUIPPED</button>`;
    else if (!item.affordable) button = `<button class="btn" disabled>CAN'T AFFORD</button>`;
    else button = `<button class="btn red" data-buy="${item.id}">BUY</button>`;

    return `<div class="card ${item.equipped ? 'equipped' : ''}">
      <div class="head"><span class="name">${esc(item.name)}</span>
        ${item.equipped ? '<span class="badge">IN USE</span>' : ''}</div>
      <div class="price">${item.price === 0 ? 'FREE (STARTER)' : money(item.price)}</div>
      ${stats}${button}</div>`;
  }).join('');

  document.querySelectorAll('[data-buy]').forEach(btn => {
    btn.onclick = async () => {
      handleAction(await api(`/buy?type=${marketTab}&id=${btn.dataset.buy}`, 'POST'), 'PURCHASED!');
      renderMarket();
    };
  });
}

// ---------- TRACK SELECT ----------
// The fixed facts of a track, as a small table of label/value pairs
function trackFacts(t) {
  return `<div class="facts">
    <span>LAPS</span><span>${t.laps} (${t.lengthKm} KM)</span>
    <span>AIR TEMP</span><span>${t.airTemp} C</span>
    <span>TRACK TEMP</span><span>${t.trackTemp} C</span>
    <span>HUMIDITY</span><span>${t.humidity}%</span>
    <span>RAIN CHANCE</span><span>${t.rainChance}%</span>
    <span>TYRE WEAR</span><span>${t.abrasiveness}/100</span>
    <span>DOWNFORCE</span><span>${t.downforceDemand}/100</span>
    <span>ENGINE POWER</span><span>${t.powerDemand}/100</span>
  </div>`;
}

async function renderTracks() {
  await refreshTeam();
  const data = await api('/tracks');
  if (!data) return;
  tracks = data.tracks;

  $('trackGrid').innerHTML = tracks.map(t => `
    <button class="card click" data-track="${t.id}">
      <div class="head"><span class="name">${esc(t.name)}</span><span>${esc(t.country)}</span></div>
      <div class="track-art"><canvas id="trackArt${t.id}"></canvas></div>
      <div class="desc">${esc(t.description)}</div>
      ${trackFacts(t)}
    </button>`).join('');

  tracks.forEach(t => drawTrack($('trackArt' + t.id), t.name));

  document.querySelectorAll('[data-track]').forEach(card => {
    card.onclick = () => {
      selectedTrack = tracks[Number(card.dataset.track)];
      selectedTyre = null;
      show('tyres');
    };
  });
}

// ---------- TYRE SELECT ----------
async function renderTyres() {
  if (!selectedTrack) { show('tracks'); return; }
  const data = await api('/tyres?track=' + selectedTrack.id);
  if (!data) return;

  $('trackInfo').innerHTML = `<h3>CHOSEN TRACK</h3>
    <div class="head"><span class="name">${esc(selectedTrack.name)}</span> <span>${esc(selectedTrack.country)}</span></div>
    <div class="track-art" style="margin:10px 0"><canvas id="tyreTrackArt"></canvas></div>
    <div class="desc">${esc(selectedTrack.description)}</div>${trackFacts(selectedTrack)}`;
  drawTrack($('tyreTrackArt'), selectedTrack.name);

  const grid = $('tyreGrid');
  grid.innerHTML = '';
  data.tyres.forEach(t => {
    const fitClass = 'fit-' + t.hint.split(' ')[0].toLowerCase();   // Great / Good / Risky / Poor
    const btn = document.createElement('button');
    btn.className = 'tyre' + (selectedTyre === t.id ? ' selected' : '');
    btn.appendChild(tyreIcon(t.id));
    const text = document.createElement('div');
    text.innerHTML = `<div class="tname">${esc(t.name.toUpperCase())}</div>
      <div class="${fitClass}">${esc(t.hint.toUpperCase())} (${Math.round(t.fit)}/100)</div>
      <div class="tinfo">DRY GRIP ${t.dryGrip} | WET GRIP ${t.wetGrip} | WEAR ${t.wear} | BEST ${t.tempMin}-${t.tempMax} C | PIT STOPS ${t.pitStops}</div>`;
    btn.appendChild(text);
    btn.onclick = () => {
      selectedTyre = t.id;
      grid.querySelectorAll('.tyre').forEach(b => b.classList.remove('selected'));
      btn.classList.add('selected');
      $('startRace').disabled = false;
    };
    grid.appendChild(btn);
  });
  $('startRace').disabled = (selectedTyre === null);
}

// Ask the server to run the race, then play the animation
async function startRace() {
  $('startRace').disabled = true;
  const reply = await api(`/race?track=${selectedTrack.id}&tyre=${selectedTyre}`, 'POST');
  if (!reply || !reply.ok) {
    toast(reply ? reply.message : 'SERVER ERROR', false);
    $('startRace').disabled = false;
    return;
  }
  raceData = reply;     // the result is already decided; the animation just shows it
  show('race');
}

// ---------- RESULTS ----------
function renderResults() {
  setTeam(raceData.team);   // now show the new money (prize included)
  const results = raceData.results;
  $('resultsTitle').textContent = 'RESULTS - ' + selectedTrack.name.toUpperCase();

  // podium: 2nd, 1st, 3rd (like a real podium)
  const order = [1, 0, 2];
  $('podium').innerHTML = '';
  order.forEach(i => {
    const r = results[i];
    const div = document.createElement('div');
    div.className = `pod p${i + 1}${r.isPlayer ? ' me' : ''}`;
    div.appendChild(copyCanvas(carSprite(teamColor(r), 3)));
    div.insertAdjacentHTML('beforeend',
      `<div class="tn">${esc(r.team)}${r.isPlayer ? ' (YOU)' : ''}</div><div class="dn">${esc(r.driver)}</div>
       <div class="block">${i + 1}</div>`);
    $('podium').appendChild(div);
  });

  // summary line
  const me = results.find(r => r.isPlayer);
  $('summary').innerHTML = me.dnf
    ? `YOUR CAR BROKE DOWN - <span class="big">DNF</span><br>NO PRIZE MONEY. BETTER MECHANICS HELP. BUDGET: ${money(team.budget)}`
    : `YOU FINISHED <span class="big">P${me.position}</span> AND WON <span class="big">${money(me.prize)}</span><br>NEW BUDGET: ${money(team.budget)}`;

  // full table
  $('resultsTable').querySelector('tbody').innerHTML = results.map(r => `
    <tr class="${r.isPlayer ? 'me' : ''}">
      <td>${r.position}</td><td>${esc(r.team)}${r.isPlayer ? ' (YOU)' : ''}</td><td>${esc(r.driver)}</td>
      <td>${esc(r.tyre)}</td><td>${r.pitStops}</td>
      <td class="${r.dnf ? 'dnf' : ''}">${r.dnf ? 'DNF' : 'FINISHED'}</td>
      <td>${r.prize ? money(r.prize) : '-'}</td>
    </tr>`).join('');
}

// canvases cannot be put in two places, so podium cars get their own copy
function copyCanvas(src) {
  const c = document.createElement('canvas');
  c.width = src.width;
  c.height = src.height;
  c.getContext('2d').drawImage(src, 0, 0);
  return c;
}

// ------------------------------------------------------------
// 4. RACE ANIMATION (just for show; the order is already decided)
// ------------------------------------------------------------

// A colour for each team's car. The player is always red.
const TEAM_COLORS = {
  'Crimson Arrow': '#a0153e', 'Silver Falcon': '#b8bcc8', 'Blue Meteor': '#3a8dff',
  'Orange Fox': '#ff8a00', 'Green Viper': '#2fb344', 'Black Stallion': '#55555d',
  'Yellow Hornet': '#ffd400', 'White Wolf': '#f2efe6', 'Purple Raven': '#9b59d0', 'Teal Shark': '#17b3a3'
};
function teamColor(result) {
  if (result.isPlayer) return '#e10600';
  return TEAM_COLORS[result.team] || '#888888';
}

function stopAnimation() {
  if (anim) { cancelAnimationFrame(anim.frame); anim = null; }
}

function startRaceAnimation() {
  $('raceTitle').textContent = 'RACE - ' + selectedTrack.name.toUpperCase();
  const results = raceData.results;
  const cv = $('raceCanvas');
  const ctx = cv.getContext('2d');
  ctx.imageSmoothingEnabled = false;

  const W = 960, LANE = 34, TOP = 50;
  const H = TOP + results.length * LANE + 10;
  cv.width = W;
  cv.height = H;

  const START_X = 130, FINISH_X = W - 56, CAR_W = 80;
  const LIGHTS_TIME = 1.8;      // seconds of "lights out" countdown
  const FIRST_FINISH = 6.0;     // the winner crosses the line this many seconds after lights out
  const GAP = 0.22;             // seconds between each finishing car

  // Build one object per car. Lane = a fixed scrambled order so the lanes don't reveal the result.
  const cars = results.map((r, i) => {
    const finished = !r.dnf;
    return {
      r: r,
      lane: (i * 7) % results.length,
      finishTime: finished ? FIRST_FINISH + i * GAP : 7.0,   // finishers cross in result order
      stopAt: finished ? 1 : 0.25 + ((i * 37) % 40) / 100,   // broken-down cars stop part-way
      phase: i * 1.3,                                         // makes each car wobble differently
      sprite: carSprite(teamColor(r), 2)
    };
  });
  const lastTime = Math.max(...cars.map(c => c.finishTime)) + 1.3;
  const startStamp = performance.now();
  anim = { frame: 0 };

  function draw(now) {
    const t = (now - startStamp) / 1000;
    const race = t - LIGHTS_TIME;           // seconds since lights out (negative = still waiting)

    // background + lanes
    ctx.fillStyle = '#16161a';
    ctx.fillRect(0, 0, W, H);
    cars.forEach(c => {
      const y = TOP + c.lane * LANE;
      ctx.fillStyle = c.r.isPlayer ? '#3a3410' : (c.lane % 2 ? '#26262d' : '#2d2d35');
      ctx.fillRect(0, y, W, LANE);
    });

    // start line and checkered finish line
    ctx.fillStyle = '#f2efe6';
    ctx.fillRect(START_X - 6, TOP, 4, results.length * LANE);
    for (let y = 0; y < results.length * LANE; y += 8) {
      for (let x = 0; x < 16; x += 8) {
        ctx.fillStyle = ((y / 8 + x / 8) % 2) ? '#f2efe6' : '#000';
        ctx.fillRect(FINISH_X + x, TOP + y, 8, 8);
      }
    }

    // start lights: 5 red squares that switch on one by one, then all go out
    for (let i = 0; i < 5; i++) {
      const on = t < LIGHTS_TIME - 0.3 && t > i * 0.25;
      ctx.fillStyle = on ? '#e10600' : '#3a3a44';
      ctx.fillRect(W / 2 - 100 + i * 44, 8, 32, 32);
    }
    if (race < 0) {
      ctx.fillStyle = '#f2efe6';
      ctx.font = 'bold 14px "Courier New", monospace';
      ctx.fillText('GET READY...', W / 2 + 130, 30);
    } else if (race < 1.2) {
      ctx.fillStyle = '#3ddc4a';
      ctx.font = 'bold 16px "Courier New", monospace';
      ctx.fillText('LIGHTS OUT - GO GO GO!', W / 2 - 100, 30);
    }

    // cars
    cars.forEach(c => {
      const y = TOP + c.lane * LANE + 1;
      let p = Math.max(0, race) / c.finishTime;                // 0 -> 1 progress along the track
      p += Math.sin(race * 1.7 + c.phase) * 0.03 * (1 - p);    // a little wobble so positions change mid-race
      if (race < c.finishTime) p = Math.min(p, 0.995);         // never cross the line before its finish time
      else p = 1;
      if (p > c.stopAt) p = c.stopAt;                          // broken-down cars stop
      if (p < 0) p = 0;
      const x = START_X + p * (FINISH_X - CAR_W - START_X);

      ctx.drawImage(c.sprite, Math.round(x), y);

      // smoke squares on a broken-down car
      if (c.r.dnf && p >= c.stopAt && race > 0) {
        for (let k = 0; k < 3; k++) {
          const sy = y - 4 - ((race * 20 + k * 9) % 14);
          ctx.fillStyle = k % 2 ? '#777' : '#444';
          ctx.fillRect(Math.round(x) + 14 + k * 6, sy + 10, 6, 6);
        }
        ctx.fillStyle = '#e10600';
        ctx.font = 'bold 12px "Courier New", monospace';
        ctx.fillText('DNF', Math.round(x) + 22, y + 12);
      }
      // finishing position once the car crosses the line
      if (!c.r.dnf && race >= c.finishTime) {
        ctx.fillStyle = c.r.position === 1 ? '#ffd400' : '#f2efe6';
        ctx.font = 'bold 14px "Courier New", monospace';
        ctx.fillText('P' + c.r.position, FINISH_X + 20, y + 22);
      }
      // team name at the left of the lane
      ctx.fillStyle = c.r.isPlayer ? '#ffd400' : '#9a9aa5';
      ctx.font = 'bold 12px "Courier New", monospace';
      ctx.fillText(c.r.isPlayer ? 'YOU' : c.r.team.toUpperCase().slice(0, 15), 6, y + 22);
    });

    if (race > lastTime) { show('results'); return; }
    anim.frame = requestAnimationFrame(draw);
  }
  anim.frame = requestAnimationFrame(draw);
}

// ------------------------------------------------------------
// 5. START-UP: connect the buttons and open the home screen
// ------------------------------------------------------------
document.querySelectorAll('[data-go]').forEach(btn => { btn.onclick = () => show(btn.dataset.go); });
document.querySelectorAll('.tab').forEach(tab => {
  tab.onclick = () => { marketTab = tab.dataset.tab; renderMarket(); };
});
$('homeBtn').onclick = () => show('home');
$('backToTracks').onclick = () => show('tracks');
$('startRace').onclick = startRace;
$('skipBtn').onclick = () => show('results');

show('home');
