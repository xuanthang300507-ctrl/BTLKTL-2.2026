/*
 * ============================================================
 *  SMART PARKING SYSTEM  –  Web UI Server (C++)
 *  Chạy: g++ -o parking_server parking_server.cpp && ./parking_server
 *  Mở:   http://localhost:8080
 * ============================================================
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <vector>
#include <algorithm>
#include <cstdlib>
// ── Socket headers ─────────────────────────────────────────
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define CLOSE(s) closesocket(s)
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define CLOSE(s) close(s)
#endif

// ============================================================
//  DATA STRUCTURES
// ============================================================

#define ROW 5
#define COL 8
#define MAX_SLOTS 20
#define MAX_CARS 100
#define MAX_HISTORY 500

struct Position
{
  int x, y;
};

struct Node
{
  Position pos;
  int g, h, f;
  bool visited;
  Position parent;
};

struct Slot
{
  char id[10];
  Position pos;
  bool occupied;
};

struct Car
{
  char plate[20];
  char slotID[10];
  char checkIn[32]; // timestamp
};

struct HistoryEntry
{
  char text[120];
};

struct ParkingLot
{
  Slot slots[MAX_SLOTS];
  Car cars[MAX_CARS];
  HistoryEntry history[MAX_HISTORY];
  Node nodes[ROW][COL];
  int slotCount, carCount, historyCount;
};

// ============================================================
//  MAP  (0=road, 1=wall, 2=parking slot)
// ============================================================

int parkingMap[ROW][COL] = {
    {0, 0, 0, 0, 2, 0, 2, 0},
    {0, 1, 1, 0, 2, 0, 2, 0},
    {2, 0, 0, 0, 2, 0, 2, 0},
    {2, 1, 1, 0, 2, 0, 2, 0},
    {2, 2, 2, 2, 2, 2, 2, 2}};

// ============================================================
//  A* PATH-FINDER
// ============================================================

struct QueueNode
{
  Node *ptr;
  int priority;
};

bool isValid(int x, int y)
{
  return x >= 0 && x < ROW && y >= 0 && y < COL && parkingMap[x][y] != 1;
}

int heuristic(Position a, Position b)
{
  return abs(a.x - b.x) + abs(a.y - b.y);
}

void resetNodes(Node nodes[ROW][COL])
{
  for (int i = 0; i < ROW; i++)
    for (int j = 0; j < COL; j++)
    {
      nodes[i][j].g = 999999;
      nodes[i][j].h = 0;
      nodes[i][j].f = 999999;
      nodes[i][j].visited = false;
      nodes[i][j].parent.x = -1;
      nodes[i][j].parent.y = -1;
    }
}

void insertPQ(QueueNode q[], int &sz, Node *n, int pri)
{
  if (sz >= ROW * COL)
    return;
  int pos = sz;
  while (pos > 0 && q[pos - 1].priority > pri)
  {
    q[pos] = q[pos - 1];
    pos--;
  }
  q[pos].ptr = n;
  q[pos].priority = pri;
  sz++;
}

QueueNode extractMin(QueueNode q[], int &sz)
{
  QueueNode r = q[0];
  for (int i = 0; i < sz - 1; i++)
    q[i] = q[i + 1];
  sz--;
  return r;
}

int getDistance(Node nodes[ROW][COL], Position s, Position g)
{
  QueueNode open[ROW * COL];
  int sz = 0;
  resetNodes(nodes);
  Node *sn = &nodes[s.x][s.y];
  sn->pos = s;
  sn->g = 0;
  sn->h = heuristic(s, g);
  sn->f = sn->h;
  insertPQ(open, sz, sn, sn->f);
  int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
  while (sz > 0)
  {
    QueueNode cur = extractMin(open, sz);
    if (cur.ptr->visited)
      continue;
    cur.ptr->visited = true;
    if (cur.ptr->pos.x == g.x && cur.ptr->pos.y == g.y)
      return cur.ptr->g;
    for (int i = 0; i < 4; i++)
    {
      int nx = cur.ptr->pos.x + dx[i], ny = cur.ptr->pos.y + dy[i];
      if (!isValid(nx, ny) || nodes[nx][ny].visited)
        continue;
      Node *nb = &nodes[nx][ny];
      int ng = cur.ptr->g + 1;
      if (ng < nb->g)
      {
        nb->pos.x = nx;
        nb->pos.y = ny;
        nb->g = ng;
        nb->h = heuristic({nx, ny}, g);
        nb->f = nb->g + nb->h;
        nb->parent = cur.ptr->pos;
        insertPQ(open, sz, nb, nb->f);
      }
    }
  }
  return 999999;
}

// reconstruct path as JSON array
std::string reconstructPath(Node nodes[ROW][COL], int ex, int ey)
{
  if (ex == -1 || ey == -1)
    return "[]";
  std::vector<std::pair<int, int>> path;
  int cx = ex, cy = ey;
  while (cx != -1 && cy != -1)
  {
    path.push_back({cx, cy});
    int px = nodes[cx][cy].parent.x, py = nodes[cx][cy].parent.y;
    cx = px;
    cy = py;
  }
  std::reverse(path.begin(), path.end());
  std::string s = "[";
  for (size_t i = 0; i < path.size(); i++)
  {
    if (i)
      s += ",";
    s += "[" + std::to_string(path[i].first) + "," + std::to_string(path[i].second) + "]";
  }
  s += "]";
  return s;
}

std::string findPathJSON(Node nodes[ROW][COL], Position s, Position g)
{
  QueueNode open[ROW * COL];
  int sz = 0;
  resetNodes(nodes);
  Node *sn = &nodes[s.x][s.y];
  sn->pos = s;
  sn->g = 0;
  sn->h = heuristic(s, g);
  sn->f = sn->h;
  insertPQ(open, sz, sn, sn->f);
  int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
  while (sz > 0)
  {
    QueueNode cur = extractMin(open, sz);
    if (cur.ptr->visited)
      continue;
    cur.ptr->visited = true;
    if (cur.ptr->pos.x == g.x && cur.ptr->pos.y == g.y)
      return reconstructPath(nodes, g.x, g.y);
    for (int i = 0; i < 4; i++)
    {
      int nx = cur.ptr->pos.x + dx[i], ny = cur.ptr->pos.y + dy[i];
      if (!isValid(nx, ny) || nodes[nx][ny].visited)
        continue;
      Node *nb = &nodes[nx][ny];
      int ng = cur.ptr->g + 1;
      if (ng < nb->g)
      {
        nb->pos.x = nx;
        nb->pos.y = ny;
        nb->g = ng;
        nb->h = heuristic({nx, ny}, g);
        nb->f = nb->g + nb->h;
        nb->parent = cur.ptr->pos;
        insertPQ(open, sz, nb, nb->f);
      }
    }
  }
  return "[]";
}

// ============================================================
//  PARKING LOT LOGIC
// ============================================================

ParkingLot lot;

void initLot()
{
  lot.slotCount = 0;
  lot.carCount = 0;
  lot.historyCount = 0;
  int num = 1;
  for (int i = 0; i < ROW; i++)
    for (int j = 0; j < COL; j++)
      if (parkingMap[i][j] == 2 && lot.slotCount < MAX_SLOTS)
      {
        sprintf(lot.slots[lot.slotCount].id, "P%d", num++);
        lot.slots[lot.slotCount].pos.x = i;
        lot.slots[lot.slotCount].pos.y = j;
        lot.slots[lot.slotCount].occupied = false;
        lot.slotCount++;
      }
}

int findCar(const char *plate)
{
  for (int i = 0; i < lot.carCount; i++)
    if (strcmp(lot.cars[i].plate, plate) == 0)
      return i;
  return -1;
}

int findSlot(const char *id)
{
  for (int i = 0; i < lot.slotCount; i++)
    if (strcmp(lot.slots[i].id, id) == 0)
      return i;
  return -1;
}

int findNearestSlot()
{
  Position start = {0, 0};
  int best = -1, bestDist = 999999;
  for (int i = 0; i < lot.slotCount; i++)
  {
    if (!lot.slots[i].occupied)
    {
      int d = getDistance(lot.nodes, start, lot.slots[i].pos);
      if (d < bestDist)
      {
        bestDist = d;
        best = i;
      }
    }
  }
  return best;
}

void addHistory(const char *text)
{
  if (lot.historyCount < MAX_HISTORY)
    strcpy(lot.history[lot.historyCount++].text, text);
}

void saveData()
{
  std::ofstream f("parking_data.txt");
  for (int i = 0; i < lot.carCount; i++)
    f << lot.cars[i].plate << " " << lot.cars[i].slotID << " " << lot.cars[i].checkIn << "\n";
}

void loadData()
{
  std::ifstream f("parking_data.txt");
  if (!f)
    return;
  char plate[20], slotID[10], ts[32];
  while (f >> plate >> slotID >> ts && lot.carCount < MAX_CARS)
  {
    strcpy(lot.cars[lot.carCount].plate, plate);
    strcpy(lot.cars[lot.carCount].slotID, slotID);
    strcpy(lot.cars[lot.carCount].checkIn, ts);
    int si = findSlot(slotID);
    if (si != -1)
      lot.slots[si].occupied = true;
    lot.carCount++;
  }
}

void loadHistory()
{
  std::ifstream f("history.txt");
  if (!f)
    return;
  char line[120];
  while (f.getline(line, sizeof(line)) && lot.historyCount < MAX_HISTORY)
    strcpy(lot.history[lot.historyCount++].text, line);
}

void saveHistory(const char *text)
{
  std::ofstream f("history.txt", std::ios::app);
  f << text << "\n";
}

// ============================================================
//  URL DECODE
// ============================================================

std::string urlDecode(const std::string &s)
{
  std::string r;
  for (size_t i = 0; i < s.size(); i++)
  {
    if (s[i] == '%' && i + 2 < s.size())
    {
      int v;
      sscanf(s.substr(i + 1, 2).c_str(), "%x", &v);
      r += (char)v;
      i += 2;
    }
    else if (s[i] == '+')
      r += ' ';
    else
      r += s[i];
  }
  return r;
}

std::string getParam(const std::string &body, const std::string &key)
{
  size_t p = body.find(key + "=");
  if (p == std::string::npos)
    return "";
  p += key.size() + 1;
  size_t e = body.find('&', p);
  return urlDecode(e == std::string::npos ? body.substr(p) : body.substr(p, e - p));
}

// ============================================================
//  JSON BUILDERS
// ============================================================

std::string lotStatusJSON()
{
  int occupied = 0;
  for (int i = 0; i < lot.slotCount; i++)
    if (lot.slots[i].occupied)
      occupied++;
  int free = lot.slotCount - occupied;

  std::string s = "{";
  s += "\"total\":" + std::to_string(lot.slotCount) + ",";
  s += "\"occupied\":" + std::to_string(occupied) + ",";
  s += "\"free\":" + std::to_string(free) + ",";
  s += "\"cars\":[";
  for (int i = 0; i < lot.carCount; i++)
  {
    if (i)
      s += ",";
    s += "{\"plate\":\"" + std::string(lot.cars[i].plate) + "\",";
    s += "\"slot\":\"" + std::string(lot.cars[i].slotID) + "\",";
    s += "\"checkin\":\"" + std::string(lot.cars[i].checkIn) + "\"}";
  }
  s += "],\"slots\":[";
  for (int i = 0; i < lot.slotCount; i++)
  {
    if (i)
      s += ",";
    s += "{\"id\":\"" + std::string(lot.slots[i].id) + "\",";
    s += "\"x\":" + std::to_string(lot.slots[i].pos.x) + ",";
    s += "\"y\":" + std::to_string(lot.slots[i].pos.y) + ",";
    s += "\"occupied\":" + (lot.slots[i].occupied ? std::string("true") : std::string("false")) + "}";
  }
  s += "],\"history\":[";
  int start = std::max(0, lot.historyCount - 20);
  for (int i = lot.historyCount - 1; i >= start; i--)
  {
    if (i != lot.historyCount - 1)
      s += ",";
    // escape quotes
    std::string txt = lot.history[i].text;
    std::string esc;
    for (char c : txt)
    {
      if (c == '"')
        esc += "\\\"";
      else
        esc += c;
    }
    s += "\"" + esc + "\"";
  }
  s += "]}";
  return s;
}

// ============================================================
//  ACTION HANDLERS  → return JSON {ok, msg, path, steps}
// ============================================================

std::string handlePark(const std::string &body)
{
  std::string plate = getParam(body, "plate");
  if (plate.empty())
    return "{\"ok\":false,\"msg\":\"Thiếu biển số xe\"}";
  // sanitize
  for (char c : plate)
    if (!isalnum(c) && c != '-')
      return "{\"ok\":false,\"msg\":\"Biển số không hợp lệ\"}";

  if (findCar(plate.c_str()) != -1)
    return "{\"ok\":false,\"msg\":\"Xe đã có trong bãi!\"}";
  if (lot.carCount >= MAX_CARS)
    return "{\"ok\":false,\"msg\":\"Hệ thống đã đầy!\"}";

  int si = findNearestSlot();
  if (si == -1)
    return "{\"ok\":false,\"msg\":\"Bãi đã đầy!\"}";

  // A* path
  Position start = {0, 0}, goal = lot.slots[si].pos;
  std::string pathArr = findPathJSON(lot.nodes, start, goal);
  int dist = getDistance(lot.nodes, start, goal);

  lot.slots[si].occupied = true;
  strcpy(lot.cars[lot.carCount].plate, plate.c_str());
  strcpy(lot.cars[lot.carCount].slotID, lot.slots[si].id);
  time_t t = time(0);
  struct tm *tm = localtime(&t);
  char ts[32];
  strftime(ts, sizeof(ts), "%H:%M %d/%m/%Y", tm);
  strcpy(lot.cars[lot.carCount].checkIn, ts);
  lot.carCount++;

  char hist[120];
  sprintf(hist, "[IN ] %s → %s  (%s)", plate.c_str(), lot.slots[si].id, ts);
  addHistory(hist);
  saveHistory(hist);
  saveData();

  return "{\"ok\":true,\"msg\":\"Gửi xe thành công!\","
         "\"slot\":\"" +
         std::string(lot.slots[si].id) + "\","
                                         "\"x\":" +
         std::to_string(lot.slots[si].pos.x) + ","
                                               "\"y\":" +
         std::to_string(lot.slots[si].pos.y) + ","
                                               "\"steps\":" +
         std::to_string(dist) + ","
                                "\"path\":" +
         pathArr + "}";
}

std::string handleLeave(const std::string &body)
{
  std::string plate = getParam(body, "plate");
  if (plate.empty())
    return "{\"ok\":false,\"msg\":\"Thiếu biển số xe\"}";

  int idx = findCar(plate.c_str());
  if (idx == -1)
    return "{\"ok\":false,\"msg\":\"Không tìm thấy xe trong bãi!\"}";

  char slotID[10];
  strcpy(slotID, lot.cars[idx].slotID);
  int si = findSlot(slotID);
  if (si != -1)
    lot.slots[si].occupied = false;

  for (int i = idx; i < lot.carCount - 1; i++)
    lot.cars[i] = lot.cars[i + 1];
  lot.carCount--;

  time_t t = time(0);
  struct tm *tm = localtime(&t);
  char ts[32];
  strftime(ts, sizeof(ts), "%H:%M %d/%m/%Y", tm);
  char hist[120];
  sprintf(hist, "[OUT] %s ← %s  (%s)", plate.c_str(), slotID, ts);
  addHistory(hist);
  saveHistory(hist);
  saveData();

  return "{\"ok\":true,\"msg\":\"Lấy xe thành công!\",\"slot\":\"" + std::string(slotID) + "\"}";
}

std::string handleReset()
{
  lot.carCount = 0;
  lot.historyCount = 0;
  for (int i = 0; i < lot.slotCount; i++)
    lot.slots[i].occupied = false;
  std::ofstream("parking_data.txt").close();
  std::ofstream("history.txt").close();
  return "{\"ok\":true,\"msg\":\"Reset hệ thống thành công!\"}";
}

// ============================================================
//  HTML PAGE
// ============================================================

const char *HTML = R"RAW(<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>🅿️ Smart Parking System</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Space+Mono:wght@400;700&family=Syne:wght@400;700;800&display=swap');

  :root{
    --bg:#0b0e14;
    --panel:#131720;
    --panel2:#1a2030;
    --border:#252d40;
    --accent:#00e5ff;
    --accent2:#ff6b35;
    --green:#00ff9d;
    --red:#ff4466;
    --yellow:#ffd700;
    --text:#e2e8f0;
    --muted:#6b7fa3;
    --radius:12px;
  }

  *{box-sizing:border-box;margin:0;padding:0}
  body{background:var(--bg);color:var(--text);font-family:'Syne',sans-serif;min-height:100vh;overflow-x:hidden}

  /* ── HEADER ── */
  header{
    background:linear-gradient(135deg,#0d1829 0%,#091221 100%);
    border-bottom:1px solid var(--border);
    padding:0 32px;
    display:flex;align-items:center;justify-content:space-between;
    height:64px;
    position:sticky;top:0;z-index:100;
    box-shadow:0 4px 24px rgba(0,229,255,.06);
  }
  .logo{display:flex;align-items:center;gap:12px;font-size:1.2rem;font-weight:800;letter-spacing:-.5px}
  .logo-icon{width:36px;height:36px;background:var(--accent);border-radius:8px;display:grid;place-items:center;font-size:1.1rem}
  .badge{background:var(--accent);color:#000;font-size:.65rem;font-weight:700;padding:2px 8px;border-radius:99px;font-family:'Space Mono',monospace}
  .clock{font-family:'Space Mono',monospace;font-size:.8rem;color:var(--muted)}

  /* ── LAYOUT ── */
  .layout{display:grid;grid-template-columns:300px 1fr;min-height:calc(100vh - 64px)}

  /* ── SIDEBAR ── */
  aside{
    background:var(--panel);
    border-right:1px solid var(--border);
    padding:24px 16px;
    display:flex;flex-direction:column;gap:16px;
    overflow-y:auto;
  }
  .section-label{font-size:.65rem;font-weight:700;letter-spacing:.15em;text-transform:uppercase;color:var(--muted);margin-bottom:8px}

  /* stats cards */
  .stats-grid{display:grid;grid-template-columns:1fr 1fr;gap:8px}
  .stat-card{
    background:var(--panel2);
    border:1px solid var(--border);
    border-radius:var(--radius);
    padding:12px;
    text-align:center;
  }
  .stat-card .val{font-size:1.8rem;font-weight:800;font-family:'Space Mono',monospace}
  .stat-card .lbl{font-size:.65rem;color:var(--muted);margin-top:2px}
  .stat-card.total .val{color:var(--accent)}
  .stat-card.free .val{color:var(--green)}
  .stat-card.busy .val{color:var(--red)}
  .stat-card.rate .val{color:var(--yellow)}

  /* form card */
  .form-card{
    background:var(--panel2);
    border:1px solid var(--border);
    border-radius:var(--radius);
    padding:16px;
  }
  .tab-row{display:flex;gap:6px;margin-bottom:14px}
  .tab{
    flex:1;padding:7px;border-radius:8px;border:1px solid var(--border);
    background:transparent;color:var(--muted);font-family:'Syne',sans-serif;
    font-size:.8rem;font-weight:700;cursor:pointer;transition:all .2s;
  }
  .tab.active{background:var(--accent);color:#000;border-color:var(--accent)}
  .tab:hover:not(.active){border-color:var(--accent);color:var(--accent)}

  .plate-input{
    width:100%;background:#0b0e14;border:1px solid var(--border);
    border-radius:8px;padding:10px 12px;color:var(--text);
    font-family:'Space Mono',monospace;font-size:1rem;letter-spacing:.1em;
    text-transform:uppercase;outline:none;transition:border .2s;
  }
  .plate-input:focus{border-color:var(--accent);box-shadow:0 0 0 3px rgba(0,229,255,.1)}
  .plate-input::placeholder{color:var(--muted);font-size:.8rem;letter-spacing:0;text-transform:none}

  .btn{
    width:100%;margin-top:10px;padding:11px;border:none;border-radius:8px;
    font-family:'Syne',sans-serif;font-size:.85rem;font-weight:700;
    cursor:pointer;transition:all .2s;
  }
  .btn-park{background:linear-gradient(135deg,var(--green),#00c77a);color:#000}
  .btn-leave{background:linear-gradient(135deg,var(--red),#cc2244);color:#fff}
  .btn-reset{background:transparent;border:1px solid var(--red)!important;color:var(--red);margin-top:0}
  .btn:hover{transform:translateY(-1px);filter:brightness(1.1)}
  .btn:active{transform:translateY(0)}

  /* toast */
  #toast{
    position:fixed;bottom:32px;left:50%;transform:translateX(-50%) translateY(80px);
    background:var(--panel);border:1px solid var(--border);border-radius:10px;
    padding:12px 24px;font-size:.85rem;font-weight:700;
    transition:all .35s cubic-bezier(.34,1.56,.64,1);z-index:999;
    box-shadow:0 8px 32px rgba(0,0,0,.4);white-space:nowrap;
  }
  #toast.show{transform:translateX(-50%) translateY(0)}
  #toast.ok{border-color:var(--green);color:var(--green)}
  #toast.err{border-color:var(--red);color:var(--red)}

  /* ── MAIN ── */
  main{padding:24px;overflow-y:auto;display:flex;flex-direction:column;gap:24px}

  .main-grid{display:grid;grid-template-columns:1fr 1fr;gap:24px}

  .card{
    background:var(--panel);
    border:1px solid var(--border);
    border-radius:var(--radius);
    overflow:hidden;
  }
  .card-header{
    padding:14px 20px;
    border-bottom:1px solid var(--border);
    display:flex;align-items:center;justify-content:space-between;
  }
  .card-title{font-size:.85rem;font-weight:700;letter-spacing:.05em}
  .card-body{padding:20px}

  /* MAP */
  #map-canvas{display:block;margin:0 auto}
  .map-legend{display:flex;gap:16px;justify-content:center;margin-top:12px;flex-wrap:wrap}
  .legend-item{display:flex;align-items:center;gap:6px;font-size:.72rem;color:var(--muted)}
  .legend-dot{width:12px;height:12px;border-radius:3px}

  /* path info */
  #path-info{
    background:var(--panel2);border:1px solid var(--border);border-radius:8px;
    padding:12px 16px;font-family:'Space Mono',monospace;font-size:.78rem;
    min-height:48px;color:var(--muted);display:none;margin-top:12px;
  }
  #path-info.visible{display:block;color:var(--accent)}

  /* CARS TABLE */
  .table-wrap{overflow-x:auto}
  table{width:100%;border-collapse:collapse;font-size:.8rem}
  th{padding:8px 12px;text-align:left;font-size:.65rem;letter-spacing:.1em;text-transform:uppercase;color:var(--muted);border-bottom:1px solid var(--border)}
  td{padding:10px 12px;border-bottom:1px solid rgba(37,45,64,.5)}
  tr:last-child td{border-bottom:none}
  tr:hover td{background:var(--panel2)}
  .plate-badge{
    background:var(--panel2);border:1px solid var(--border);
    border-radius:6px;padding:3px 8px;font-family:'Space Mono',monospace;
    font-size:.75rem;letter-spacing:.08em;
  }
  .slot-tag{color:var(--accent);font-family:'Space Mono',monospace;font-weight:700}
  .ts-text{color:var(--muted);font-size:.72rem}
  .empty-state{text-align:center;color:var(--muted);padding:32px;font-size:.85rem}

  /* HISTORY */
  #history-list{
    max-height:220px;overflow-y:auto;
    display:flex;flex-direction:column;gap:4px;
  }
  #history-list::-webkit-scrollbar{width:4px}
  #history-list::-webkit-scrollbar-track{background:transparent}
  #history-list::-webkit-scrollbar-thumb{background:var(--border);border-radius:4px}
  .hist-item{
    padding:7px 12px;border-radius:6px;
    font-family:'Space Mono',monospace;font-size:.72rem;
    background:var(--panel2);border:1px solid transparent;
  }
  .hist-item.in{border-color:rgba(0,255,157,.15);color:var(--green)}
  .hist-item.out{border-color:rgba(255,68,102,.15);color:var(--red)}

  /* slot grid */
  #slot-grid{display:flex;flex-wrap:wrap;gap:8px}
  .slot-chip{
    padding:6px 12px;border-radius:8px;font-family:'Space Mono',monospace;
    font-size:.75rem;font-weight:700;border:1px solid;transition:all .2s;cursor:default;
  }
  .slot-chip.free{background:rgba(0,255,157,.08);border-color:rgba(0,255,157,.3);color:var(--green)}
  .slot-chip.busy{background:rgba(255,68,102,.08);border-color:rgba(255,68,102,.3);color:var(--red)}

  /* loading overlay on button */
  .spinning{animation:spin .6s linear infinite}
  @keyframes spin{to{transform:rotate(360deg)}}

  /* responsive */
  @media(max-width:900px){
    .layout{grid-template-columns:1fr}
    aside{padding:16px}
    .main-grid{grid-template-columns:1fr}
  }
</style>
</head>
<body>

<header>
  <div class="logo">
    <div class="logo-icon">🅿️</div>
    <span>Smart Parking</span>
    <span class="badge">LIVE</span>
  </div>
  <div class="clock" id="clock">--:--:--</div>
</header>

<div class="layout">
  <!-- ── SIDEBAR ── -->
  <aside>
    <div>
      <div class="section-label">Tổng quan</div>
      <div class="stats-grid">
        <div class="stat-card total"><div class="val" id="s-total">—</div><div class="lbl">Tổng slot</div></div>
        <div class="stat-card free"><div class="val" id="s-free">—</div><div class="lbl">Còn trống</div></div>
        <div class="stat-card busy"><div class="val" id="s-busy">—</div><div class="lbl">Đã đỗ</div></div>
        <div class="stat-card rate"><div class="val" id="s-rate">—</div><div class="lbl">Lấp đầy</div></div>
      </div>
    </div>

    <div class="form-card">
      <div class="tab-row">
        <button class="tab active" id="tab-park" onclick="switchTab('park')">🚗 Gửi xe</button>
        <button class="tab" id="tab-leave" onclick="switchTab('leave')">🔓 Lấy xe</button>
      </div>
      <input class="plate-input" id="plate-in" placeholder="Nhập biển số (VD: 51A-12345)" maxlength="12" oninput="this.value=this.value.toUpperCase()">
      <button class="btn btn-park" id="action-btn" onclick="doAction()">⟶ Gửi xe</button>
    </div>

    <div>
      <div class="section-label">Slot bãi xe</div>
      <div id="slot-grid"></div>
    </div>

    <div style="margin-top:auto">
      <div class="section-label">Hệ thống</div>
      <button class="btn btn-reset" onclick="doReset()">🗑 Reset toàn bộ dữ liệu</button>
    </div>
  </aside>

  <!-- ── MAIN ── -->
  <main>
    <div class="main-grid">
      <!-- MAP -->
      <div class="card">
        <div class="card-header">
          <span class="card-title">🗺 Sơ đồ bãi xe + A* Pathfinder</span>
        </div>
        <div class="card-body">
          <canvas id="map-canvas" width="400" height="260"></canvas>
          <div id="path-info"></div>
          <div class="map-legend">
            <div class="legend-item"><div class="legend-dot" style="background:#1e3a5f"></div>Đường đi</div>
            <div class="legend-item"><div class="legend-dot" style="background:#2d1b1b"></div>Tường</div>
            <div class="legend-item"><div class="legend-dot" style="background:#0d3320"></div>Slot trống</div>
            <div class="legend-item"><div class="legend-dot" style="background:#3a0d1a"></div>Slot có xe</div>
            <div class="legend-item"><div class="legend-dot" style="background:#00e5ff"></div>Đường A*</div>
          </div>
        </div>
      </div>

      <!-- HISTORY -->
      <div class="card">
        <div class="card-header">
          <span class="card-title">📋 Lịch sử giao dịch</span>
        </div>
        <div class="card-body">
          <div id="history-list"><div class="empty-state">Chưa có giao dịch nào</div></div>
        </div>
      </div>
    </div>

    <!-- CARS TABLE -->
    <div class="card">
      <div class="card-header">
        <span class="card-title">🚙 Xe đang trong bãi</span>
        <span id="car-count" style="font-family:'Space Mono';font-size:.75rem;color:var(--accent)">0 xe</span>
      </div>
      <div class="card-body" style="padding:0">
        <div class="table-wrap">
          <table>
            <thead><tr><th>#</th><th>Biển số</th><th>Slot</th><th>Giờ vào</th></tr></thead>
            <tbody id="cars-tbody"></tbody>
          </table>
        </div>
      </div>
    </div>
  </main>
</div>

<div id="toast"></div>

<script>
// ── State
let state = {tab:'park', path:[], slotTarget:null};

// ── Clock
function updateClock(){
  const n=new Date();
  document.getElementById('clock').textContent=
    n.toLocaleTimeString('vi-VN',{hour12:false});
}
setInterval(updateClock,1000); updateClock();

// ── Tab switch
function switchTab(t){
  state.tab=t;
  document.getElementById('tab-park').className='tab'+(t==='park'?' active':'');
  document.getElementById('tab-leave').className='tab'+(t==='leave'?' active':'');
  const btn=document.getElementById('action-btn');
  if(t==='park'){btn.textContent='⟶ Gửi xe';btn.className='btn btn-park';}
  else{btn.textContent='⟵ Lấy xe';btn.className='btn btn-leave';}
}

// ── Toast
function toast(msg,ok){
  const el=document.getElementById('toast');
  el.textContent=msg; el.className='show '+(ok?'ok':'err');
  setTimeout(()=>el.className='',2800);
}

// ── Fetch status
async function refresh(){
  try{
    const r=await fetch('/api/status');
    const d=await r.json();
    updateUI(d);
  }catch(e){}
}

function updateUI(d){
  document.getElementById('s-total').textContent=d.total;
  document.getElementById('s-free').textContent=d.free;
  document.getElementById('s-busy').textContent=d.occupied;
  const pct=d.total?Math.round(d.occupied/d.total*100):0;
  document.getElementById('s-rate').textContent=pct+'%';

  // slot chips
  const sg=document.getElementById('slot-grid');
  sg.innerHTML='';
  d.slots.forEach(s=>{
    const el=document.createElement('div');
    el.className='slot-chip '+(s.occupied?'busy':'free');
    el.textContent=s.id;
    el.title=s.occupied?'Đã có xe':'Trống';
    sg.appendChild(el);
  });

  // cars table
  const tb=document.getElementById('cars-tbody');
  document.getElementById('car-count').textContent=d.cars.length+' xe';
  if(d.cars.length===0){
    tb.innerHTML='<tr><td colspan="4" class="empty-state">Không có xe nào trong bãi</td></tr>';
  } else {
    tb.innerHTML=d.cars.map((c,i)=>`
      <tr>
        <td style="color:var(--muted)">${i+1}</td>
        <td><span class="plate-badge">${c.plate}</span></td>
        <td><span class="slot-tag">${c.slot}</span></td>
        <td><span class="ts-text">${c.checkin}</span></td>
      </tr>`).join('');
  }

  // history
  const hl=document.getElementById('history-list');
  if(d.history.length===0){
    hl.innerHTML='<div class="empty-state">Chưa có giao dịch nào</div>';
  } else {
    hl.innerHTML=d.history.map(h=>{
      const isIn=h.startsWith('[IN');
      return `<div class="hist-item ${isIn?'in':'out'}">${isIn?'↓':'↑'} ${h}</div>`;
    }).join('');
  }

  // draw map (no new path)
  drawMap(d.slots, state.path, state.slotTarget);
}

// ── Canvas map
function drawMap(slots, path, target){
  const canvas=document.getElementById('map-canvas');
  const ctx=canvas.getContext('2d');
  const W=canvas.width, H=canvas.height;
  const rows=5, cols=8;
  const cw=Math.floor(W/cols), ch=Math.floor(H/rows);
  ctx.clearRect(0,0,W,H);

  const pathSet=new Set(path.map(p=>p[0]+','+p[1]));

  // cell colors
  const slotMap={};
  slots.forEach(s=>slotMap[s.x+','+s.y]={id:s.id,occupied:s.occupied});

  for(let r=0;r<rows;r++){
    for(let c=0;c<cols;c++){
      const key=r+','+c;
      const px=c*cw, py=r*ch;
      const v=[[0,0,0,0,2,0,2,0],[0,1,1,0,2,0,2,0],[2,0,0,0,2,0,2,0],[2,1,1,0,2,0,2,0],[2,2,2,2,2,2,2,2]][r][c];

      // base fill
      if(v===1) ctx.fillStyle='#1e1520';
      else if(v===2){
        if(slotMap[key]&&slotMap[key].occupied) ctx.fillStyle='#2a0f1a';
        else ctx.fillStyle='#0a2818';
      } else ctx.fillStyle='#101828';

      ctx.fillRect(px+1,py+1,cw-2,ch-2);
      ctx.strokeStyle='#1a2236'; ctx.lineWidth=1;
      ctx.strokeRect(px+1,py+1,cw-2,ch-2);

      // path highlight
      if(pathSet.has(key) && v!==1){
        ctx.fillStyle='rgba(0,229,255,0.18)';
        ctx.fillRect(px+1,py+1,cw-2,ch-2);
        // draw path dot
        ctx.beginPath();
        ctx.arc(px+cw/2,py+ch/2,3,0,Math.PI*2);
        ctx.fillStyle='rgba(0,229,255,0.8)';
        ctx.fill();
      }

      // wall stripes
      if(v===1){
        ctx.fillStyle='#2a1a2e';
        for(let d=0;d<cw+ch;d+=8){
          ctx.beginPath();ctx.moveTo(px+d,py);ctx.lineTo(px,py+d);
          ctx.strokeStyle='#1a0f22';ctx.lineWidth=1;ctx.stroke();
        }
      }

      // slot label
      if(v===2 && slotMap[key]){
        const s=slotMap[key];
        const isTarget=(target&&target[0]===r&&target[1]===c);
        ctx.font=`bold ${Math.floor(ch*0.3)}px 'Space Mono',monospace`;
        ctx.textAlign='center'; ctx.textBaseline='middle';
        ctx.fillStyle=isTarget?'#00ff9d':(s.occupied?'#ff4466':'#00e5ff');
        ctx.fillText(s.id, px+cw/2, py+ch/2);
        if(s.occupied){
          ctx.font=`${Math.floor(ch*0.25)}px monospace`;
          ctx.fillStyle='rgba(255,68,102,.6)';
          ctx.fillText('🚗', px+cw/2, py+ch/2+ch*0.28);
        }
      }

      // entry point
      if(r===0&&c===0){
        ctx.font=`${Math.floor(ch*0.5)}px monospace`;
        ctx.textAlign='center';ctx.textBaseline='middle';
        ctx.fillText('🚦',px+cw/2,py+ch/2);
      }
    }
  }

  // draw path line
  if(path.length>1){
    ctx.beginPath();
    ctx.moveTo(path[0][1]*cw+cw/2, path[0][0]*ch+ch/2);
    for(let i=1;i<path.length;i++)
      ctx.lineTo(path[i][1]*cw+cw/2, path[i][0]*ch+ch/2);
    ctx.strokeStyle='rgba(0,229,255,0.7)';
    ctx.lineWidth=2.5;
    ctx.setLineDash([5,3]);
    ctx.stroke();
    ctx.setLineDash([]);

    // arrowhead at end
    const last=path[path.length-1];
    ctx.beginPath();
    ctx.arc(last[1]*cw+cw/2, last[0]*ch+ch/2, 6,0,Math.PI*2);
    ctx.fillStyle='#00e5ff';ctx.fill();
  }
}

// ── Actions
async function doAction(){
  const plate=document.getElementById('plate-in').value.trim();
  if(!plate){toast('Vui lòng nhập biển số xe!',false);return;}
  const btn=document.getElementById('action-btn');
  btn.disabled=true; btn.textContent='...';

  const endpoint=state.tab==='park'?'/api/park':'/api/leave';
  const body='plate='+encodeURIComponent(plate);
  try{
    const r=await fetch(endpoint,{method:'POST',body,headers:{'Content-Type':'application/x-www-form-urlencoded'}});
    const d=await r.json();
    toast(d.msg, d.ok);
    if(d.ok && state.tab==='park' && d.path){
      state.path=d.path;
      state.slotTarget=[d.x,d.y];
      const pi=document.getElementById('path-info');
      pi.className='visible';
      pi.textContent=`A* → Slot ${d.slot}  |  ${d.steps} bước  |  Path: ${d.path.map(p=>'('+p[0]+','+p[1]+')').join(' → ')}`;
    } else if(state.tab==='leave'){
      state.path=[];state.slotTarget=null;
      document.getElementById('path-info').className='';
    }
    if(d.ok) document.getElementById('plate-in').value='';
    await refresh();
  } finally{
    btn.disabled=false;
    switchTab(state.tab);
  }
}

async function doReset(){
  if(!confirm('Reset toàn bộ dữ liệu bãi xe?')) return;
  const r=await fetch('/api/reset',{method:'POST'});
  const d=await r.json();
  state.path=[];state.slotTarget=null;
  document.getElementById('path-info').className='';
  toast(d.msg,d.ok);
  refresh();
}

// ── Keyboard
document.getElementById('plate-in').addEventListener('keydown',e=>{if(e.key==='Enter')doAction();});

// ── Init
refresh();
setInterval(refresh, 3000);
</script>
</body>
</html>
)RAW";

// ============================================================
//  HTTP SERVER
// ============================================================

std::string httpResponse(int code, const std::string &ct, const std::string &body)
{
  std::string status = (code == 200) ? "200 OK" : (code == 404) ? "404 Not Found"
                                                                : "400 Bad Request";
  std::ostringstream r;
  r << "HTTP/1.1 " << status << "\r\n"
    << "Content-Type: " << ct << "\r\n"
    << "Content-Length: " << body.size() << "\r\n"
    << "Access-Control-Allow-Origin: *\r\n"
    << "Connection: close\r\n\r\n"
    << body;
  return r.str();
}

void handleClient(int sock)
{
  char buf[8192] = {};
  recv(sock, buf, sizeof(buf) - 1, 0);

  std::string req(buf);
  // parse method + path
  std::string method, path, bodyStr;
  {
    std::istringstream ss(req);
    std::string version;
    ss >> method >> path >> version;
  }

  // find body (after \r\n\r\n)
  size_t bp = req.find("\r\n\r\n");
  if (bp != std::string::npos)
    bodyStr = req.substr(bp + 4);

  std::string resp;
  if (path == "/" || path == "/index.html")
  {
    resp = httpResponse(200, "text/html; charset=utf-8", std::string(HTML));
  }
  else if (path == "/api/status" && method == "GET")
  {
    resp = httpResponse(200, "application/json", lotStatusJSON());
  }
  else if (path == "/api/park" && method == "POST")
  {
    resp = httpResponse(200, "application/json", handlePark(bodyStr));
  }
  else if (path == "/api/leave" && method == "POST")
  {
    resp = httpResponse(200, "application/json", handleLeave(bodyStr));
  }
  else if (path == "/api/reset" && method == "POST")
  {
    resp = httpResponse(200, "application/json", handleReset());
  }
  else
  {
    resp = httpResponse(404, "text/plain", "Not found");
  }

  send(sock, resp.c_str(), resp.size(), 0);
  CLOSE(sock);
}

int main()
{
#ifdef _WIN32
  WSADATA wd;
  WSAStartup(MAKEWORD(2, 2), &wd);
#endif

  initLot();
  loadData();
  loadHistory();

  int server = socket(AF_INET, SOCK_STREAM, 0);
  if (server < 0)
  {
    std::cerr << "Cannot create socket\n";
    return 1;
  }

  int opt = 1;
  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(8080);

  if (bind(server, (sockaddr *)&addr, sizeof(addr)) < 0)
  {
    std::cerr << "Bind failed (port 8080 busy?)\n";
    return 1;
  }
  listen(server, 16);
  std::cout << "Starting server..." << std::endl;
  system("start http://localhost:8080");

  while (true)
  {
    sockaddr_in cli{};
    socklen_t clen = sizeof(cli);
    int client = accept(server, (sockaddr *)&cli, &clen);
    if (client < 0)
      continue;
    handleClient(client);
  }

  CLOSE(server);
  return 0;
}
