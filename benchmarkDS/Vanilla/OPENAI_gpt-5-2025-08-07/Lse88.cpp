#include <bits/stdc++.h>
using namespace std;

// Trim helper
static inline string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Entry point function: receives YAML payload as string and returns total price as string
string calculate_price(const string& yaml_payload) {
    string product_code;
    int quantity = 0;

    unordered_map<string, double> prices{
        {"P001", 10.0},
        {"P002", 15.5},
        {"P003", 7.25},
        {"P004", 100.0},
        {"P005", 0.99}
    };

    stringstream ss(yaml_payload);
    string line;
    while (getline(ss, line)) {
        string t = trim(line);
        if (t.empty() || (!t.empty() && t[0] == '#')) continue;
        size_t pos = t.find(':');
        if (pos == string::npos) continue;
        string key = trim(t.substr(0, pos));
        string value = trim(t.substr(pos + 1));
        if (key == "product_code") {
            product_code = value;
        } else if (key == "quantity") {
            try {
                quantity = stoi(value);
            } catch (...) {
                quantity = 0;
            }
        }
    }

    double price = 0.0;
    auto it = prices.find(product_code);
    if (it != prices.end()) price = it->second;
    double total = price * max(0, quantity);

    ostringstream out;
    out.setf(std::ios::fixed); 
    out << setprecision(2) << total;
    return out.str();
}

int main() {
    vector<string> tests = {
        "product_code: P001\nquantity: 3",
        "quantity: 2\nproduct_code: P002",
        "product_code: P003\nquantity: 4",
        "product_code: P999\nquantity: 1",
        "product_code: P005\nquantity: 5"
    };
    for (auto& t : tests) {
        cout << calculate_price(t) << "\n";
    }
    return 0;
}