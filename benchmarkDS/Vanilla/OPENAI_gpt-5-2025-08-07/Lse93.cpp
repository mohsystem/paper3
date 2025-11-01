#include <bits/stdc++.h>
using namespace std;

struct User {
    string username;
    string hashedPassword;
};

static vector<User> database;

string hash_password(const string& password) {
    // FNV-1a 64-bit
    const uint64_t FNV_OFFSET = 1469598103934665603ULL;
    const uint64_t FNV_PRIME  = 1099511628211ULL;
    uint64_t hash = FNV_OFFSET;
    for (unsigned char c : password) {
        hash ^= c;
        hash *= FNV_PRIME;
    }
    stringstream ss;
    ss << hex << nouppercase << setfill('0') << setw(16) << hash;
    return ss.str();
}

bool insert_user(const string& username, const string& password) {
    string hashed = hash_password(password);
    database.push_back(User{username, hashed});
    return true;
}

int main() {
    insert_user("alice", "password123");
    insert_user("bob", "s3cr3t");
    insert_user("charlie", "letmein");
    insert_user("diana", "P@ssw0rd!");
    insert_user("eve", "qwerty");

    cout << "Current database:\n";
    for (size_t i = 0; i < database.size(); ++i) {
        cout << (i + 1) << ") " << database[i].username << " | " << database[i].hashedPassword << "\n";
    }
    return 0;
}