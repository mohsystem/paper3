#include <bits/stdc++.h>
using namespace std;

static const string SALT = "Task102_SALT";

string fnv1a64_hex(const string& s) {
    const uint64_t offset = 14695981039346656037ull;
    const uint64_t prime = 1099511628211ull;
    uint64_t h = offset;
    for (unsigned char c : s) {
        h ^= c;
        h *= prime;
    }
    ostringstream oss;
    oss << std::hex << std::nouppercase << setfill('0') << setw(16) << h;
    return oss.str();
}

string hash_password(const string& password) {
    return fnv1a64_hex(SALT + password);
}

bool update_password(unordered_map<string, string>& db, const string& username, const string& new_password) {
    auto it = db.find(username);
    if (it == db.end() || new_password.empty()) return false;
    it->second = hash_password(new_password);
    return true;
}

string get_password_hash(const unordered_map<string, string>& db, const string& username) {
    auto it = db.find(username);
    return it == db.end() ? string() : it->second;
}

unordered_map<string, string> init_db() {
    unordered_map<string, string> db;
    db["alice"] = hash_password("old1");
    db["bob"] = hash_password("old2");
    db["charlie"] = hash_password("old3");
    return db;
}

int main() {
    auto db = init_db();
    cout << "Initial DB: ";
    for (auto& kv : db) cout << kv.first << "=" << kv.second << " ";
    cout << "\n";

    bool r1 = update_password(db, "alice", "newPass1");
    cout << "Test1 update alice: " << (r1 ? "true" : "false") << " hash=" << get_password_hash(db, "alice") << "\n";

    bool r2 = update_password(db, "bob", "P@ssw0rd!");
    cout << "Test2 update bob: " << (r2 ? "true" : "false") << " hash=" << get_password_hash(db, "bob") << "\n";

    bool r3 = update_password(db, "david", "something");
    cout << "Test3 update david: " << (r3 ? "true" : "false") << "\n";

    bool r4 = update_password(db, "charlie", "");
    cout << "Test4 update charlie with empty: " << (r4 ? "true" : "false") << " hash=" << get_password_hash(db, "charlie") << "\n";

    bool r5 = update_password(db, "alice", "another123");
    cout << "Test5 update alice again: " << (r5 ? "true" : "false") << " hash=" << get_password_hash(db, "alice") << "\n";

    return 0;
}