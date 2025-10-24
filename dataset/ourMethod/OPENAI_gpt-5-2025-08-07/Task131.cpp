#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <iomanip>

struct UserRecord {
    uint32_t userId;
    std::string username;
    uint8_t age;
    std::string email;
};

static const uint8_t MAGIC[4] = {'U','S','R','1'};
static const uint8_t VERSION = 1;
static const size_t USERNAME_MIN = 1;
static const size_t USERNAME_MAX = 64;
static const size_t EMAIL_MIN = 3;
static const size_t EMAIL_MAX = 254;
static const uint8_t AGE_MIN = 0;
static const uint8_t AGE_MAX = 120;

static bool isAsciiPrintable(const std::string& s) {
    for (unsigned char c : s) {
        if (c < 0x20 || c > 0x7E) return false;
    }
    return true;
}

static bool usernameValid(const std::string& s) {
    if (s.size() < USERNAME_MIN || s.size() > USERNAME_MAX) return false;
    for (char ch : s) {
        if (!((ch >= 'A' && ch <= 'Z') ||
              (ch >= 'a' && ch <= 'z') ||
              (ch >= '0' && ch <= '9') ||
              ch == '_' || ch == '.' || ch == '-')) return false;
    }
    return true;
}

static bool emailValid(const std::string& s) {
    if (s.size() < EMAIL_MIN || s.size() > EMAIL_MAX) return false;
    // Simple validation: allowed chars, one '@', dot after '@'
    size_t atPos = std::string::npos;
    for (size_t i = 0; i < s.size(); ++i) {
        char ch = s[i];
        bool allowed = ((ch >= 'A' && ch <= 'Z') ||
                        (ch >= 'a' && ch <= 'z') ||
                        (ch >= '0' && ch <= '9') ||
                        ch == '.' || ch == '_' || ch == '%' || ch == '+' || ch == '-' || ch == '@');
        if (!allowed) return false;
        if (ch == '@') {
            if (atPos != std::string::npos) return false; // multiple @
            atPos = i;
        }
    }
    if (atPos == std::string::npos || atPos == 0 || atPos == s.size() - 1) return false;
    size_t dotAfter = s.find('.', atPos + 1);
    if (dotAfter == std::string::npos || dotAfter == atPos + 1 || dotAfter == s.size() - 1) return false;
    return true;
}

static void ensure(bool cond, const char* msg) {
    if (!cond) throw std::invalid_argument(msg);
}

std::vector<uint8_t> serialize(const UserRecord& r) {
    ensure(r.username.size() >= USERNAME_MIN && r.username.size() <= USERNAME_MAX, "username length invalid");
    ensure(usernameValid(r.username), "username content invalid");
    ensure(isAsciiPrintable(r.username), "username not ASCII");
    ensure(isAsciiPrintable(r.email), "email not ASCII");
    ensure(emailValid(r.email), "email content invalid");
    if (r.age < AGE_MIN || r.age > AGE_MAX) throw std::invalid_argument("age out of range");
    std::vector<uint8_t> out;
    out.reserve(4 + 1 + 4 + 1 + r.username.size() + 1 + 1 + r.email.size());
    out.insert(out.end(), MAGIC, MAGIC + 4);
    out.push_back(VERSION);
    out.push_back((uint8_t)((r.userId >> 24) & 0xFF));
    out.push_back((uint8_t)((r.userId >> 16) & 0xFF));
    out.push_back((uint8_t)((r.userId >> 8) & 0xFF));
    out.push_back((uint8_t)(r.userId & 0xFF));
    out.push_back((uint8_t)(r.username.size() & 0xFF));
    out.insert(out.end(), r.username.begin(), r.username.end());
    out.push_back(r.age);
    out.push_back((uint8_t)(r.email.size() & 0xFF));
    out.insert(out.end(), r.email.begin(), r.email.end());
    return out;
}

bool deserialize(const std::vector<uint8_t>& data, UserRecord& out, std::string& error) {
    try {
        if (data.size() < 12) throw std::invalid_argument("Data too short");
        if (!(data[0] == MAGIC[0] && data[1] == MAGIC[1] && data[2] == MAGIC[2] && data[3] == MAGIC[3])) {
            throw std::invalid_argument("Invalid magic");
        }
        size_t pos = 4;
        uint8_t version = data[pos++];
        if (version != VERSION) throw std::invalid_argument("Unsupported version");
        if (pos + 4 > data.size()) throw std::invalid_argument("Insufficient data for userId");
        uint32_t userId = (uint32_t)data[pos] << 24 |
                          (uint32_t)data[pos+1] << 16 |
                          (uint32_t)data[pos+2] << 8  |
                          (uint32_t)data[pos+3];
        pos += 4;
        if (pos >= data.size()) throw std::invalid_argument("Missing username length");
        size_t ulen = data[pos++];
        if (ulen < USERNAME_MIN || ulen > USERNAME_MAX) throw std::invalid_argument("Invalid username length");
        if (pos + ulen > data.size()) throw std::invalid_argument("Insufficient data for username");
        std::string username((const char*)(&data[pos]), (const char*)(&data[pos + ulen]));
        pos += ulen;
        if (!isAsciiPrintable(username)) throw std::invalid_argument("Username not ASCII");
        if (!usernameValid(username)) throw std::invalid_argument("Invalid username content");
        if (pos >= data.size()) throw std::invalid_argument("Missing age");
        uint8_t age = data[pos++];
        if (age < AGE_MIN || age > AGE_MAX) throw std::invalid_argument("Invalid age");
        if (pos >= data.size()) throw std::invalid_argument("Missing email length");
        size_t elen = data[pos++];
        if (elen < EMAIL_MIN || elen > EMAIL_MAX) throw std::invalid_argument("Invalid email length");
        if (pos + elen > data.size()) throw std::invalid_argument("Insufficient data for email");
        std::string email((const char*)(&data[pos]), (const char*)(&data[pos + elen]));
        pos += elen;
        if (!isAsciiPrintable(email)) throw std::invalid_argument("Email not ASCII");
        if (!emailValid(email)) throw std::invalid_argument("Invalid email content");
        if (pos != data.size()) throw std::invalid_argument("Trailing data present");
        out = UserRecord{userId, username, age, email};
        return true;
    } catch (const std::exception& ex) {
        error = ex.what();
        return false;
    }
}

static void printRecord(const UserRecord& r) {
    std::cout << "UserRecord{id=" << r.userId << ", username='" << r.username
              << "', age=" << (int)r.age << ", email='" << r.email << "'}\n";
}

int main() {
    // 1) Valid
    UserRecord r1{123456u, "alice_01", 30u, "alice@example.com"};
    std::vector<uint8_t> d1 = serialize(r1);
    UserRecord out{};
    std::string err;
    if (deserialize(d1, out, err)) {
        std::cout << "T1: "; printRecord(out);
    } else {
        std::cout << "T1 fail: " << err << "\n";
    }

    // 2) Invalid magic
    std::vector<uint8_t> badMagic = d1;
    badMagic[0] = 'B';
    if (deserialize(badMagic, out, err)) {
        std::cout << "T2 unexpected success\n";
    } else {
        std::cout << "T2: " << err << "\n";
    }

    // 3) Invalid age (200)
    std::vector<uint8_t> badAge = d1;
    size_t ulen = d1[4 + 1 + 4];
    size_t agePos = 4 + 1 + 4 + 1 + ulen;
    if (agePos < badAge.size()) badAge[agePos] = 200;
    if (deserialize(badAge, out, err)) {
        std::cout << "T3 unexpected success\n";
    } else {
        std::cout << "T3: " << err << "\n";
    }

    // 4) Invalid username (space) - serializer should fail
    try {
        UserRecord r4{42u, "bad name", 25u, "bn@example.org"};
        auto d4 = serialize(r4);
        std::cout << "T4 unexpected serialize success, size=" << d4.size() << "\n";
    } catch (const std::exception& ex) {
        std::cout << "T4: " << ex.what() << "\n";
    }

    // 5) Invalid email (missing @) - serializer should fail
    try {
        UserRecord r5{77u, "bob-77", 44u, "bob#example.com"};
        auto d5 = serialize(r5);
        std::cout << "T5 unexpected serialize success, size=" << d5.size() << "\n";
    } catch (const std::exception& ex) {
        std::cout << "T5: " << ex.what() << "\n";
    }
    return 0;
}