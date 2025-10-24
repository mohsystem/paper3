#include <bits/stdc++.h>
using namespace std;

// Simple in-memory "database"
struct User {
    int id;
    string name;
    string email;
    int age;
};

static vector<User> DB_USERS;
static int DB_NEXT_ID = 1;

// Public API functions
int addUser(const string& name, const string& email, int age) {
    User u{DB_NEXT_ID++, name, email, age};
    DB_USERS.push_back(u);
    return u.id;
}

string escapeJson(const string& s) {
    string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        if (c == '\\') out += "\\\\";
        else if (c == '\"') out += "\\\"";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else out += c;
    }
    return out;
}

string getAllUsersJson() {
    string out = "[";
    for (size_t i = 0; i < DB_USERS.size(); ++i) {
        const auto& u = DB_USERS[i];
        out += "{\"id\":" + to_string(u.id) +
               ",\"name\":\"" + escapeJson(u.name) + "\"" +
               ",\"email\":\"" + escapeJson(u.email) + "\"" +
               ",\"age\":" + to_string(u.age) + "}";
        if (i + 1 < DB_USERS.size()) out += ",";
    }
    out += "]";
    return out;
}

// Very naive JSON parser for keys "name","email","age"
static bool extractString(const string& json, const string& key, string& value) {
    string pat = "\"" + key + "\"";
    size_t pos = json.find(pat);
    if (pos == string::npos) return false;
    pos = json.find(':', pos);
    if (pos == string::npos) return false;
    pos = json.find('"', pos);
    if (pos == string::npos) return false;
    size_t end = json.find('"', pos + 1);
    if (end == string::npos) return false;
    value = json.substr(pos + 1, end - (pos + 1));
    return true;
}

static bool extractInt(const string& json, const string& key, int& value) {
    string pat = "\"" + key + "\"";
    size_t pos = json.find(pat);
    if (pos == string::npos) return false;
    pos = json.find(':', pos);
    if (pos == string::npos) return false;
    // skip spaces
    while (pos < json.size() && (json[pos] == ':' || isspace((unsigned char)json[pos]))) pos++;
    // read integer
    bool neg = false;
    if (pos < json.size() && json[pos] == '-') { neg = true; pos++; }
    if (pos >= json.size() || !isdigit((unsigned char)json[pos])) return false;
    long v = 0;
    while (pos < json.size() && isdigit((unsigned char)json[pos])) {
        v = v * 10 + (json[pos] - '0');
        pos++;
    }
    value = (int)(neg ? -v : v);
    return true;
}

// Simulated API endpoint handler
string handleRequest(const string& method, const string& path, const string& body) {
    if (method == "POST" && path == "/users") {
        string name, email;
        int age;
        if (!extractString(body, "name", name) || !extractString(body, "email", email) || !extractInt(body, "age", age)) {
            return string("{\"error\":\"Invalid input\"}");
        }
        int id = addUser(name, email, age);
        return string("{\"status\":\"ok\",\"id\":") + to_string(id) + "}";
    } else if (method == "GET" && path == "/users") {
        return getAllUsersJson();
    }
    return string("{\"error\":\"Not found\"}");
}

int main() {
    // 5 test cases - POST users
    vector<string> payloads = {
        "{\"name\":\"Alice\",\"email\":\"alice@example.com\",\"age\":30}",
        "{\"name\":\"Bob\",\"email\":\"bob@example.com\",\"age\":25}",
        "{\"name\":\"Carol\",\"email\":\"carol@example.com\",\"age\":28}",
        "{\"name\":\"Dave\",\"email\":\"dave@example.com\",\"age\":40}",
        "{\"name\":\"Eve\",\"email\":\"eve@example.com\",\"age\":22}"
    };
    for (auto& p : payloads) {
        cout << handleRequest("POST", "/users", p) << "\n";
    }
    // GET users
    cout << handleRequest("GET", "/users", "") << "\n";
    return 0;
}