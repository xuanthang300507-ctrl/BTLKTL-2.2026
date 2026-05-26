#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include "ui.h"

using namespace std;

// ======================================================
// MAP BAI XE
// ======================================================

int parkingMap[ROW][COL] =
    {
        {0, 0, 0, 0, 2, 0, 2, 0},
        {0, 1, 1, 0, 2, 0, 2, 0},
        {2, 0, 0, 0, 2, 0, 2, 0},
        {2, 1, 1, 0, 2, 0, 2, 0},
        {2, 2, 2, 2, 2, 2, 2, 2}};

// ======================================================
// QUEUE NODE CHO A*
// ======================================================

struct QueueNode
{
    Node *nodePtr;
    int priority;
};

// ======================================================
// PATHFINDER FUNCTIONS
// ======================================================

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
    {
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
}

void insertPriority(QueueNode queue[], int &size, Node *node, int priority)
{
    if (size >= ROW * COL)
        return;

    int pos = size;
    while (pos > 0 && queue[pos - 1].priority > priority)
    {
        queue[pos] = queue[pos - 1];
        pos--;
    }

    queue[pos].nodePtr = node;
    queue[pos].priority = priority;
    size++;
}

QueueNode extractMin(QueueNode queue[], int &size)
{
    QueueNode result = queue[0];
    for (int i = 0; i < size - 1; i++)
    {
        queue[i] = queue[i + 1];
    }
    size--;
    return result;
}

void printPath(Node nodes[ROW][COL], int x, int y)
{
    if (x == -1 || y == -1)
        return;

    printPath(nodes, nodes[x][y].parent.x, nodes[x][y].parent.y);

    cout << "(" << x << "," << y << ") ";
}

int getDistance(Node nodes[ROW][COL], Position start, Position goal)
{
    QueueNode openList[ROW * COL];
    int openSize = 0;

    resetNodes(nodes);

    Node *startNode = &nodes[start.x][start.y];
    startNode->pos = start;
    startNode->g = 0;
    startNode->h = heuristic(start, goal);
    startNode->f = startNode->g + startNode->h;

    insertPriority(openList, openSize, startNode, startNode->f);

    int dx[4] = {-1, 1, 0, 0};
    int dy[4] = {0, 0, -1, 1};

    while (openSize > 0)
    {
        QueueNode current = extractMin(openList, openSize);

        if (current.nodePtr->visited)
            continue;

        current.nodePtr->visited = true;

        if (current.nodePtr->pos.x == goal.x && current.nodePtr->pos.y == goal.y)
        {
            return current.nodePtr->g;
        }

        for (int i = 0; i < 4; i++)
        {
            int nx = current.nodePtr->pos.x + dx[i];
            int ny = current.nodePtr->pos.y + dy[i];

            if (!isValid(nx, ny))
                continue;
            if (nodes[nx][ny].visited)
                continue;

            Node *neighbor = &nodes[nx][ny];
            int newG = current.nodePtr->g + 1;

            if (newG < neighbor->g)
            {
                neighbor->pos.x = nx;
                neighbor->pos.y = ny;
                neighbor->g = newG;
                neighbor->h = heuristic({nx, ny}, goal);
                neighbor->f = neighbor->g + neighbor->h;
                neighbor->parent = current.nodePtr->pos;

                insertPriority(openList, openSize, neighbor, neighbor->f);
            }
        }
    }

    return 999999;
}

void findPath(Node nodes[ROW][COL], Position start, Position goal)
{
    QueueNode openList[ROW * COL];
    int openSize = 0;

    resetNodes(nodes);

    Node *startNode = &nodes[start.x][start.y];
    startNode->pos = start;
    startNode->g = 0;
    startNode->h = heuristic(start, goal);
    startNode->f = startNode->g + startNode->h;

    insertPriority(openList, openSize, startNode, startNode->f);

    int dx[4] = {-1, 1, 0, 0};
    int dy[4] = {0, 0, -1, 1};

    while (openSize > 0)
    {
        QueueNode current = extractMin(openList, openSize);

        if (current.nodePtr->visited)
            continue;

        current.nodePtr->visited = true;

        if (current.nodePtr->pos.x == goal.x && current.nodePtr->pos.y == goal.y)
        {
            cout << "\n  Đường đi:\n  ";
            printPath(nodes, goal.x, goal.y);
            cout << "\n  Khoảng cách: " << current.nodePtr->g << " bước\n";
            return;
        }

        for (int i = 0; i < 4; i++)
        {
            int nx = current.nodePtr->pos.x + dx[i];
            int ny = current.nodePtr->pos.y + dy[i];

            if (!isValid(nx, ny))
                continue;
            if (nodes[nx][ny].visited)
                continue;

            Node *neighbor = &nodes[nx][ny];
            int newG = current.nodePtr->g + 1;

            if (newG < neighbor->g)
            {
                neighbor->pos.x = nx;
                neighbor->pos.y = ny;
                neighbor->g = newG;
                neighbor->h = heuristic({nx, ny}, goal);
                neighbor->f = neighbor->g + neighbor->h;
                neighbor->parent = current.nodePtr->pos;

                insertPriority(openList, openSize, neighbor, neighbor->f);
            }
        }
    }

    showError("Không tìm thấy đường đi!");
}

// ======================================================
// PARKING LOT FUNCTIONS
// ======================================================

void initializeParkingLot(ParkingLot &lot, int maxSlots, int maxCars, int maxHistory)
{
    lot.slots = new Slot[maxSlots];
    lot.cars = new Car[maxCars];
    lot.history = new HistoryEntry[maxHistory];

    lot.maxSlots = maxSlots;
    lot.maxCars = maxCars;
    lot.maxHistory = maxHistory;

    lot.slotCount = 0;
    lot.carCount = 0;
    lot.historyCount = 0;

    int number = 1;
    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            if (parkingMap[i][j] == 2 && lot.slotCount < maxSlots)
            {
                sprintf(lot.slots[lot.slotCount].id, "P%d", number);
                lot.slots[lot.slotCount].pos.x = i;
                lot.slots[lot.slotCount].pos.y = j;
                lot.slots[lot.slotCount].occupied = false;

                lot.slotCount++;
                number++;
            }
        }
    }
}

void destroyParkingLot(ParkingLot &lot)
{
    delete[] lot.slots;
    delete[] lot.cars;
    delete[] lot.history;
}

int findCar(ParkingLot &lot, const char *plate)
{
    for (int i = 0; i < lot.carCount; i++)
    {
        if (strcmp(lot.cars[i].plate, plate) == 0)
            return i;
    }
    return -1;
}

int findSlot(ParkingLot &lot, const char *id)
{
    for (int i = 0; i < lot.slotCount; i++)
    {
        if (strcmp(lot.slots[i].id, id) == 0)
            return i;
    }
    return -1;
}

void sortSlotsByDistance(Slot temp[], int count)
{
    Position start = {0, 0};

    for (int i = 0; i < count - 1; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            int d1 = heuristic(start, temp[i].pos);
            int d2 = heuristic(start, temp[j].pos);

            if (d2 < d1)
            {
                Slot tmp = temp[i];
                temp[i] = temp[j];
                temp[j] = tmp;
            }
        }
    }
}

int findNearestSlot(ParkingLot &lot)
{
    Slot *temp = new Slot[lot.slotCount];
    int count = 0;

    for (int i = 0; i < lot.slotCount; i++)
    {
        if (!lot.slots[i].occupied)
        {
            temp[count] = lot.slots[i];
            count++;
        }
    }

    if (count == 0)
    {
        delete[] temp;
        return -1;
    }

    sortSlotsByDistance(temp, count);

    Position start = {0, 0};
    for (int i = 0; i < count; i++)
    {
        int distance = getDistance(lot.nodes, start, temp[i].pos);
        if (distance != 999999)
        {
            int result = findSlot(lot, temp[i].id);
            delete[] temp;
            return result;
        }
    }

    delete[] temp;
    return -1;
}

void saveData(ParkingLot &lot, const char *filename)
{
    ofstream file(filename);

    for (int i = 0; i < lot.carCount; i++)
    {
        file << lot.cars[i].plate << " " << lot.cars[i].slotID << "\n";
    }

    file.close();
}

void loadData(ParkingLot &lot, const char *filename)
{
    ifstream file(filename);

    char plate[20];
    char slotID[10];

    while (file >> plate >> slotID)
    {
        if (lot.carCount < lot.maxCars)
        {
            strcpy(lot.cars[lot.carCount].plate, plate);
            strcpy(lot.cars[lot.carCount].slotID, slotID);

            int slotIndex = findSlot(lot, slotID);
            if (slotIndex != -1)
            {
                lot.slots[slotIndex].occupied = true;
            }

            lot.carCount++;
        }
    }

    file.close();
}

void saveHistory(ParkingLot &lot, const char *filename, const char *text)
{
    ofstream file(filename, ios::app);
    file << text << "\n";
    file.close();

    if (lot.historyCount < lot.maxHistory)
    {
        strcpy(lot.history[lot.historyCount].text, text);
        lot.historyCount++;
    }
}

void loadHistory(ParkingLot &lot, const char *filename)
{
    ifstream file(filename);
    char line[100];

    while (file.getline(line, sizeof(line)))
    {
        if (lot.historyCount < lot.maxHistory)
        {
            strcpy(lot.history[lot.historyCount].text, line);
            lot.historyCount++;
        }
    }

    file.close();
}

// ======================================================
// ENHANCED UI FUNCTIONS
// ======================================================

void parkCarUI(ParkingLot &lot)
{
    clearScreen();
    displayBox("GỬI XE VÀO BÃI");

    char plate[20];
    getCarPlate(plate);

    if (findCar(lot, plate) != -1)
    {
        showError("Xe này đã tồn tại trong hệ thống!");
        pressEnterToContinue();
        return;
    }

    int slotIndex = findNearestSlot(lot);

    if (slotIndex == -1)
    {
        showError("Bãi xe đã đầy! Không có slot trống nào.");
        pressEnterToContinue();
        return;
    }

    if (lot.carCount >= lot.maxCars)
    {
        showError("Hệ thống đã đầy!");
        pressEnterToContinue();
        return;
    }

    lot.slots[slotIndex].occupied = true;
    strcpy(lot.cars[lot.carCount].plate, plate);
    strcpy(lot.cars[lot.carCount].slotID, lot.slots[slotIndex].id);
    lot.carCount++;

    Position start = {0, 0};
    Position goal = lot.slots[slotIndex].pos;

    cout << "\n  ┌─────────────────────────────────────┐\n";
    cout << "  │ Slot được chọn: " << lot.slots[slotIndex].id << "\n";
    cout << "  │ Vị trí: (" << lot.slots[slotIndex].pos.x << "," << lot.slots[slotIndex].pos.y << ")\n";
    cout << "  └─────────────────────────────────────┘\n";

    findPath(lot.nodes, start, goal);

    char historyText[100];
    sprintf(historyText, "[IN ] %s -> %s", plate, lot.slots[slotIndex].id);

    saveHistory(lot, "history.txt", historyText);
    saveData(lot, "parking_data.txt");

    showSuccess("Gửi xe thành công!");
    pressEnterToContinue();
}

void removeCarUI(ParkingLot &lot)
{
    clearScreen();
    displayBox("LẤY XE RA KHỎI BÃI");

    char plate[20];
    getCarPlate(plate);

    int index = findCar(lot, plate);

    if (index == -1)
    {
        showError("Không tìm thấy xe này trong bãi xe!");
        pressEnterToContinue();
        return;
    }

    char slotID[10];
    strcpy(slotID, lot.cars[index].slotID);

    int slotIndex = findSlot(lot, slotID);
    if (slotIndex != -1)
    {
        lot.slots[slotIndex].occupied = false;
    }

    for (int i = index; i < lot.carCount - 1; i++)
    {
        lot.cars[i] = lot.cars[i + 1];
    }
    lot.carCount--;

    char historyText[100];
    sprintf(historyText, "[OUT] %s <- %s", plate, slotID);

    saveHistory(lot, "history.txt", historyText);
    saveData(lot, "parking_data.txt");

    showSuccess("Lấy xe thành công!");
    pressEnterToContinue();
}

void viewParkingMap(ParkingLot &lot)
{
    clearScreen();
    displayParkingMap(lot);
    pressEnterToContinue();
}

void viewSlotStatus(ParkingLot &lot)
{
    clearScreen();
    displaySlotStatus(lot);
    pressEnterToContinue();
}

void viewAllCars(ParkingLot &lot)
{
    clearScreen();
    displayAllCars(lot);
    pressEnterToContinue();
}

void viewParkingHistory(ParkingLot &lot)
{
    clearScreen();
    displayHistory(lot);
    pressEnterToContinue();
}

void resetParkingLotUI(ParkingLot &lot)
{
    clearScreen();
    displayBox("RESET HỆ THỐNG");

    cout << "\n  ⚠️  BẠN SẮP XÓA TOÀN BỘ DỮ LIỆU!\n";
    cout << "  Nhập [YES] để xác nhận hoặc bất cứ phím khác để hủy: ";

    string confirm;
    cin >> confirm;

    if (confirm == "YES")
    {
        lot.carCount = 0;
        lot.historyCount = 0;

        for (int i = 0; i < lot.slotCount; i++)
        {
            lot.slots[i].occupied = false;
        }

        ofstream file1("parking_data.txt");
        file1.close();

        ofstream file2("history.txt");
        file2.close();

        showSuccess("Reset hệ thống thành công!");
    }
    else
    {
        showInfo("Đã hủy lệnh reset!");
    }

    pressEnterToContinue();
}

// ======================================================
// MAIN UI LOOP
// ======================================================

void runParkingSystemUI(ParkingLot &lot)
{
    int choice;

    do
    {
        clearScreen();
        displayMainMenu();
        cin >> choice;

        switch (choice)
        {
        case 1:
            parkCarUI(lot);
            break;
        case 2:
            removeCarUI(lot);
            break;
        case 3:
            viewParkingMap(lot);
            break;
        case 4:
            viewSlotStatus(lot);
            break;
        case 5:
            viewAllCars(lot);
            break;
        case 6:
            viewParkingHistory(lot);
            break;
        case 7:
            resetParkingLotUI(lot);
            break;
        case 0:
            clearScreen();
            displayTitle();
            cout << "  ✅ Cảm ơn bạn đã sử dụng Smart Parking!\n";
            cout << "  📌 Tạm biệt!\n\n";
            break;
        default:
            showError("Lựa chọn không hợp lệ! Vui lòng nhập từ 0 đến 7.");
            pressEnterToContinue();
        }

    } while (choice != 0);
}

// ======================================================
// MAIN ENTRY POINT
// ======================================================

int main()
{
    system("chcp 65001");
    ParkingLot lot;

    initializeParkingLot(lot, 20, 100, 500);

    loadData(lot, "parking_data.txt");
    loadHistory(lot, "history.txt");

    runParkingSystemUI(lot);

    destroyParkingLot(lot);

    return 0;
}
