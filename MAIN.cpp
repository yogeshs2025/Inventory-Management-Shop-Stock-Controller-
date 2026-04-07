/*
Project Title: Inventory Management – “Shop Stock Controller”
Student Name: Yogesh S
Register No: 25BCE1650
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <limits>
#include <cstdlib>

using namespace std;

#ifdef _WIN32
  #include <windows.h>
  void enableANSI() {
      HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
      DWORD mode = 0;
      GetConsoleMode(h, &mode);
      SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }
#else
  void enableANSI() {}
#endif

#define RESET    "\033[0m"
#define BOLD     "\033[1m"
#define DIM      "\033[2m"
#define BRED     "\033[91m"
#define BGREEN   "\033[92m"
#define BYELLOW  "\033[93m"
#define BBLUE    "\033[94m"
#define BMAGENTA "\033[95m"
#define BCYAN    "\033[96m"
#define BWHITE   "\033[97m"
#define WHITE    "\033[37m"
#define CYAN     "\033[36m"
#define BG_GREEN "\033[42m"
#define BG_RED   "\033[41m"

const int LOW_STOCK_THRESHOLD = 5;
const string DATA_FILE = "inventory.dat";


void printLine(char ch = '-', int len = 64) {
    cout << CYAN;
    for (int i = 0; i < len; i++) cout << ch;
    cout << RESET << "\n";
}

void printDoubleLine(int len = 64) {
    cout << BBLUE;
    for (int i = 0; i < len; i++) cout << "=";
    cout << RESET << "\n";
}

void clearScreen() {
    cout << "\033[2J\033[H";
}

void printHeader(const string& title) {
    cout << "\n";
    printDoubleLine();
    int pad = (64 - (int)title.size()) / 2;
    cout << BBLUE << "|" << RESET
         << string(pad, ' ')
         << BOLD << BWHITE << title << RESET
         << string(64 - pad - (int)title.size(), ' ')
         << BBLUE << "|" << RESET << "\n";
    printDoubleLine();
}

void printSuccess(const string& msg) {
    cout << "\n  " << BGREEN << "[ OK  ] " << BOLD << msg << RESET << "\n";
}
void printError(const string& msg) {
    cout << "\n  " << BRED   << "[FAIL ] " << BOLD << msg << RESET << "\n";
}
void printWarning(const string& msg) {
    cout << "\n  " << BYELLOW << "[WARN ] " << msg << RESET << "\n";
}
void printInfo(const string& msg) {
    cout << "  " << BCYAN << "[INFO ] " << msg << RESET << "\n";
}

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}
void pauseScreen() {
    cout << "\n  " << DIM << "Press ENTER to continue..." << RESET;
    clearInput();
}

class Product {
private:
    string productID;
    string name;
    double price;
    int    quantity;

public:
    Product() : productID(""), name(""), price(0.0), quantity(0) {}

    Product(string id, string n, double p, int q)
        : productID(id), name(n), price(p), quantity(q) {}

    string getID()       const { return productID; }
    string getName()     const { return name;      }
    double getPrice()    const { return price;     }
    int    getQuantity() const { return quantity;  }
    double getValue()    const { return price * quantity; }

    void setPrice(double p)  { price    = p; }
    void setQuantity(int q)  { quantity = q; }
    void setName(string n)   { name     = n; }

    bool sell(int qty) {
        if (qty > quantity) return false;
        quantity -= qty;
        return true;
    }
    void purchase(int qty) { quantity += qty; }
    bool isLowStock() const { return quantity <= LOW_STOCK_THRESHOLD; }

    string serialize() const {
        ostringstream oss;
        oss << productID << "|" << name << "|"
            << fixed << setprecision(2) << price << "|" << quantity;
        return oss.str();
    }

    static Product deserialize(const string& line) {
        istringstream iss(line);
        string id, nm, ps, qs;
        getline(iss, id, '|');
        getline(iss, nm, '|');
        getline(iss, ps, '|');
        getline(iss, qs, '|');
        return Product(id, nm, stod(ps), stoi(qs));
    }

    void printRow(bool highlight = false) const {
        string bg    = isLowStock() ? BG_RED   : BG_GREEN;
        string label = isLowStock() ? " LOW  "  : "  OK  ";
        cout << (highlight ? BYELLOW : WHITE);
        cout << "  | " << left  << setw(10) << productID
             << " | "  << setw(22) << name
             << " | "  << BGREEN << "Rs." << right << setw(8)
             << fixed  << setprecision(2) << price << RESET
             << (highlight ? BYELLOW : WHITE)
             << " | "  << setw(5) << quantity
             << " | "  << bg << BOLD << WHITE << label << RESET
             << (highlight ? BYELLOW : WHITE) << " |" << RESET << "\n";
    }

    static void printTableHeader() {
        printLine('-', 68);
        cout << BOLD << BCYAN
             << "  | " << left  << setw(10) << "ID"
             << " | "  << setw(22) << "Product Name"
             << " | "  << setw(11) << "    Price"
             << " | "  << setw(5)  << "Qty"
             << " | "  << setw(6)  << "Stock"
             << " |"   << RESET << "\n";
        printLine('-', 68);
    }
};


class Inventory {
private:
    vector<Product> products;

    int findIndexByID(const string& id) const {
        for (int i = 0; i < (int)products.size(); i++)
            if (products[i].getID() == id) return i;
        return -1;
    }

public:
    Inventory()  { loadFromFile(); }
    ~Inventory() { saveToFile();   }

    
    void addProduct() {
        printHeader("  ADD NEW PRODUCT  ");
        string id, name;
        double price; int qty;

        cout << "\n  " << BYELLOW << "Enter Product ID    : " << RESET;
        cin >> id; clearInput();

        if (findIndexByID(id) != -1) {
            printError("Product ID '" + id + "' already exists!");
            pauseScreen(); return;
        }

        cout << "  " << BYELLOW << "Enter Product Name  : " << RESET;
        getline(cin, name);
        if (name.empty()) { printError("Name cannot be empty."); pauseScreen(); return; }

        cout << "  " << BYELLOW << "Enter Price (Rs.)   : " << RESET;
        while (!(cin >> price) || price < 0) {
            printError("Enter a valid positive price.");
            clearInput();
            cout << "  " << BYELLOW << "Enter Price (Rs.)   : " << RESET;
        }

        cout << "  " << BYELLOW << "Enter Initial Qty   : " << RESET;
        while (!(cin >> qty) || qty < 0) {
            printError("Enter a valid quantity.");
            clearInput();
            cout << "  " << BYELLOW << "Enter Initial Qty   : " << RESET;
        }
        clearInput();

        products.push_back(Product(id, name, price, qty));
        saveToFile();
        printSuccess("Product '" + name + "' added successfully!");
        pauseScreen();
    }

    
    void purchaseStock() {
        printHeader("  PURCHASE STOCK  ");
        string id; int qty;

        cout << "\n  " << BYELLOW << "Enter Product ID : " << RESET;
        cin >> id;
        int idx = findIndexByID(id);
        if (idx == -1) {
            printError("Product not found: " + id);
            clearInput(); pauseScreen(); return;
        }
        cout << "  " << BCYAN << "Product     : " << BOLD << products[idx].getName() << RESET << "\n";
        cout << "  " << BCYAN << "Current Qty : " << products[idx].getQuantity() << RESET << "\n";
        cout << "  " << BYELLOW << "Qty to add  : " << RESET;
        while (!(cin >> qty) || qty <= 0) {
            printError("Enter a positive quantity.");
            clearInput();
            cout << "  " << BYELLOW << "Qty to add  : " << RESET;
        }
        clearInput();
        products[idx].purchase(qty);
        saveToFile();
        printSuccess("Stock updated! New quantity: " + to_string(products[idx].getQuantity()));
        pauseScreen();
    }

    
    void sellStock() {
        printHeader("  SELL STOCK  ");
        string id; int qty;

        cout << "\n  " << BYELLOW << "Enter Product ID : " << RESET;
        cin >> id;
        int idx = findIndexByID(id);
        if (idx == -1) {
            printError("Product not found: " + id);
            clearInput(); pauseScreen(); return;
        }
        cout << "  " << BCYAN << "Product   : " << BOLD << products[idx].getName() << RESET << "\n";
        cout << "  " << BCYAN << "Available : " << products[idx].getQuantity() << RESET << "\n";
        cout << "  " << BYELLOW << "Qty to sell : " << RESET;
        while (!(cin >> qty) || qty <= 0) {
            printError("Enter a positive quantity.");
            clearInput();
            cout << "  " << BYELLOW << "Qty to sell : " << RESET;
        }
        clearInput();

        if (!products[idx].sell(qty)) {
            printError("Insufficient stock! Available only: " + to_string(products[idx].getQuantity()));
            pauseScreen(); return;
        }
        saveToFile();
        printSuccess("Sale complete! Remaining: " + to_string(products[idx].getQuantity()));
        if (products[idx].isLowStock())
            printWarning("Stock is LOW for '" + products[idx].getName() + "'. Restock soon!");
        cout << "  " << BGREEN << "  Sale Value : Rs." << fixed << setprecision(2)
             << products[idx].getPrice() * qty << RESET << "\n";
        pauseScreen();
    }

    
    void searchProduct() {
        printHeader("  SEARCH PRODUCT  ");
        cout << "\n  " << BYELLOW << "  [1] Search by Product ID\n"
             << "  [2] Search by Name Keyword\n"
             << "  " << BYELLOW << "  Choice : " << RESET;
        int ch; cin >> ch; clearInput();

        vector<int> res;
        if (ch == 1) {
            string id;
            cout << "  " << BYELLOW << "Enter Product ID : " << RESET;
            cin >> id; clearInput();
            int idx = findIndexByID(id);
            if (idx != -1) res.push_back(idx);
        } else if (ch == 2) {
            string kw;
            cout << "  " << BYELLOW << "Enter Keyword : " << RESET;
            getline(cin, kw);
            string kl = kw;
            transform(kl.begin(), kl.end(), kl.begin(), ::tolower);
            for (int i = 0; i < (int)products.size(); i++) {
                string nl = products[i].getName();
                transform(nl.begin(), nl.end(), nl.begin(), ::tolower);
                if (nl.find(kl) != string::npos) res.push_back(i);
            }
        } else { printError("Invalid choice."); pauseScreen(); return; }

        if (res.empty()) {
            printError("No matching products found.");
        } else {
            cout << "\n  " << BGREEN << "Found " << res.size() << " result(s):\n" << RESET;
            Product::printTableHeader();
            for (int i : res) products[i].printRow(true);
            printLine('-', 68);
        }
        pauseScreen();
    }

    
    void deleteProduct() {
        printHeader("  DELETE PRODUCT  ");
        string id;
        cout << "\n  " << BYELLOW << "Enter Product ID to delete : " << RESET;
        cin >> id; clearInput();

        int idx = findIndexByID(id);
        if (idx == -1) { printError("Product not found: " + id); pauseScreen(); return; }

        cout << "\n  " << BRED << "You are about to permanently delete:\n" << RESET;
        Product::printTableHeader();
        products[idx].printRow();
        printLine('-', 68);

        cout << "\n  " << BOLD << BYELLOW << "  Confirm delete? (yes / no) : " << RESET;
        string confirm; getline(cin, confirm);
        transform(confirm.begin(), confirm.end(), confirm.begin(), ::tolower);

        if (confirm == "yes" || confirm == "y") {
            products.erase(products.begin() + idx);
            saveToFile();
            printSuccess("Product deleted successfully.");
        } else {
            printInfo("Deletion cancelled. No changes made.");
        }
        pauseScreen();
    }

    
    void viewAllProducts() {
        printHeader("  ALL PRODUCTS  ");
        if (products.empty()) { printWarning("Inventory is empty."); pauseScreen(); return; }
        Product::printTableHeader();
        for (auto& p : products) p.printRow();
        printLine('-', 68);
        cout << "  " << DIM << "Total Products: " << products.size() << RESET << "\n";
        pauseScreen();
    }

    
    void lowStockReport() {
        printHeader("  [!] LOW STOCK ALERT REPORT  ");
        vector<int> low;
        for (int i = 0; i < (int)products.size(); i++)
            if (products[i].isLowStock()) low.push_back(i);

        if (low.empty()) {
            printSuccess("Great! All products are well stocked.");
        } else {
            cout << "\n  " << BRED << "  Items at or below " << LOW_STOCK_THRESHOLD << " units:\n\n" << RESET;
            Product::printTableHeader();
            for (int i : low) products[i].printRow();
            printLine('-', 68);
            printWarning("Please restock " + to_string(low.size()) + " item(s) urgently!");
        }
        pauseScreen();
    }

    
    void inventoryValueReport() {
        printHeader("  INVENTORY VALUE REPORT  ");
        if (products.empty()) { printWarning("Inventory is empty."); pauseScreen(); return; }

        double total = 0;
        cout << "\n";
        Product::printTableHeader();
        for (auto& p : products) { p.printRow(); total += p.getValue(); }
        printDoubleLine(68);
        cout << "  " << BOLD << BGREEN
             << "  TOTAL INVENTORY VALUE  :  Rs."
             << fixed << setprecision(2) << total << RESET << "\n";
        printDoubleLine(68);
        pauseScreen();
    }

    
    void saveToFile() {
        ofstream file(DATA_FILE);
        if (!file.is_open()) { printError("Could not write to file!"); return; }
        for (auto& p : products) file << p.serialize() << "\n";
        file.close();
    }

    void loadFromFile() {
        ifstream file(DATA_FILE);
        if (!file.is_open()) return;
        string line;
        while (getline(file, line))
            if (!line.empty()) products.push_back(Product::deserialize(line));
        file.close();
    }

    int getProductCount() const { return (int)products.size(); }
};


void printMainMenu(int count) {
    printDoubleLine(64);
    cout << BBLUE << "|" << RESET
         << BOLD << BWHITE << "          SHOP STOCK CONTROLLER  v1.0               " << RESET
         << BBLUE << "|" << RESET << "\n";
    cout << BBLUE << "|" << RESET
         << CYAN  << "            Inventory Management System              " << RESET
         << BBLUE << "|" << RESET << "\n";
    printDoubleLine(64);
    cout << "  " << DIM << "  Products in inventory: " << BWHITE << count << RESET << "\n";

    cout << "\n" << BOLD << BCYAN << "  +------ STOCK OPERATIONS ----------------------------+\n" << RESET;
    cout << "  " << BGREEN  << "  [1]" << WHITE << "  Add New Product\n"   << RESET;
    cout << "  " << BBLUE   << "  [2]" << WHITE << "  Purchase Stock\n"    << RESET;
    cout << "  " << BYELLOW << "  [3]" << WHITE << "  Sell Stock\n"        << RESET;
    cout << "  " << BCYAN   << "  [4]" << WHITE << "  Search Product\n"    << RESET;
    cout << "  " << BRED    << "  [5]" << WHITE << "  Delete Product\n"    << RESET;
    cout << "  " << WHITE   << "  [6]" << WHITE << "  View All Products\n" << RESET;

    cout << "\n" << BOLD << BCYAN << "  +------ REPORTS --------------------------------------+\n" << RESET;
    cout << "  " << BYELLOW << "  [7]" << WHITE << "  Low Stock Alert Report\n" << RESET;
    cout << "  " << BGREEN  << "  [8]" << WHITE << "  Total Inventory Value\n"  << RESET;

    cout << "\n" << BOLD << BCYAN << "  +------ SYSTEM ---------------------------------------+\n" << RESET;
    cout << "  " << BMAGENTA << "  [0]" << WHITE << "  Save & Exit\n" << RESET;

    printLine('-', 64);
    cout << "  " << BOLD << BYELLOW << "  Enter your choice : " << RESET;
}


int main() {
    enableANSI();
    clearScreen();

    printDoubleLine(64);
    cout << BBLUE << "|" << RESET << string(64, ' ') << BBLUE << "|" << RESET << "\n";
    cout << BBLUE << "|" << RESET
         << BOLD << BWHITE << "          SHOP STOCK CONTROLLER  v1.0               " << RESET
         << BBLUE << "|" << RESET << "\n";
    cout << BBLUE << "|" << RESET
         << CYAN  << "            Inventory Management System              " << RESET
         << BBLUE << "|" << RESET << "\n";
    cout << BBLUE << "|" << RESET << string(64, ' ') << BBLUE << "|" << RESET << "\n";
    printDoubleLine(64);

    cout << "\n  " << DIM << "Initializing system and loading saved data...\n" << RESET;

    Inventory inventory;

    cout << "  " << BGREEN << "[ OK  ] System ready. "
         << inventory.getProductCount() << " product(s) loaded from file.\n" << RESET;
    pauseScreen();

    int choice;
    do {
        clearScreen();
        printMainMenu(inventory.getProductCount());
        cin >> choice;
        clearInput();

        switch (choice) {
            case 1: inventory.addProduct();           break;
            case 2: inventory.purchaseStock();        break;
            case 3: inventory.sellStock();            break;
            case 4: inventory.searchProduct();        break;
            case 5: inventory.deleteProduct();        break;
            case 6: inventory.viewAllProducts();      break;
            case 7: inventory.lowStockReport();       break;
            case 8: inventory.inventoryValueReport(); break;
            case 0:
                printDoubleLine(64);
                cout << "  " << BGREEN << BOLD
                     << "  [ OK  ] Data saved. Thank you for using Shop Stock Controller!\n"
                     << RESET;
                printDoubleLine(64);
                cout << "\n";
                break;
            default:
                printError("Invalid choice. Enter a number from 0 to 8.");
                pauseScreen();
        }
    } while (choice != 0);

    return 0;
}
