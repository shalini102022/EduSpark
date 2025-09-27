// tourmate_fixed.cpp
// TourMate Vehicle Management System - fixed version (C++17)
// Compile: g++ -std=c++17 tourmate_fixed.cpp -o tourmate

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <random>

using namespace std;

const string VEH_FILE = "vehicles.txt";
const string SALES_FILE = "sales.txt";
const string TEMP_FILE = "temp.txt";

struct Vehicle {
    string id;         // unique
    string make;
    string model;
    int year = 0;
    string plate;
    double rentPerDay = 0.0;
    string status;     // "available" or "rented"
};

struct Sale {
    string saleId;
    string vehicleId;
    string vehicleDesc;
    string customerName;
    int days = 0;
    double total = 0.0;
    string dateTime;
};

// get current date/time string
string nowToString() {
    time_t t = time(nullptr);
    char buf[64];
    if (strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t)))
        return string(buf);
    return "";
}

// Split CSV line respecting quotes
vector<string> splitCSV(const string &line) {
    vector<string> out;
    string cur;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            // Handle double quotes inside quoted field
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                cur.push_back('"');
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(cur);
    return out;
}

// Escape a field if it contains comma or quote
string escapeCSV(const string &s) {
    bool needQuotes = s.find(',') != string::npos || s.find('"') != string::npos || s.find('\n') != string::npos;
    if (!needQuotes) return s;
    string out = "\"";
    for (char c : s) {
        if (c == '"') out += "\"\"";
        else out.push_back(c);
    }
    out += "\"";
    return out;
}

// ---- Vehicle file IO ----
bool saveVehicle(const Vehicle &v) {
    ofstream ofs(VEH_FILE, ios::app);
    if (!ofs) return false;
    ofs << escapeCSV(v.id) << "," << escapeCSV(v.make) << "," << escapeCSV(v.model) << ","
        << v.year << "," << escapeCSV(v.plate) << "," << v.rentPerDay << "," << escapeCSV(v.status) << "\n";
    return true;
}

vector<Vehicle> loadAllVehicles() {
    vector<Vehicle> list;
    ifstream ifs(VEH_FILE);
    if (!ifs) return list;
    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        vector<string> f = splitCSV(line);
        if (f.size() < 7) continue;
        Vehicle v;
        v.id = f[0]; v.make = f[1]; v.model = f[2];
        try { v.year = stoi(f[3]); } catch(...) { v.year = 0; }
        v.plate = f[4];
        try { v.rentPerDay = stod(f[5]); } catch(...) { v.rentPerDay = 0.0; }
        v.status = f[6];
        list.push_back(v);
    }
    return list;
}

bool overwriteVehicles(const vector<Vehicle> &list) {
    ofstream ofs(TEMP_FILE);
    if (!ofs) return false;
    for (const auto &v : list) {
        ofs << escapeCSV(v.id) << "," << escapeCSV(v.make) << "," << escapeCSV(v.model) << ","
            << v.year << "," << escapeCSV(v.plate) << "," << v.rentPerDay << "," << escapeCSV(v.status) << "\n";
    }
    ofs.close();
    // replace original file
    if (remove(VEH_FILE.c_str()) != 0) {
        // maybe original didn't exist, ignore
    }
    if (rename(TEMP_FILE.c_str(), VEH_FILE.c_str()) != 0) {
        // rename failed
        return false;
    }
    return true;
}

// ---- Sales file IO ----
bool saveSale(const Sale &s) {
    ofstream ofs(SALES_FILE, ios::app);
    if (!ofs) return false;
    ofs << escapeCSV(s.saleId) << "," << escapeCSV(s.vehicleId) << "," << escapeCSV(s.vehicleDesc) << ","
        << escapeCSV(s.customerName) << "," << s.days << "," << s.total << "," << escapeCSV(s.dateTime) << "\n";
    return true;
}

vector<Sale> loadAllSales() {
    vector<Sale> list;
    ifstream ifs(SALES_FILE);
    if (!ifs) return list;
    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;
        vector<string> f = splitCSV(line);
        if (f.size() < 7) continue;
        Sale s;
        s.saleId = f[0]; s.vehicleId = f[1]; s.vehicleDesc = f[2];
        s.customerName = f[3];
        try { s.days = stoi(f[4]); } catch(...) { s.days = 0; }
        try { s.total = stod(f[5]); } catch(...) { s.total = 0.0; }
        s.dateTime = f[6];
        list.push_back(s);
    }
    return list;
}

// ---- Input helpers ----
string inputLineNonEmpty(const string &prompt) {
    string s;
    do {
        cout << prompt;
        getline(cin, s);
        if (s.empty()) cout << "Input cannot be empty. Try again.\n";
    } while (s.empty());
    return s;
}

int inputInt(const string &prompt) {
    string s;
    int val;
    while (true) {
        cout << prompt;
        getline(cin, s);
        try {
            size_t pos;
            val = stoi(s, &pos);
            if (pos != s.size()) throw invalid_argument("extra chars");
            return val;
        } catch(...) {
            cout << "Invalid integer. Try again.\n";
        }
    }
}

double inputDouble(const string &prompt) {
    string s;
    double val;
    while (true) {
        cout << prompt;
        getline(cin, s);
        try {
            size_t pos;
            val = stod(s, &pos);
            if (pos != s.size()) throw invalid_argument("extra chars");
            return val;
        } catch(...) {
            cout << "Invalid number. Try again.\n";
        }
    }
}

// ---- Core features ----
void viewVehicles() {
    auto all = loadAllVehicles();
    if (all.empty()) { cout << "No vehicles found.\n"; return; }
    cout << left << setw(8) << "ID" << setw(14) << "Make" << setw(14) << "Model" << setw(6) << "Year"
         << setw(14) << "Plate" << setw(10) << "Rent/day" << setw(12) << "Status" << "\n";
    cout << string(80, '-') << "\n";
    for (const auto &v : all) {
        cout << left << setw(8) << v.id << setw(14) << v.make << setw(14) << v.model << setw(6) << v.year
             << setw(14) << v.plate << setw(10) << fixed << setprecision(2) << v.rentPerDay << setw(12) << v.status << "\n";
    }
}

void addVehicle() {
    cout << "\n--- Add Vehicle ---\n";
    Vehicle v;
    v.id = inputLineNonEmpty("Enter vehicle ID (unique): ");
    auto list = loadAllVehicles();
    for (const auto &x : list) if (x.id == v.id) {
        cout << "Vehicle ID already exists! Aborting add.\n"; return;
    }
    v.make = inputLineNonEmpty("Make: ");
    v.model = inputLineNonEmpty("Model: ");
    v.year = inputInt("Year: ");
    v.plate = inputLineNonEmpty("Plate number: ");
    v.rentPerDay = inputDouble("Rent per day (e.g., 2500.00): ");
    v.status = "available";
    if (saveVehicle(v)) cout << "Vehicle saved successfully.\n";
    else cout << "Error saving vehicle.\n";
}

void updateVehicle() {
    cout << "\n--- Update Vehicle ---\n";
    string vid = inputLineNonEmpty("Enter vehicle ID to update: ");
    auto list = loadAllVehicles();
    bool found = false;
    for (auto &v : list) {
        if (v.id == vid) {
            found = true;
            cout << "Press ENTER to keep current value.\n";
            cout << "Current Make: " << v.make << "\nNew Make: "; string s; getline(cin, s); if (!s.empty()) v.make = s;
            cout << "Current Model: " << v.model << "\nNew Model: "; getline(cin, s); if (!s.empty()) v.model = s;
            cout << "Current Year: " << v.year << "\nNew Year: "; getline(cin, s); if (!s.empty()) {
                try { v.year = stoi(s); } catch(...) {}
            }
            cout << "Current Plate: " << v.plate << "\nNew Plate: "; getline(cin, s); if (!s.empty()) v.plate = s;
            cout << "Current RentPerDay: " << v.rentPerDay << "\nNew RentPerDay: "; getline(cin, s); if (!s.empty()) {
                try { v.rentPerDay = stod(s); } catch(...) {}
            }
            cout << "Current Status: " << v.status << "\nNew Status (available/rented): "; getline(cin, s); if (!s.empty()) v.status = s;
            break;
        }
    }
    if (!found) { cout << "Vehicle not found.\n"; return; }
    if (overwriteVehicles(list)) cout << "Vehicle updated successfully.\n";
    else cout << "Error updating vehicle.\n";
}

void deleteVehicle() {
    cout << "\n--- Delete Vehicle ---\n";
    string vid = inputLineNonEmpty("Enter vehicle ID to delete: ");
    auto list = loadAllVehicles();
    bool found=false;
    for (const auto &v: list) if (v.id==vid) { found=true; break; }
    if (!found) { cout << "Vehicle not found.\n"; return; }
    cout << "Are you sure to delete vehicle " << vid << " ? (Y/N): ";
    string choice; getline(cin, choice);
    if (choice.empty() || (choice[0]!='Y' && choice[0]!='y')) { cout << "Deletion cancelled.\n"; return; }
    vector<Vehicle> newlist;
    for (auto &v: list) if (v.id != vid) newlist.push_back(v);
    if (overwriteVehicles(newlist)) cout << "Deleted successfully.\n";
    else cout << "Error deleting.\n";
}

string genId(const string &prefix) {
    // Use time + random to reduce collisions
    auto now = chrono::system_clock::now();
    long long sec = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count();
    static std::mt19937_64 rng((unsigned)chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(100, 999);
    return prefix + to_string(sec) + to_string(dist(rng));
}

void createSale() {
    cout << "\n--- Create Sale / Rental ---\n";
    auto vehicles = loadAllVehicles();
    vector<Vehicle> avail;
    for (const auto &v: vehicles) if (v.status == "available") avail.push_back(v);
    if (avail.empty()) { cout << "No available vehicles for rent.\n"; return; }
    cout << "Available vehicles:\n";
    for (const auto &v: avail) cout << v.id << " => " << v.make << " " << v.model << " | Rent/day: " << v.rentPerDay << "\n";
    string vid = inputLineNonEmpty("Enter Vehicle ID to rent: ");
    bool ok=false; Vehicle sel;
    for (auto &v: vehicles) if (v.id==vid && v.status=="available") { ok=true; sel = v; break; }
    if (!ok) { cout << "Vehicle not available or not found.\n"; return; }
    string cust = inputLineNonEmpty("Customer name: ");
    int days = inputInt("Number of days: ");
    double total = days * sel.rentPerDay;
    cout << "Total charge: " << fixed << setprecision(2) << total << "\n";
    string confirm;
    cout << "Confirm booking? (Y/N): "; getline(cin, confirm);
    if (confirm.empty() || (confirm[0]!='Y' && confirm[0]!='y')) { cout << "Booking cancelled.\n"; return; }
    // mark vehicle as rented
    for (auto &v: vehicles) if (v.id==vid) { v.status = "rented"; break; }
    if (!overwriteVehicles(vehicles)) { cout << "Error updating vehicle status.\n"; return; }
    Sale s;
    s.saleId = genId("S");
    s.vehicleId = sel.id;
    s.vehicleDesc = sel.make + " " + sel.model + " (" + sel.plate + ")";
    s.customerName = cust;
    s.days = days;
    s.total = total;
    s.dateTime = nowToString();
    if (saveSale(s)) cout << "Sale recorded. Sale ID: " << s.saleId << "\n";
    else cout << "Error saving sale.\n";
}

void viewSales() {
    auto list = loadAllSales();
    if (list.empty()) { cout << "No sales found.\n"; return; }
    cout << left << setw(12) << "SaleID" << setw(10) << "VehID" << setw(22) << "Customer" << setw(6) << "Days" << setw(10) << "Total" << "DateTime\n";
    cout << string(80, '-') << "\n";
    for (const auto &s: list) {
        cout << left << setw(12) << s.saleId << setw(10) << s.vehicleId << setw(22) << s.customerName
             << setw(6) << s.days << setw(10) << fixed << setprecision(2) << s.total << s.dateTime << "\n";
    }
}

void manageSalesMenu() {
    while (true) {
        cout << "\n--- Sales Menu ---\n1. Create Sale (Rent)\n2. View Sales\n3. Back to Main Menu\nChoice: ";
        string ch; getline(cin, ch);
        if (ch=="1") createSale();
        else if (ch=="2") viewSales();
        else if (ch=="3") break;
        else cout << "Invalid choice.\n";
    }
}

void viewCompanyDetails() {
    cout << "\n--- TourMate Company Details ---\n";
    cout << "TourMate (Pvt) Ltd\nAddress: No. 12, City Road\nContact: +94 77 123 4567\nEmail: support@tourmate.lk\nServices: Cab rentals, Chauffeur services, Corporate accounts\n\n";
}

bool authenticate() {
    const string USER = "admin";
    const string PASS = "admin123";
    int attempts = 0;
    while (attempts < 3) {
        cout << "Username: "; string user; getline(cin, user);
        cout << "Password: "; string pass; getline(cin, pass);
        if (user == USER && pass == PASS) return true;
        attempts++;
        cout << "Invalid credentials. Attempts left: " << (3 - attempts) << "\n";
    }
    return false;
}

void backupFiles() {
    ifstream src1(VEH_FILE, ios::binary), src2(SALES_FILE, ios::binary);
    ofstream dst1("vehicles_backup.txt", ios::binary), dst2("sales_backup.txt", ios::binary);
    if (src1 && dst1) dst1 << src1.rdbuf();
    if (src2 && dst2) dst2 << src2.rdbuf();
    cout << "Backups created (vehicles_backup.txt, sales_backup.txt).\n";
}

void mainMenu() {
    while (true) {
        cout << "\n=== TourMate Vehicle Management ===\n";
        cout << "1. View Vehicles\n2. Add Vehicle\n3. Update Vehicle\n4. Delete Vehicle\n5. Manage Sales\n6. View Company Details\n7. Backup Files\n8. Logout\n9. Exit\nChoice: ";
        string choice; getline(cin, choice);
        if (choice=="1") viewVehicles();
        else if (choice=="2") addVehicle();
        else if (choice=="3") updateVehicle();
        else if (choice=="4") deleteVehicle();
        else if (choice=="5") manageSalesMenu();
        else if (choice=="6") viewCompanyDetails();
        else if (choice=="7") backupFiles();
        else if (choice=="8") { cout << "Logging out...\n"; break; }
        else if (choice=="9") { cout << "Exiting application...\n"; exit(0); }
        else cout << "Invalid choice. Try again.\n";
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << "Welcome to TourMate Vehicle Management System\n";
    if (!authenticate()) { cout << "Authentication failed. Exiting.\n"; return 0; }
    cout << "Login successful.\n";
    mainMenu();
    cout << "Thank you for using TourMate. Goodbye.\n";
    return 0;
}

