#ifndef UI_H
#define UI_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <iomanip>

using namespace std;

// ======================================================
// CONSTANTS
// ======================================================

constexpr int ROW = 5;
constexpr int COL = 8;

extern int parkingMap[ROW][COL];

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
    char id[10];
    Position pos;
    bool occupied;
};

// ======================================================
// XE
// ======================================================

struct Car
{
    char plate[20];
    char slotID[10];
};

// ======================================================
// HISTORY ENTRY
// ======================================================

struct HistoryEntry
{
    char text[100];
};

// ======================================================
// BÃI GIỮ XE
// ======================================================

struct ParkingLot
{
    Slot *slots;
    Car *cars;
    HistoryEntry *history;

    int slotCount;
    int carCount;
    int historyCount;
    int maxSlots;
    int maxCars;
    int maxHistory;

    Node nodes[ROW][COL];
};

// ======================================================
// CONSOLE UTILITIES
// ======================================================

void clearScreen()
{
    system("cls");
}

void displayTitle()
{
    cout << "\n";
    cout << "  ╔════════════════════════════════════════════╗\n";
    cout << "  ║                                            ║\n";
    cout << "  ║        🚗 SMART PARKING MANAGEMENT 🚗      ║\n";
    cout << "  ║             Bãi Xe Thông Minh              ║\n";
    cout << "  ║                                            ║\n";
    cout << "  ╚════════════════════════════════════════════╝\n";
    cout << "\n";
}

void displaySeparator()
{
    cout << "  ════════════════════════════════════════════\n";
}

void displayBox(const char *title)
{
    cout << "\n  ╔═══════════════════════════════════════════╗\n";
    cout << "  ║  " << setw(38) << left << title << " ║\n";
    cout << "  ╚═══════════════════════════════════════════╝\n";
}

// ======================================================
// PARKING MAP VISUALIZATION
// ======================================================

void displayParkingMap(ParkingLot &lot)
{
    displayBox("BÃNXEVISTOP (Bản Đồ Bãi Xe)");

    cout << "\n  ┌─────────────────────────────────────┐\n";

    for (int i = 0; i < 5; i++)
    {
        cout << "  │ ";
        for (int j = 0; j < 8; j++)
        {
            if (parkingMap[i][j] == 0)
                cout << "· "; // Đường đi
            else if (parkingMap[i][j] == 1)
                cout << "█ "; // Vật cản
            else if (parkingMap[i][j] == 2)
            {
                // Kiểm tra xem slot này có xe không
                bool occupied = false;
                for (int k = 0; k < lot.slotCount; k++)
                {
                    if (lot.slots[k].pos.x == i && lot.slots[k].pos.y == j)
                    {
                        occupied = lot.slots[k].occupied;
                        break;
                    }
                }
                cout << (occupied ? "🚙" : "□ "); // Xe hoặc slot trống
            }
        }
        cout << "│\n";
    }

    cout << "  └─────────────────────────────────────┘\n";
    cout << "  CHÂN DUNG: ·(đường) █(vật cản) □(slot) 🚙(xe)\n";
}

// ======================================================
// PARKING SLOTS DISPLAY
// ======================================================

void displaySlotStatus(ParkingLot &lot)
{
    displayBox("TRẠNG THÁI CÁC SLOT XE");

    int emptyCount = 0;
    int occupiedCount = 0;

    cout << "\n";
    for (int i = 0; i < lot.slotCount; i++)
    {
        if (lot.slots[i].occupied)
        {
            occupiedCount++;
            cout << "  ║ " << setw(3) << lot.slots[i].id << " → 🚙 OCCUPIED - Vị trí: ("
                 << lot.slots[i].pos.x << "," << lot.slots[i].pos.y << ")\n";
        }
        else
        {
            emptyCount++;
            cout << "  ║ " << setw(3) << lot.slots[i].id << " → □ EMPTY    - Vị trí: ("
                 << lot.slots[i].pos.x << "," << lot.slots[i].pos.y << ")\n";
        }
    }

    cout << "\n  ╭─────────────────────────────────────╮\n";
    cout << "  │ Tổng slot: " << setw(2) << lot.slotCount
         << "  |  Có xe: " << setw(2) << occupiedCount
         << "  |  Trống: " << setw(2) << emptyCount << "  │\n";
    cout << "  ╰─────────────────────────────────────╯\n";
}

// ======================================================
// CARS DISPLAY
// ======================================================

void displayAllCars(ParkingLot &lot)
{
    displayBox("DANH SÁCH XE ĐẬU TẠI BÃI");

    if (lot.carCount == 0)
    {
        cout << "\n  ⚠ Không có xe đậu tại bãi xe\n";
        return;
    }

    cout << "\n  ╔════════════════════════════════════════╗\n";
    for (int i = 0; i < lot.carCount; i++)
    {
        cout << "  ║ " << (i + 1) << ". Biển số: " << setw(15) << left << lot.cars[i].plate
             << " | Slot: " << setw(3) << lot.cars[i].slotID << " ║\n";
    }
    cout << "  ╚════════════════════════════════════════╝\n";
    cout << "  Tổng xe: " << lot.carCount << "\n";
}

// ======================================================
// HISTORY DISPLAY
// ======================================================

void displayHistory(ParkingLot &lot)
{
    displayBox("LỊCH SỬ HOẠT ĐỘNG");

    if (lot.historyCount == 0)
    {
        cout << "\n  ⚠ Không có lịch sử hoạt động\n";
        return;
    }

    cout << "\n  ╔════════════════════════════════════════╗\n";
    for (int i = lot.historyCount - 1; i >= 0 && i >= lot.historyCount - 20; i--)
    {
        if (strstr(lot.history[i].text, "[IN]"))
            cout << "  ║ ➕ " << setw(34) << left << lot.history[i].text << " ║\n";
        else if (strstr(lot.history[i].text, "[OUT]"))
            cout << "  ║ ➖ " << setw(34) << left << lot.history[i].text << " ║\n";
        else
            cout << "  ║    " << setw(34) << left << lot.history[i].text << " ║\n";
    }
    cout << "  ╚════════════════════════════════════════╝\n";
    cout << "  (Hiển thị 20 bản ghi gần nhất)\n";
}

// ======================================================
// MAIN MENU DISPLAY
// ======================================================

void displayMainMenu()
{
    displayTitle();
    displaySeparator();

    cout << "  ╔════════════════════════════════════════╗\n";
    cout << "  ║         MENU CHÍNH                     ║\n";
    cout << "  ╠════════════════════════════════════════╣\n";
    cout << "  ║  [1] 🚗 Gửi xe vào bãi                 ║\n";
    cout << "  ║  [2] 🚙 Lấy xe ra khỏi bãi             ║\n";
    cout << "  ║  [3] 📍 Xem bản đồ bãi xe              ║\n";
    cout << "  ║  [4] 📋 Xem trạng thái các slot        ║\n";
    cout << "  ║  [5] 🚗 Xem danh sách xe đậu           ║\n";
    cout << "  ║  [6] 📝 Xem lịch sử hoạt động          ║\n";
    cout << "  ║  [7] 🔄 Reset (xóa toàn bộ dữ liệu)    ║\n";
    cout << "  ║  [0] ❌ Thoát chương trình              ║\n";
    cout << "  ╚════════════════════════════════════════╝\n";
    cout << "  Nhập lựa chọn (0-7): ";
}

// ======================================================
// INPUT FUNCTIONS
// ======================================================

void getCarPlate(char *plate)
{
    cout << "\n  Nhập biển số xe (VD: ABC123): ";
    cin >> plate;
}

void pressEnterToContinue()
{
    cout << "\n  Nhấn Enter để tiếp tục...";
    cin.ignore();
    cin.get();
}

// ======================================================
// SUCCESS/ERROR MESSAGES
// ======================================================

void showSuccess(const char *message)
{
    cout << "\n  ✅ " << message << "\n";
}

void showError(const char *message)
{
    cout << "\n  ❌ " << message << "\n";
}

void showInfo(const char *message)
{
    cout << "\n  ℹ️  " << message << "\n";
}

// ======================================================
// ADVANCED INFO DISPLAY
// ======================================================

void displayParkingInfo(ParkingLot &lot)
{
    displayTitle();

    int emptySlots = 0;
    for (int i = 0; i < lot.slotCount; i++)
    {
        if (!lot.slots[i].occupied)
            emptySlots++;
    }

    cout << "  ╔════════════════════════════════════════╗\n";
    cout << "  ║           THÔNG TIN BÃI XE             ║\n";
    cout << "  ╠════════════════════════════════════════╣\n";
    cout << "  ║ Tổng số slot: " << setw(24) << lot.slotCount << " ║\n";
    cout << "  ║ Slot đang dùng: " << setw(23) << (lot.slotCount - emptySlots) << " ║\n";
    cout << "  ║ Slot còn trống: " << setw(23) << emptySlots << " ║\n";
    cout << "  ║ Xe đang đậu: " << setw(25) << lot.carCount << " ║\n";
    cout << "  ║ Tỉ lệ sử dụng: " << setw(22)
         << fixed << setprecision(1)
         << ((lot.slotCount - emptySlots) * 100.0 / lot.slotCount) << "% ║\n";
    cout << "  ║ Lịch sử ghi nhận: " << setw(21) << lot.historyCount << " ║\n";
    cout << "  ╚════════════════════════════════════════╝\n";
}

#endif
