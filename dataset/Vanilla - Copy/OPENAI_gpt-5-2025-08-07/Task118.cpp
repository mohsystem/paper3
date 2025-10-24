#include <bits/stdc++.h>
using namespace std;

class Task118 {
public:
    static bool isValidIPv4(const string &ip) {
        if (ip.empty()) return false;
        string s = ip;
        // trim
        auto l = s.find_first_not_of(" \t\r\n");
        auto r = s.find_last_not_of(" \t\r\n");
        if (l == string::npos) return false;
        s = s.substr(l, r - l + 1);
        if (s.front() == '.' || s.back() == '.') return false;
        vector<string> parts;
        splitByChar(s, '.', parts);
        if (parts.size() != 4) return false;
        for (auto &p : parts) {
            if (p.size() == 0 || p.size() > 3) return false;
            for (char c : p) if (!isdigit((unsigned char)c)) return false;
            if (p.size() > 1 && p[0] == '0') return false;
            int val = 0;
            try {
                val = stoi(p);
            } catch (...) { return false; }
            if (val < 0 || val > 255) return false;
        }
        return true;
    }

    static bool isValidIPv6(const string &ip) {
        if (ip.empty()) return false;
        string s = trim(ip);
        if (s.empty()) return false;
        if (s.find('.') != string::npos) return false; // no embedded IPv4 support
        size_t dc = s.find("::");
        if (dc != string::npos) {
            if (s.find("::", dc + 2) != string::npos) return false;
            string left = s.substr(0, dc);
            string right = s.substr(dc + 2);
            vector<string> leftParts, rightParts;
            if (!left.empty()) {
                splitByChar(left, ':', leftParts);
                if (any_of(leftParts.begin(), leftParts.end(), [](const string& p){ return p.empty() || !isHexGroup(p); })) return false;
            }
            if (!right.empty()) {
                splitByChar(right, ':', rightParts);
                if (any_of(rightParts.begin(), rightParts.end(), [](const string& p){ return p.empty() || !isHexGroup(p); })) return false;
            }
            int groups = (int)leftParts.size() + (int)rightParts.size();
            if (groups > 7) return false;
            return true;
        } else {
            vector<string> parts;
            splitByChar(s, ':', parts);
            if (parts.size() != 8) return false;
            for (auto &p : parts) if (!isHexGroup(p)) return false;
            return true;
        }
    }

    static string validateIPAddress(const string &ip) {
        if (isValidIPv4(ip)) return "IPv4";
        if (isValidIPv6(ip)) return "IPv6";
        return "Neither";
    }

    static string normalizeIPv4(const string &ip) {
        if (!isValidIPv4(ip)) return string();
        string s = trim(ip);
        vector<string> parts;
        splitByChar(s, '.', parts);
        ostringstream oss;
        for (int i = 0; i < 4; ++i) {
            if (i) oss << '.';
            oss << stoi(parts[i]);
        }
        return oss.str();
    }

    static string normalizeIPv6(const string &ip) {
        if (!isValidIPv6(ip)) return string();
        string s = trim(ip);
        vector<string> groups;
        size_t dc = s.find("::");
        if (dc != string::npos) {
            string left = s.substr(0, dc);
            string right = s.substr(dc + 2);
            vector<string> leftParts, rightParts;
            if (!left.empty()) splitByChar(left, ':', leftParts);
            if (!right.empty()) splitByChar(right, ':', rightParts);
            for (auto &p : leftParts) groups.push_back(p);
            int zerosToInsert = 8 - ((int)leftParts.size() + (int)rightParts.size());
            for (int i = 0; i < zerosToInsert; ++i) groups.push_back("0");
            for (auto &p : rightParts) groups.push_back(p);
        } else {
            splitByChar(s, ':', groups);
        }
        if (groups.size() != 8) return string();
        for (auto &g : groups) g = pad4Upper(g);
        return join(groups, ':');
    }

private:
    static string trim(const string &s) {
        size_t l = s.find_first_not_of(" \t\r\n");
        if (l == string::npos) return "";
        size_t r = s.find_last_not_of(" \t\r\n");
        return s.substr(l, r - l + 1);
        }

    static void splitByChar(const string &s, char delim, vector<string> &out) {
        out.clear();
        string cur;
        for (char c : s) {
            if (c == delim) {
                out.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        out.push_back(cur);
    }

    static bool isHexGroup(const string &s) {
        if (s.size() < 1 || s.size() > 4) return false;
        for (char c : s) {
            if (!(isdigit((unsigned char)c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
                return false;
        }
        return true;
    }

    static string pad4Upper(const string &h) {
        string u = h;
        for (auto &c : u) c = (char)toupper((unsigned char)c);
        if (u.size() < 4) {
            return string(4 - u.size(), '0') + u;
        }
        return u;
    }

    static string join(const vector<string> &v, char sep) {
        ostringstream oss;
        for (size_t i = 0; i < v.size(); ++i) {
            if (i) oss << sep;
            oss << v[i];
        }
        return oss.str();
    }
};

int main() {
    vector<string> tests = {
        "172.16.254.1",
        "2001:0db8:85a3:0000:0000:8A2E:0370:7334",
        "2001:db8:85a3::8a2e:370:7334",
        "256.256.256.256",
        "01.1.1.1"
    };
    for (auto &ip : tests) {
        string kind = Task118::validateIPAddress(ip);
        cout << "IP: " << ip << " -> " << kind << "\n";
        if (kind == "IPv4") {
            cout << "Normalized IPv4: " << Task118::normalizeIPv4(ip) << "\n";
        } else if (kind == "IPv6") {
            cout << "Normalized IPv6: " << Task118::normalizeIPv6(ip) << "\n";
        }
        cout << "\n";
    }
    return 0;
}