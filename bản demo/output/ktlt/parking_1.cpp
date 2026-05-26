#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include <string>
#include <cmath>
#include <fstream>

using namespace std;

const int ROW = 5;
const int COL = 8;

struct Node
{
    int x;
    int y;

    int g;
    int h;
    int f;

    Node* parent;

    Node(int _x, int _y)
    {
        x = _x;
        y = _y;

        g = h = f = 0;

        parent = NULL;
    }
};

struct Compare
{
    bool operator()(Node* a, Node* b)
    {
        return a->f > b->f;
    }
};

int parkingMap[ROW][COL] =
{
    {0,0,0,0,2,0,2,0},
    {0,1,1,0,2,0,2,0},
    {2,0,0,0,2,0,2,0},
    {2,1,1,0,2,0,2,0},
    {2,2,2,2,2,2,2,2}
};

unordered_map<string, string> slotStatus;

unordered_map<string, pair<int,int>>
slotPosition;
unordered_map<string,string>
carSlot;

string makeKey(int x, int y)
{
    return to_string(x)
         + ","
         + to_string(y);
}

bool isValid(int x, int y)
{
    return x >= 0 &&
           x < ROW &&
           y >= 0 &&
           y < COL &&
           parkingMap[x][y] != 1;
}

int heuristic(
    int x1,int y1,
    int x2,int y2)
{
    return abs(x1 - x2)
         + abs(y1 - y2);
}

void saveData()
{
    ofstream file("parking_data.txt");

    for(auto car : carSlot)
    {
        file << car.first
             << " "
             << car.second
             << endl;
    }

    file.close();
}

void loadData()
{
    ifstream file("parking_data.txt");

    string plate;
    string slot;

    while(file >> plate >> slot)
    {
        // KHOI PHUC XE
        carSlot[plate] = slot;

        // KHOI PHUC SLOT
        slotStatus[slot] =
            "OCCUPIED";
    }

    file.close();
}

void initializeSlots()
{
    int count = 1;

    for(int i = 0; i < ROW; i++)
    {
        for(int j = 0; j < COL; j++)
        {
            if(parkingMap[i][j] == 2)
            {
                string slotName =
                    "P" + to_string(count);

                slotStatus[slotName] =
                    "EMPTY";

                slotPosition[slotName] =
                    {i,j};

                count++;
            }
        }
    }
}

string findNearestSlot()
{
    int minDistance = 999999;

    string bestSlot = "FULL";

    int startX = 0;
    int startY = 0;

    for(auto slot : slotStatus)
    {
        // CHI XET SLOT TRONG
        if(slot.second == "EMPTY")
        {
            pair<int,int> pos =
                slotPosition[slot.first];

            int distance =
                heuristic(
                    startX,
                    startY,
                    pos.first,
                    pos.second
                );

            if(distance < minDistance)
            {
                minDistance =
                    distance;

                bestSlot =
                    slot.first;
            }
        }
    }

    return bestSlot;
}

void printPath(Node* goal)
{
    if(goal == NULL)
        return;

    printPath(goal->parent);

    cout << "("
         << goal->x
         << ","
         << goal->y
         << ") ";
}

void AStar(
    int startX,
    int startY,
    int goalX,
    int goalY)
{
    priority_queue<
        Node*,
        vector<Node*>,
        Compare
    > openList;

    unordered_map<string,bool>
    visited;

    Node* start =
        new Node(startX,startY);

    // KHOI TAO START
    start->g = 0;

    start->h =
        heuristic(
            startX,
            startY,
            goalX,
            goalY
        );

    start->f =
        start->g + start->h;

    openList.push(start);

    int dx[4] = {-1,1,0,0};
    int dy[4] = {0,0,-1,1};

    while(!openList.empty())
    {
        Node* current =
            openList.top();

        openList.pop();

        string key =
            makeKey(
                current->x,
                current->y
            );

        if(visited[key])
            continue;

        visited[key] = true;

        if(current->x == goalX &&
           current->y == goalY)
        {
            cout << "\nDuong di:\n";

            printPath(current);

            cout << endl;

            return;
        }

        for(int i = 0; i < 4; i++)
        {
            int nx =
                current->x + dx[i];

            int ny =
                current->y + dy[i];

            if(!isValid(nx,ny))
                continue;

            string nextKey =
                makeKey(nx,ny);

            if(visited[nextKey])
                continue;

            Node* neighbor =
                new Node(nx,ny);

            neighbor->g =
                current->g + 1;

            neighbor->h =
                heuristic(
                    nx,
                    ny,
                    goalX,
                    goalY
                );

            neighbor->f =
                neighbor->g +
                neighbor->h;

            neighbor->parent =
                current;

            openList.push(neighbor);
        }
    }

    cout << "Khong tim thay duong di\n";
}

void runParkingSystem()
{
    initializeSlots();
    loadData();

    string plateNumber;

    cout << "Nhap bien so xe: ";
    cin >> plateNumber;

    // KIEM TRA XE DA GUI
    if(carSlot.find(plateNumber)
       != carSlot.end())
    {
        cout << "Xe da ton tai trong bai\n";
        return;
    }

    string slot =
        findNearestSlot();

    // KHONG CON SLOT
    if(slot == "FULL")
    {
        cout << "Bai xe da full\n";
        return;
    }

    // SLOT DA CO XE
    if(slotStatus[slot]
       == "OCCUPIED")
    {
        cout << "Slot da co xe\n";
        return;
    }

    cout << "\nSlot gan nhat: "
         << slot << endl;

    pair<int,int> pos =
        slotPosition[slot];

    int carX = 0;
    int carY = 0;

    AStar(
        carX,
        carY,
        pos.first,
        pos.second
    );

    // KHOA SLOT
    slotStatus[slot] =
        "OCCUPIED";

    // LUU XE -> SLOT
    carSlot[plateNumber] =
        slot;
    
    // LUU REALTIME
    saveData();

    cout << "\nCap nhat:\n";

    cout << slot
         << " -> OCCUPIED\n";

    cout << plateNumber
         << " dang gui tai "
         << slot
         << endl;
}

int main()
{
    runParkingSystem();

    return 0;
}