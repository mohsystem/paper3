// Chain-of-Through process in code generation:
// 1) Understand: Safely parse user-supplied string into structured values.
// 2) Security: Avoid native deserialization; enforce strict schema and limits.
// 3) Implement: Defensive parsing, validation, allowlists.
// 4) Review: No unbounded memory ops; simple checks; clear errors.
// 5) Output: Canonical JSON string or error.
#include <bits/stdc++.h>
using namespace std;

static const size_t MAX_INPUT_LENGTH_CPP = 1024;
static const unordered_set<string> ALLOWED_KEYS_CPP = {"name","age","email","roles"};
static const unordered_set<string> ALLOWED_ROLES_CPP = {"user","admin","moderator","viewer","editor"};

static bool isValidName(const string& s){
    if (s.size() < 1 || s.size() > 50) return false;
    for (unsigned char c : s){
        if (!(isalpha(c) || c==' ' || c=='-' || c=='\'')) return false;
    }
    return true;
}
static optional<int> parseAge(const string& s){
    if (s.empty() || s.size() > 3) return nullopt;
    int val = 0;
    for (char c : s){
        if (!isdigit((unsigned char)c)) return nullopt;
        val = val*10 + (c - '0');
        if (val > 150) return nullopt;
    }
    if (val < 0 || val > 120) return nullopt;
    return val;
}
static bool checkLocal(const string& s){
    if (s.empty()) return false;
    for (unsigned char c : s){
        if (!(isalnum(c) || c=='.' || c=='_' || c=='%' || c=='+' || c=='-')) return false;
    }
    return true;
}
static bool checkDomain(const string& s){
    if (s.size() < 3 || s.size() > 100) return false;
    if (s.front()=='.' || s.back()=='.') return false;
    if (s.find("..") != string::npos) return false;
    for (unsigned char c : s){
        if (!(isalnum(c) || c=='.' || c=='-')) return false;
    }
    auto pos = s.rfind('.');
    if (pos == string::npos || pos == 0 || pos == s.size()-1) return false;
    string tld = s.substr(pos+1);
    if (tld.size() < 2 || tld.size() > 10) return false;
    for (unsigned char c : tld){
        if (!isalpha(c)) return false;
    }
    return true;
}
static bool isValidEmail(const string& s){
    if (s.size() < 6 || s.size() > 100) return false;
    size_t at1 = s.find('@');
    if (at1 == string::npos) return false;
    if (s.find('@', at1+1) != string::npos) return false;
    if (at1 == 0 || at1 >= s.size()-1) return false;
    string local = s.substr(0, at1);
    string domain = s.substr(at1+1);
    return checkLocal(local) && checkDomain(domain);
}
static optional<vector<string>> parseRoles(const string& s){
    if (s.empty() || s.size() > 200) return nullopt;
    vector<string> parts;
    string cur;
    for (char c : s){
        if (c == ','){
            if (cur.empty()) return nullopt;
            parts.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) parts.push_back(cur);
    if (parts.empty() || parts.size() > 5) return nullopt;
    vector<string> out;
    unordered_set<string> seen;
    for (auto &r : parts){
        // trim
        size_t b = r.find_first_not_of(" \t\r\n");
        size_t e = r.find_last_not_of(" \t\r\n");
        string role = (b==string::npos)? string() : r.substr(b, e-b+1);
        if (role.empty()) return nullopt;
        if (!ALLOWED_ROLES_CPP.count(role)) return nullopt;
        if (!seen.count(role)){
            seen.insert(role);
            out.push_back(role);
        }
    }
    return out;
}
static string escapeJson(const string& s){
    string out; out.reserve(s.size()+8);
    for (unsigned char c : s){
        switch(c){
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20){
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

string deserializeUserData(const string& input){
    if (input.empty()) return "ERROR: empty input";
    if (input.size() > MAX_INPUT_LENGTH_CPP) return "ERROR: input too long";
    unordered_map<string,string> kv;
    // split by ';'
    size_t start = 0;
    int fieldCount = 0;
    while (start <= input.size()){
        size_t pos = input.find(';', start);
        string seg = (pos==string::npos)? input.substr(start) : input.substr(start, pos-start);
        if (!seg.empty()){
            fieldCount++;
            if (fieldCount > 16) return "ERROR: too many fields";
            size_t eq = seg.find('=');
            if (eq == string::npos || eq==0 || eq==seg.size()-1) return "ERROR: invalid key=value pair";
            string key = seg.substr(0, eq);
            string val = seg.substr(eq+1);
            // trim
            auto trim = [](string &x){
                size_t b = x.find_first_not_of(" \t\r\n");
                size_t e = x.find_last_not_of(" \t\r\n");
                if (b==string::npos){ x.clear(); return; }
                x = x.substr(b, e-b+1);
            };
            trim(key); trim(val);
            if (!ALLOWED_KEYS_CPP.count(key)) return "ERROR: unknown key: " + key;
            if (kv.count(key)) return "ERROR: duplicate key: " + key;
            if (val.empty()) return "ERROR: empty value for " + key;
            kv[key] = val;
        }
        if (pos == string::npos) break;
        start = pos + 1;
    }
    for (auto &req : ALLOWED_KEYS_CPP){
        if (!kv.count(req)) return "ERROR: missing required field: " + req;
    }
    string name = kv["name"];
    string ageStr = kv["age"];
    string email = kv["email"];
    string rolesStr = kv["roles"];
    if (!isValidName(name)) return "ERROR: invalid name";
    auto ageOpt = parseAge(ageStr);
    if (!ageOpt) return "ERROR: invalid age";
    if (!isValidEmail(email)) return "ERROR: invalid email";
    auto rolesOpt = parseRoles(rolesStr);
    if (!rolesOpt) return "ERROR: invalid roles";
    const auto& roles = *rolesOpt;

    // Build JSON
    string out = "{\"name\":\"" + escapeJson(name) + "\",\"age\":" + to_string(*ageOpt) +
                 ",\"email\":\"" + escapeJson(email) + "\",\"roles\":[";
    for (size_t i=0;i<roles.size();i++){
        if (i) out += ",";
        out += "\"" + roles[i] + "\"";
    }
    out += "]}";
    return out;
}

int main(){
    vector<string> tests = {
        "name=Alice Johnson;age=30;email=alice.j@example.com;roles=user,editor",
        "name=Bob;age=45;email=bob@example.org;roles=admin,moderator,viewer",
        "name=Carol;age=abc;email=carol@example.com;roles=user",
        "name=Dan;age=25;email=dan@bad_domain;roles=user",
        "name=Eve;age=29;email=eve@example.com;roles=user;token=xyz"
    };
    for (auto &t : tests){
        cout << deserializeUserData(t) << "\n";
    }
    string big(1100, 'a');
    cout << deserializeUserData("name="+big+";age=20;email=aa@bb.cc;roles=user") << "\n";
    return 0;
}