#include <bits/stdc++.h>
using namespace std;

class Lse144 {
public:
    static int checkAndUnsubscribe(const std::string& email) {
        static const unordered_set<string> DATABASE = {
            "alice@example.com",
            "bob@fail.com",
            "carol@example.com",
            "dave@example.com",
            "eve@example.org"
        };
        if (DATABASE.find(email) == DATABASE.end()) return 0;
        int rc = unsubscribeScript(email);
        return (rc == 0) ? 1 : 0;
    }
private:
    static int unsubscribeScript(const std::string& email) {
        auto pos = email.find_last_of('@');
        string domain = (pos == string::npos) ? "" : email.substr(pos + 1);
        if (!domain.empty()) {
            string low = domain;
            for (auto& c : low) c = (char)tolower((unsigned char)c);
            if (low == "fail.com") return 2; // simulate failure
        }
        return 0; // success
    }
};

int main() {
    vector<string> tests = {
        "alice@example.com",    // exists, script success -> 1
        "bob@fail.com",         // exists, script failure -> 0
        "notfound@example.com", // not in DB -> 0
        "carol@example.com",    // exists, script success -> 1
        "unknown@fail.com"      // not in DB -> 0
    };
    for (const auto& email : tests) {
        int result = Lse144::checkAndUnsubscribe(email);
        cout << email << " -> " << result << "\n";
    }
    return 0;
}