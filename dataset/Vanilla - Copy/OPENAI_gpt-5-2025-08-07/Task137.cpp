#include <bits/stdc++.h>
using namespace std;

struct Product {
    int id;
    string name;
    string category;
    double price;
    int stock;
};

static const vector<Product> PRODUCT_TABLE = {
    {1001, "Widget", "Tools", 19.99, 120},
    {1002, "Gadget", "Electronics", 99.50, 55},
    {1003, "Gizmo", "Electronics", 49.00, 200},
    {1004, "Pro Headphones", "Audio", 129.99, 35},
    {1005, "Office Chair", "Furniture", 199.99, 15},
    {1006, "Pro Keyboard", "Computers", 89.99, 60},
};

static string toLowerStr(const string& s) {
    string t = s;
    transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return (char)tolower(c); });
    return t;
}

static string jsonEscape(const string& s) {
    string out;
    out.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '\"': out += "\\\""; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += (char)c;
                }
        }
    }
    return out;
}

static string productsToJson(const vector<Product>& v) {
    ostringstream oss;
    oss.setf(std::ios::fixed); 
    oss << setprecision(2);
    oss << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        const auto& p = v[i];
        oss << "{"
            << "\"id\":" << p.id << ","
            << "\"name\":\"" << jsonEscape(p.name) << "\","
            << "\"category\":\"" << jsonEscape(p.category) << "\","
            << "\"price\":" << p.price << ","
            << "\"stock\":" << p.stock
            << "}";
        if (i + 1 < v.size()) oss << ",";
    }
    oss << "]";
    return oss.str();
}

string processInput(const string& input) {
    string s = input;
    // trim
    auto l = s.find_first_not_of(" \t\r\n");
    auto r = s.find_last_not_of(" \t\r\n");
    if (l == string::npos) return "[]";
    s = s.substr(l, r - l + 1);

    string lower = toLowerStr(s);
    vector<Product> res;

    if (lower == "all") {
        res = PRODUCT_TABLE;
        return productsToJson(res);
    }

    auto eq = s.find('=');
    if (eq == string::npos) return "[]";
    string key = s.substr(0, eq);
    string val = s.substr(eq + 1);

    // trim key/val
    auto kl = key.find_first_not_of(" \t\r\n");
    auto kr = key.find_last_not_of(" \t\r\n");
    key = (kl == string::npos) ? "" : key.substr(kl, kr - kl + 1);
    auto vl = val.find_first_not_of(" \t\r\n");
    auto vr = val.find_last_not_of(" \t\r\n");
    val = (vl == string::npos) ? "" : val.substr(vl, vr - vl + 1);

    string keyl = toLowerStr(key);
    string vall = toLowerStr(val);

    if (keyl == "id") {
        try {
            int id = stoi(val);
            for (const auto& p : PRODUCT_TABLE) if (p.id == id) { res.push_back(p); break; }
        } catch (...) { /* no result */ }
    } else if (keyl == "name") {
        for (const auto& p : PRODUCT_TABLE) if (toLowerStr(p.name) == vall) res.push_back(p);
    } else if (keyl == "category") {
        for (const auto& p : PRODUCT_TABLE) if (toLowerStr(p.category) == vall) res.push_back(p);
    } else if (keyl == "search") {
        for (const auto& p : PRODUCT_TABLE) if (toLowerStr(p.name).find(vall) != string::npos) res.push_back(p);
    } else {
        // invalid -> empty
    }

    return productsToJson(res);
}

int main() {
    vector<string> tests = {
        "id=1002",
        "name=Gizmo",
        "category=Audio",
        "search=Pro",
        "all"
    };
    for (const auto& t : tests) {
        cout << "Query: " << t << "\n";
        cout << processInput(t) << "\n";
    }
    return 0;
}