#include <iostream>
#include <queue>
#include <stack>
#include <fstream>
#include <cmath>

using namespace std;

// ======================================================
// KICH THUOC BAI XE
// ======================================================

const int ROW = 5;
const int COL = 8;

const int MAX_SLOT = 20;
const int MAX_CAR  = 100;

// ======================================================
// MAP BAI XE
//
// 0 = DUONG DI
// 1 = VAT CAN
// 2 = O DO XE
// ======================================================

int parkingMap[ROW][COL] =
{
    {0,0,0,0,2,0,2,0},
    {0,1,1,0,2,0,2,0},
    {2,0,0,0,2,0,2,0},
    {2,1,1,0,2,0,2,0},
    {2,2,2,2,2,2,2,2}
};

// ======================================================
// STRUCT TOA DO
// ======================================================

struct Position
{
    int x;
    int y;
};

// ======================================================
// NODE CHO A*
// ======================================================

struct Node
{
    Position pos;

    int g;
    int h;
    int f;

    Position parent;

    bool visited;
};

// ======================================================
// SLOT XE
// ======================================================

struct Slot
{
    string id;

    Position pos;

    bool occupied;
};

// ======================================================
// XE
// ======================================================

struct Car
{
    string plate;

    string slotID;
};

// ======================================================
// SO SANH MIN HEAP
// ======================================================

struct Compare
{
    bool operator()(Node* a, Node* b)
    {
        return a->f > b->f;
    }
};

// ======================================================
// CLASS TIM DUONG
// ======================================================

class PathFinder
{
    private:

        Node nodes[ROW][COL];

    public:

        // ==============================================
        // KIEM TRA HOP LE
        // ==============================================

        bool isValid(int x, int y)
        {
            return
                x >= 0 &&
                x < ROW &&
                y >= 0 &&
                y < COL &&
                parkingMap[x][y] != 1;
        }

        // ==============================================
        // MANHATTAN DISTANCE
        // ==============================================

        int heuristic(
            Position a,
            Position b
        )
        {
            return
                abs(a.x - b.x)
              + abs(a.y - b.y);
        }

        // ==============================================
        // RESET NODE
        // ==============================================

        void resetNodes()
        {
            for(int i = 0; i < ROW; i++)
            {
                for(int j = 0; j < COL; j++)
                {
                    nodes[i][j].g = 999999;
                    nodes[i][j].h = 0;
                    nodes[i][j].f = 999999;

                    nodes[i][j].visited = false;

                    nodes[i][j].parent.x = -1;
                    nodes[i][j].parent.y = -1;
                }
            }
        }

        // ==============================================
        // IN DUONG DI
        // ==============================================

        void printPath(int x, int y)
        {
            if(x == -1 || y == -1)
            {
                return;
            }

            printPath(
                nodes[x][y].parent.x,
                nodes[x][y].parent.y
            );

            cout
                << "("
                << x
                << ","
                << y
                << ") ";
        }

        // ==============================================
        // TIM DO DAI DUONG DI
        // ==============================================

        int getDistance(
            Position start,
            Position goal
        )
        {
            priority_queue<
                Node*,
                vector<Node*>,
                Compare
            > openList;

            resetNodes();

            Node* startNode =
                &nodes[start.x][start.y];

            startNode->pos = start;

            startNode->g = 0;

            startNode->h =
                heuristic(start, goal);

            startNode->f =
                startNode->g +
                startNode->h;

            openList.push(startNode);

            int dx[4] = {-1,1,0,0};
            int dy[4] = {0,0,-1,1};

            while(!openList.empty())
            {
                Node* current =
                    openList.top();

                openList.pop();

                if(current->visited)
                {
                    continue;
                }

                current->visited = true;

                // ======================
                // DA TOI DICH
                // ======================

                if(
                    current->pos.x == goal.x &&
                    current->pos.y == goal.y
                )
                {
                    return current->g;
                }

                // ======================
                // XET 4 HUONG
                // ======================

                for(int i = 0; i < 4; i++)
                {
                    int nx =
                        current->pos.x + dx[i];

                    int ny =
                        current->pos.y + dy[i];

                    if(!isValid(nx, ny))
                    {
                        continue;
                    }

                    if(nodes[nx][ny].visited)
                    {
                        continue;
                    }

                    Node* neighbor =
                        &nodes[nx][ny];

                    int newG =
                        current->g + 1;

                    // ==================
                    // TIM DUONG TOT HON
                    // ==================

                    if(newG < neighbor->g)
                    {
                        neighbor->pos.x = nx;
                        neighbor->pos.y = ny;

                        neighbor->g = newG;

                        neighbor->h =
                            heuristic(
                                {nx, ny},
                                goal
                            );

                        neighbor->f =
                            neighbor->g +
                            neighbor->h;

                        neighbor->parent =
                            current->pos;

                        openList.push(neighbor);
                    }
                }
            }

            return 999999;
        }

        // ==============================================
        // IN DUONG DI A*
        // ==============================================

        void findPath(
            Position start,
            Position goal
        )
        {
            priority_queue<
                Node*,
                vector<Node*>,
                Compare
            > openList;

            resetNodes();

            Node* startNode =
                &nodes[start.x][start.y];

            startNode->pos = start;

            startNode->g = 0;

            startNode->h =
                heuristic(start, goal);

            startNode->f =
                startNode->g +
                startNode->h;

            openList.push(startNode);

            int dx[4] = {-1,1,0,0};
            int dy[4] = {0,0,-1,1};

            while(!openList.empty())
            {
                Node* current =
                    openList.top();

                openList.pop();

                if(current->visited)
                {
                    continue;
                }

                current->visited = true;

                // ======================
                // TOI DICH
                // ======================

                if(
                    current->pos.x == goal.x &&
                    current->pos.y == goal.y
                )
                {
                    cout << "\nDuong di:\n";

                    printPath(
                        goal.x,
                        goal.y
                    );

                    cout << endl;

                    return;
                }

                // ======================
                // XET 4 HUONG
                // ======================

                for(int i = 0; i < 4; i++)
                {
                    int nx =
                        current->pos.x + dx[i];

                    int ny =
                        current->pos.y + dy[i];

                    if(!isValid(nx, ny))
                    {
                        continue;
                    }

                    if(nodes[nx][ny].visited)
                    {
                        continue;
                    }

                    Node* neighbor =
                        &nodes[nx][ny];

                    int newG =
                        current->g + 1;

                    if(newG < neighbor->g)
                    {
                        neighbor->pos.x = nx;
                        neighbor->pos.y = ny;

                        neighbor->g = newG;

                        neighbor->h =
                            heuristic(
                                {nx, ny},
                                goal
                            );

                        neighbor->f =
                            neighbor->g +
                            neighbor->h;

                        neighbor->parent =
                            current->pos;

                        openList.push(neighbor);
                    }
                }
            }

            cout << "Khong tim thay duong di\n";
        }
};

// ======================================================
// HE THONG BAI XE
// ======================================================

class ParkingSystem
{
    private:

        Slot slots[MAX_SLOT];

        Car cars[MAX_CAR];

        int slotCount;
        int carCount;

        stack<string> history;

        PathFinder pathFinder;

    public:

        // ==============================================
        // CONSTRUCTOR
        // ==============================================

        ParkingSystem()
        {
            slotCount = 0;
            carCount  = 0;

            initializeSlots();

            loadData();

            loadHistory();
        }

        // ==============================================
        // TAO SLOT
        // ==============================================

        void initializeSlots()
        {
            int number = 1;

            for(int i = 0; i < ROW; i++)
            {
                for(int j = 0; j < COL; j++)
                {
                    if(parkingMap[i][j] == 2)
                    {
                        slots[slotCount].id =
                            "P" + to_string(number);

                        slots[slotCount].pos.x = i;
                        slots[slotCount].pos.y = j;

                        slots[slotCount].occupied = false;

                        slotCount++;
                        number++;
                    }
                }
            }
        }

        // ==============================================
        // TIM XE
        // ==============================================

        int findCar(string plate)
        {
            for(int i = 0; i < carCount; i++)
            {
                if(cars[i].plate == plate)
                {
                    return i;
                }
            }

            return -1;
        }

        // ==============================================
        // TIM SLOT
        // ==============================================

        int findSlot(string id)
        {
            for(int i = 0; i < slotCount; i++)
            {
                if(slots[i].id == id)
                {
                    return i;
                }
            }

            return -1;
        }

        // ==============================================
        // SORT SLOT GAN NHAT
        // ==============================================

        void sortSlotsByDistance(
            Slot temp[],
            int count
        )
        {
            Position start = {0,0};

            for(int i = 0; i < count - 1; i++)
            {
                for(int j = i + 1; j < count; j++)
                {
                    int d1 =
                        pathFinder.heuristic(
                            start,
                            temp[i].pos
                        );

                    int d2 =
                        pathFinder.heuristic(
                            start,
                            temp[j].pos
                        );

                    if(d2 < d1)
                    {
                        swap(temp[i], temp[j]);
                    }
                }
            }
        }

        // ==============================================
        // TIM SLOT GAN NHAT
        // ==============================================

        int findNearestSlot()
        {
            Slot temp[MAX_SLOT];

            int count = 0;

            // ==========================
            // LAY SLOT TRONG
            // ==========================

            for(int i = 0; i < slotCount; i++)
            {
                if(!slots[i].occupied)
                {
                    temp[count] = slots[i];

                    count++;
                }
            }

            if(count == 0)
            {
                return -1;
            }

            // ==========================
            // SAP XEP THEO HEURISTIC
            // ==========================

            sortSlotsByDistance(
                temp,
                count
            );

            // ==========================
            // KIEM TRA THUC TE BANG A*
            // ==========================

            Position start = {0,0};

            for(int i = 0; i < count; i++)
            {
                int distance =
                    pathFinder.getDistance(
                        start,
                        temp[i].pos
                    );

                if(distance != 999999)
                {
                    return findSlot(temp[i].id);
                }
            }

            return -1;
        }

        // ==============================================
        // LUU FILE
        // ==============================================

        void saveData()
        {
            ofstream file(
                "parking_data.txt"
            );

            for(int i = 0; i < carCount; i++)
            {
                file
                    << cars[i].plate
                    << " "
                    << cars[i].slotID
                    << endl;
            }

            file.close();
        }

        // ==============================================
        // DOC FILE
        // ==============================================

        void loadData()
        {
            ifstream file(
                "parking_data.txt"
            );

            string plate;
            string slotID;

            while(file >> plate >> slotID)
            {
                cars[carCount].plate = plate;
                cars[carCount].slotID = slotID;

                int slotIndex =
                    findSlot(slotID);

                if(slotIndex != -1)
                {
                    slots[slotIndex].occupied = true;
                }

                carCount++;
            }

            file.close();
        }

        // ==============================================
        // LUU HISTORY
        // ==============================================

        void saveHistory(string text)
        {
            ofstream file(
                "history.txt",
                ios::app
            );

            file << text << endl;

            file.close();
        }

        // ==============================================
        // DOC HISTORY
        // ==============================================

        void loadHistory()
        {
            ifstream file("history.txt");

            string line;

            while(getline(file, line))
            {
                history.push(line);
            }

            file.close();
        }

        // ==============================================
        // HIEN THI SLOT
        // ==============================================

        void showSlots()
        {
            cout << "\n===== SLOT =====\n";

            for(int i = 0; i < slotCount; i++)
            {
                cout
                    << slots[i].id
                    << " -> ";

                if(slots[i].occupied)
                {
                    cout << "OCCUPIED";
                }
                else
                {
                    cout << "EMPTY";
                }

                cout << endl;
            }
        }

        // ==============================================
        // GUI XE
        // ==============================================

        void parkCar()
        {
            string plate;

            cout << "\nNhap bien so: ";
            cin >> plate;

            // ======================
            // KIEM TRA TRUNG
            // ======================

            if(findCar(plate) != -1)
            {
                cout << "Xe da ton tai\n";

                return;
            }

            // ======================
            // TIM SLOT
            // ======================

            int slotIndex =
                findNearestSlot();

            if(slotIndex == -1)
            {
                cout << "Bai xe da full\n";

                return;
            }

            // ======================
            // CAP NHAT
            // ======================

            slots[slotIndex].occupied = true;

            cars[carCount].plate = plate;

            cars[carCount].slotID =
                slots[slotIndex].id;

            carCount++;

            // ======================
            // IN DUONG DI
            // ======================

            Position start = {0,0};

            Position goal =
                slots[slotIndex].pos;

            cout
                << "\nSlot duoc chon: "
                << slots[slotIndex].id
                << endl;

            pathFinder.findPath(
                start,
                goal
            );

            // ======================
            // HISTORY
            // ======================

            string text =
                "[IN ] "
                + plate
                + " -> "
                + slots[slotIndex].id;

            history.push(text);

            saveHistory(text);

            saveData();

            cout << "Gui xe thanh cong\n";
        }

        // ==============================================
        // LAY XE
        // ==============================================

        void removeCar()
        {
            string plate;

            cout << "\nNhap bien so: ";
            cin >> plate;

            int index =
                findCar(plate);

            if(index == -1)
            {
                cout << "Khong tim thay xe\n";

                return;
            }

            string slotID =
                cars[index].slotID;

            int slotIndex =
                findSlot(slotID);

            if(slotIndex != -1)
            {
                slots[slotIndex].occupied = false;
            }

            // ======================
            // XOA XE
            // ======================

            for(int i = index; i < carCount - 1; i++)
            {
                cars[i] = cars[i + 1];
            }

            carCount--;

            // ======================
            // HISTORY
            // ======================

            string text =
                "[OUT] "
                + plate
                + " <- "
                + slotID;

            history.push(text);

            saveHistory(text);

            saveData();

            cout << "Lay xe thanh cong\n";
        }

        // ==============================================
        // HISTORY
        // ==============================================

        void showHistory()
        {
            stack<string> temp = history;

            cout << "\n===== HISTORY =====\n";

            while(!temp.empty())
            {
                cout
                    << temp.top()
                    << endl;

                temp.pop();
            }
        }

        // ==============================================
        // RESET
        // ==============================================

        void reset()
        {
            carCount = 0;

            while(!history.empty())
            {
                history.pop();
            }

            for(int i = 0; i < slotCount; i++)
            {
                slots[i].occupied = false;
            }

            ofstream file1(
                "parking_data.txt"
            );

            file1.close();

            ofstream file2(
                "history.txt"
            );

            file2.close();

            cout << "\nReset thanh cong\n";
        }

        // ==============================================
        // MENU
        // ==============================================

        void run()
        {
            int choice;

            do
            {
                cout << "\n===== SMART PARKING =====\n";

                cout << "1. Gui xe\n";
                cout << "2. Lay xe\n";
                cout << "3. Xem slot\n";
                cout << "4. Xem history\n";
                cout << "5. Reset\n";
                cout << "0. Thoat\n";

                cout << "Nhap lua chon: ";
                cin >> choice;

                switch(choice)
                {
                    case 1:
                        parkCar();
                        break;

                    case 2:
                        removeCar();
                        break;

                    case 3:
                        showSlots();
                        break;

                    case 4:
                        showHistory();
                        break;

                    case 5:
                        reset();
                        break;

                    case 0:
                        cout << "Thoat...\n";
                        break;

                    default:
                        cout << "Lua chon khong hop le\n";
                }

            }
            while(choice != 0);
        }
};

// ======================================================
// MAIN
// ======================================================

int main()
{
    ParkingSystem system;

    system.run();

    return 0;
}