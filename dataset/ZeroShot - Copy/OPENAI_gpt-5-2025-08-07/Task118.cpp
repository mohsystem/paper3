#include <iostream>
#include <string>
#include <vector>
#include <cctype>

class Task118 {
public:
    static bool isValidIPv4(const std::string& ip) {
        if (ip.empty()) return false;
        int n = static_cast<int>(ip.size());
        int seg = 0;
        int i = 0;
        while (i < n) {
            if (seg == 4) return false;
            if (i >= n || !std::isdigit(static_cast<unsigned char>(ip[i]))) return false;
            if (ip[i] == '0') {
                i++;
                if (i < n && std::isdigit(static_cast<unsigned char>(ip[i]))) return false;
            } else {
                int val = 0;
                int len = 0;
                while (i < n && std::isdigit(static_cast<unsigned char>(ip[i]))) {
                    if (len >= 3) return false;
                    val = val * 10 + (ip[i] - '0');
                    if (val > 255) return false;
                    len++;
                    i++;
                }
                if (len == 0) return false;
            }
            seg++;
            if (seg < 4) {
                if (i >= n || ip[i] != '.') return false;
                i++;
                if (i >= n) return false;
            } else {
                if (i != n) return false;
            }
        }
        return seg == 4;
    }

    static bool isHexGroup(const std::string& s) {
        if (s.empty() || s.size() > 4) return false;
        for (char ch : s) {
            if (!(std::isdigit(static_cast<unsigned char>(ch)) ||
                  (ch >= 'a' && ch <= 'f') ||
                  (ch >= 'A' && ch <= 'F'))) {
                return false;
            }
        }
        return true;
    }

    static bool isValidIPv6(const std::string& ip) {
        if (ip.empty()) return false;
        if (ip.find('.') != std::string::npos) return false; // not supporting embedded IPv4
        size_t pos = ip.find("::");
        if (pos != std::string::npos) {
            if (ip.find("::", pos + 2) != std::string::npos) return false;
            std::string left = ip.substr(0, pos);
            std::string right = ip.substr(pos + 2);
            int leftCount = 0, rightCount = 0;
            if (!left.empty()) {
                size_t start = 0;
                while (true) {
                    size_t colon = left.find(':', start);
                    std::string token = left.substr(start, colon - start);
                    if (token.empty() || !isHexGroup(token)) return false;
                    leftCount++;
                    if (colon == std::string::npos) break;
                    start = colon + 1;
                    if (start >= left.size()) return false; // trailing colon
                }
            }
            if (!right.empty()) {
                size_t start = 0;
                while (true) {
                    size_t colon = right.find(':', start);
                    std::string token = right.substr(start, colon - start);
                    if (token.empty() || !isHexGroup(token)) return false;
                    rightCount++;
                    if (colon == std::string::npos) break;
                    start = colon + 1;
                    if (start >= right.size()) return false; // trailing colon
                }
            }
            int total = leftCount + rightCount;
            return total < 8;
        } else {
            // No compression
            int count = 0;
            size_t start = 0;
            while (true) {
                size_t colon = ip.find(':', start);
                std::string token = ip.substr(start, colon - start);
                if (token.empty() || !isHexGroup(token)) return false;
                count++;
                if (colon == std::string::npos) break;
                start = colon + 1;
                if (start >= ip.size()) return false;
            }
            return count == 8;
        }
    }

    static std::string classifyIP(const std::string& ip) {
        if (isValidIPv4(ip)) return "IPv4";
        if (isValidIPv6(ip)) return "IPv6";
        return "Invalid";
    }
};

int main() {
    std::vector<std::string> tests = {
        "192.168.1.1",
        "255.256.0.1",
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
        "2001:db8::1",
        "01.1.1.1"
    };
    for (const auto& t : tests) {
        std::cout << t << " => " << Task118::classifyIP(t) << "\n";
    }
    return 0;
}