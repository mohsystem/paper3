#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <algorithm>
#include <cctype>

class KeyManager {
public:
    bool generateKey(const std::string& alias, size_t lengthBytes) {
        if (alias.empty() || lengthBytes == 0 || store.count(alias)) return false;
        std::vector<uint8_t> key(lengthBytes);
        fillRandom(key.data(), key.size());
        store[alias] = std::move(key);
        return true;
    }

    std::string exportKeyBase64(const std::string& alias) const {
        auto it = store.find(alias);
        if (it == store.end()) return std::string();
        return base64Encode(it->second.data(), it->second.size());
    }

    bool importKeyBase64(const std::string& alias, const std::string& b64, bool overwrite) {
        if (alias.empty()) return false;
        if (!overwrite && store.count(alias)) return false;
        std::vector<uint8_t> data;
        if (!base64Decode(b64, data) || data.empty()) return false;
        store[alias] = std::move(data);
        return true;
    }

    std::string rotateKey(const std::string& alias, size_t newLengthBytes) {
        auto it = store.find(alias);
        if (it == store.end() || newLengthBytes == 0) return std::string();
        std::vector<uint8_t> key(newLengthBytes);
        fillRandom(key.data(), key.size());
        it->second = std::move(key);
        return base64Encode(it->second.data(), it->second.size());
    }

    bool deleteKey(const std::string& alias) {
        return store.erase(alias) > 0;
    }

    std::string listAliases() const {
        std::ostringstream oss;
        bool first = true;
        for (const auto& kv : store) {
            if (!first) oss << ",";
            first = false;
            oss << kv.first;
        }
        return oss.str();
    }

    int getKeyLength(const std::string& alias) const {
        auto it = store.find(alias);
        if (it == store.end()) return -1;
        return static_cast<int>(it->second.size());
    }

private:
    std::unordered_map<std::string, std::vector<uint8_t>> store;

    static void fillRandom(uint8_t* buf, size_t len) {
        std::random_device rd;
        for (size_t i = 0; i < len; ++i) buf[i] = static_cast<uint8_t>(rd());
    }

    static std::string base64Encode(const uint8_t* data, size_t len) {
        static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        out.reserve(((len + 2) / 3) * 4);
        size_t i = 0;
        while (i + 3 <= len) {
            uint32_t n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
            out.push_back(tbl[(n >> 18) & 63]);
            out.push_back(tbl[(n >> 12) & 63]);
            out.push_back(tbl[(n >> 6) & 63]);
            out.push_back(tbl[n & 63]);
            i += 3;
        }
        if (i < len) {
            uint32_t n = data[i] << 16;
            out.push_back(tbl[(n >> 18) & 63]);
            if (i + 1 < len) {
                n |= data[i + 1] << 8;
                out.push_back(tbl[(n >> 12) & 63]);
                out.push_back(tbl[(n >> 6) & 63]);
                out.push_back('=');
            } else {
                out.push_back(tbl[(n >> 12) & 63]);
                out.push_back('=');
                out.push_back('=');
            }
        }
        return out;
    }

    static bool base64Decode(const std::string& in, std::vector<uint8_t>& out) {
        int T[256];
        std::fill(std::begin(T), std::end(T), -1);
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int i = 0; i < 64; ++i) T[static_cast<unsigned char>(chars[i])] = i;
        auto clean = in;
        clean.erase(std::remove_if(clean.begin(), clean.end(), [](unsigned char c) {
            return !(std::isalnum(c) || c == '+' || c == '/' || c == '=');
        }), clean.end());

        if (clean.size() % 4 != 0) return false;
        size_t padding = 0;
        if (!clean.empty()) {
            if (clean[clean.size() - 1] == '=') padding++;
            if (clean[clean.size() - 2] == '=') padding++;
        }
        size_t outLen = (clean.size() / 4) * 3 - padding;
        out.clear();
        out.reserve(outLen);

        for (size_t i = 0; i < clean.size(); i += 4) {
            int a = clean[i] == '=' ? 0 : T[static_cast<unsigned char>(clean[i])];
            int b = clean[i + 1] == '=' ? 0 : T[static_cast<unsigned char>(clean[i + 1])];
            int c = clean[i + 2] == '=' ? 0 : T[static_cast<unsigned char>(clean[i + 2])];
            int d = clean[i + 3] == '=' ? 0 : T[static_cast<unsigned char>(clean[i + 3])];
            if (a < 0 || b < 0 || c < 0 || d < 0) return false;

            uint32_t n = (a << 18) | (b << 12) | (c << 6) | d;
            out.push_back((n >> 16) & 0xFF);
            if (clean[i + 2] != '=') out.push_back((n >> 8) & 0xFF);
            if (clean[i + 3] != '=') out.push_back(n & 0xFF);
        }
        return true;
    }
};

int main() {
    KeyManager km;

    // Test 1
    bool t1gen = km.generateKey("api", 32);
    std::string t1exp = km.exportKeyBase64("api");
    std::cout << "T1 gen=" << t1gen << " len=" << km.getKeyLength("api") << " b64prefix=" << (t1exp.empty() ? std::string("null") : t1exp.substr(0, 10)) << "\n";

    // Test 2
    bool t2gen = km.generateKey("db", 16);
    std::cout << "T2 gen=" << t2gen << " aliases=" << km.listAliases() << "\n";

    // Test 3
    std::string exported = km.exportKeyBase64("api");
    bool t3imp = km.importKeyBase64("imported", exported, false);
    std::cout << "T3 import=" << t3imp << " importedLen=" << km.getKeyLength("imported") << "\n";

    // Test 4
    std::string t4rot = km.rotateKey("api", 24);
    std::cout << "T4 rotatedLen=" << km.getKeyLength("api") << " rotB64prefix=" << (t4rot.empty() ? std::string("null") : t4rot.substr(0, 10)) << "\n";

    // Test 5
    bool t5del = km.deleteKey("db");
    std::cout << "T5 delete=" << t5del << " aliases=" << km.listAliases() << "\n";

    return 0;
}