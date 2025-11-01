#include <bits/stdc++.h>
using namespace std;

class Lse144 {
public:
    // Returns 1 if email exists and unsubscribe script succeeds (returns 0), else 0
    static int unsubscribeIfExists(const string& email) {
        if (!initialized) initDB();
        if (email.empty()) return 0;
        if (database.find(email) == database.end()) return 0;
        int scriptResult = callUnsubscribeScript(email);
        return scriptResult == 0 ? 1 : 0;
    }

private:
    static inline bool initialized = false;
    static inline unordered_set<string> database;

    static void initDB() {
        database.insert("alice@example.com");
        database.insert("bob_fail@example.com");
        database.insert("charlie@example.com");
        initialized = true;
    }

    // Simulated external script: returns 0 on success, non-zero on failure
    static int callUnsubscribeScript(const string& email) {
        if (email.empty()) return 1;              // failure
        if (email.find("fail") != string::npos) return 2; // simulate failure
        return 0; // success
    }
};

int main() {
    vector<string> tests = {
        "alice@example.com",     // 1
        "missing@example.com",   // 0
        "bob_fail@example.com",  // 0
        "",                      // 0
        "charlie@example.com"    // 1
    };
    for (const auto& t : tests) {
        cout << Lse144::unsubscribeIfExists(t) << "\n";
    }
    return 0;
}