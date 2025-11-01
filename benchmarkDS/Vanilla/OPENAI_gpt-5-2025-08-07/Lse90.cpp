#include <bits/stdc++.h>
using namespace std;

class Lse90 {
public:
    static double get_price_for_product_code(const string& product_code) {
        static const unordered_map<string, double> price_map = {
            {"A100", 10.00},
            {"B200", 5.50},
            {"C300", 2.25},
            {"D400", 12.75},
            {"E500", 99.99}
        };
        auto it = price_map.find(trim(product_code));
        if (it != price_map.end()) return it->second;
        return 0.0;
    }

    static string calculate_price(const string& yaml_query_param_payload) {
        auto data = parse_yaml(yaml_query_param_payload);
        string code = (data.count("product_code") ? data["product_code"] : "");
        int quantity = 0;
        if (data.count("quantity")) {
            try {
                quantity = stoi(data["quantity"]);
            } catch (...) {
                quantity = 0;
            }
        }
        double price = get_price_for_product_code(code);
        double total = price * quantity;
        ostringstream oss;
        oss.setf(std::ios::fixed); 
        oss << setprecision(2);
        oss << "total_price: " << total;
        return oss.str();
    }

private:
    static string trim(const string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static unordered_map<string, string> parse_yaml(const string& yaml) {
        unordered_map<string, string> m;
        stringstream ss(yaml);
        string line;
        while (getline(ss, line)) {
            auto pos = line.find(':');
            if (pos != string::npos) {
                string key = trim(line.substr(0, pos));
                string val = trim(line.substr(pos + 1));
                m[key] = val;
            }
        }
        return m;
    }
};

int main() {
    vector<string> tests = {
        "product_code: A100\nquantity: 3",
        "product_code: B200\nquantity: 2",
        "product_code: C300\nquantity: 0",
        "product_code: UNKNOWN\nquantity: 5",
        "quantity: 4\nproduct_code: D400"
    };
    for (const auto& t : tests) {
        cout << Lse90::calculate_price(t) << endl;
    }
    return 0;
}