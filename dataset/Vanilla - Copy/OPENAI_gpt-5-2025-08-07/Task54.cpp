#include <bits/stdc++.h>
#include <chrono>
#include <thread>
using namespace std;

struct OTPEntry {
    string code;
    long long expiryMillis;
};

class OTPManager {
    unordered_map<string, OTPEntry> store;
    mt19937 rng;
public:
    OTPManager() {
        random_device rd;
        rng.seed(rd());
    }
    string generateOTP(const string& username, int length = 6, long long ttlMillis = 120000) {
        uniform_int_distribution<int> dist(0, 9);
        string code;
        code.reserve(length);
        for (int i = 0; i < length; ++i) code.push_back(char('0' + dist(rng)));
        long long now = chrono::duration_cast<chrono::milliseconds>(
                            chrono::steady_clock::now().time_since_epoch()).count();
        store[username] = OTPEntry{code, now + ttlMillis};
        return code;
    }
    bool verifyOTP(const string& username, const string& code) {
        auto it = store.find(username);
        if (it == store.end()) return false;
        long long now = chrono::duration_cast<chrono::milliseconds>(
                            chrono::steady_clock::now().time_since_epoch()).count();
        if (now > it->second.expiryMillis) {
            store.erase(it);
            return false;
        }
        bool ok = (it->second.code == code);
        if (ok) store.erase(it); // one-time use
        return ok;
    }
};

class AuthService {
    unordered_map<string, string> users;
    OTPManager& otpManager;
public:
    AuthService(OTPManager& mgr) : otpManager(mgr) {}
    void addUser(const string& u, const string& p) { users[u] = p; }
    string startLogin(const string& username, const string& password, int otpLength = 6, long long ttlMillis = 120000) {
        auto it = users.find(username);
        if (it != users.end() && it->second == password) {
            return otpManager.generateOTP(username, otpLength, ttlMillis);
        }
        return string();
    }
    bool finishLogin(const string& username, const string& otp) {
        if (otp.empty()) return false;
        return otpManager.verifyOTP(username, otp);
    }
};

int main() {
    OTPManager otpMgr;
    AuthService auth(otpMgr);
    auth.addUser("alice", "password123");
    auth.addUser("bob", "qwerty");
    auth.addUser("charlie", "letmein");
    auth.addUser("dave", "passw0rd");
    auth.addUser("eve", "123456");

    // Test 1: Successful login
    string otp1 = auth.startLogin("alice", "password123", 6, 120000);
    bool t1 = auth.finishLogin("alice", otp1);
    cout << "Test1:" << (t1 ? "true" : "false") << "\n";

    // Test 2: Wrong password
    string otp2 = auth.startLogin("bob", "wrong", 6, 120000);
    bool t2 = auth.finishLogin("bob", "000000");
    cout << "Test2:" << (t2 ? "true" : "false") << "\n";

    // Test 3: Correct password, wrong OTP
    string otp3 = auth.startLogin("charlie", "letmein", 6, 120000);
    bool t3 = auth.finishLogin("charlie", "111111");
    cout << "Test3:" << (t3 ? "true" : "false") << "\n";

    // Test 4: Expired OTP
    string otp4 = auth.startLogin("dave", "passw0rd", 6, 1000);
    this_thread::sleep_for(chrono::milliseconds(1500));
    bool t4 = auth.finishLogin("dave", otp4);
    cout << "Test4:" << (t4 ? "true" : "false") << "\n";

    // Test 5: Reuse OTP attempt
    string otp5 = auth.startLogin("eve", "123456", 6, 120000);
    bool t5a = auth.finishLogin("eve", otp5);
    bool t5b = auth.finishLogin("eve", otp5);
    cout << "Test5:" << ((t5a && !t5b) ? "true" : "false") << "\n";

    return 0;
}