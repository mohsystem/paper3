#include <bits/stdc++.h>
using namespace std;

class Lse145 {
public:
    struct Record {
        int id;
        string stock;
        int qty;
        long long ts;
    };

    static string buy_function(const string& stockName) {
        string s = stockName;
        // trim
        auto l = s.find_first_not_of(" \t\n\r");
        auto r = s.find_last_not_of(" \t\n\r");
        if (l == string::npos) return "";
        s = s.substr(l, r - l + 1);
        if (s.empty()) return "";
        for (auto& ch : s) ch = toupper(static_cast<unsigned char>(ch));
        return s;
    }

    static int buy_stock(const string& stockName, int quantity) {
        string normalized = buy_function(stockName);
        if (normalized.empty() || quantity <= 0) return -1;
        Record rec;
        rec.id = nextId++;
        rec.stock = normalized;
        rec.qty = quantity;
        rec.ts = static_cast<long long>(time(nullptr));
        db.push_back(rec);
        return rec.id;
    }

    static vector<Record> get_all_records() {
        return db;
    }

private:
    static inline vector<Record> db;
    static inline int nextId = 1;
};

int main() {
    int id1 = Lse145::buy_stock("AAPL", 10);
    int id2 = Lse145::buy_stock("goog", 5);
    int id3 = Lse145::buy_stock("  msft  ", 20);
    int id4 = Lse145::buy_stock("", 7);      // invalid
    int id5 = Lse145::buy_stock("TSLA", -3); // invalid

    cout << "Inserted IDs: " << id1 << " " << id2 << " " << id3 << " " << id4 << " " << id5 << "\n";
    cout << "Database contents:\n";
    for (const auto& r : Lse145::get_all_records()) {
        cout << "ID=" << r.id << " Stock=" << r.stock << " Qty=" << r.qty << " Ts=" << r.ts << "\n";
    }
    return 0;
}