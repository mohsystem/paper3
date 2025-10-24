#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>

static const size_t MAX_INPUT_LENGTH = 1000;

bool isLengthSafe(const std::string& s) {
    return !s.empty() && s.size() <= MAX_INPUT_LENGTH;
}

bool isAscii(const std::string& s) {
    for (unsigned char c : s) {
        if (c > 0x7F) return false;
    }
    return true;
}

bool isValidIPv4(const std::string& s) {
    if (!isLengthSafe(s) || !isAscii(s)) return false;
    for (char c : s) {
        if (!(std::isdigit(static_cast<unsigned char>(c)) || c == '.')) return false;
    }
    std::vector<std::string> parts;
    std::string token;
    std::istringstream iss(s);
    while (std::getline(iss, token, '.')) {
        parts.push_back(token);
    }
    if (parts.size() != 4) return false;
    for (const auto& p : parts) {
        if (p.empty() || p.size() > 3) return false;
        if (!std::all_of(p.begin(), p.end(), [](char c){ return std::isdigit(static_cast<unsigned char>(c)); })) return false;
        if (p.size() > 1 && p[0] == '0') return false;
        int v = 0;
        try {
            v = std::stoi(p);
        } catch (...) {
            return false;
        }
        if (v < 0 || v > 255) return false;
    }
    return true;
}

std::string normalizeIPv4(const std::string& s) {
    if (!isValidIPv4(s)) return std::string();
    std::vector<std::string> parts;
    std::string token;
    std::istringstream iss(s);
    while (std::getline(iss, token, '.')) parts.push_back(token);
    std::ostringstream oss;
    for (size_t i = 0; i < 4; ++i) {
        int v = std::stoi(parts[i]);
        if (i) oss << '.';
        oss << v;
    }
    return oss.str();
}

static bool parseIPv6Side(const std::string& side, std::vector<uint16_t>& out) {
    if (side.empty()) return true;
    std::istringstream iss(side);
    std::string tok;
    while (std::getline(iss, tok, ':')) {
        if (tok.empty() || tok.size() > 4) return false;
        uint32_t val = 0;
        for (char c : tok) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
            val = (val << 4) | static_cast<uint32_t>(std::isdigit(static_cast<unsigned char>(c)) ? c - '0' : (std::tolower(c) - 'a' + 10));
            if (val > 0xFFFFu) return false;
        }
        out.push_back(static_cast<uint16_t>(val));
        if (out.size() > 8) return false;
    }
    return true;
}

static bool parseIPv6ToGroups(const std::string& s, std::vector<uint16_t>& groups) {
    if (!isLengthSafe(s) || !isAscii(s)) return false;
    if (s.empty()) return false;
    for (char c : s) {
        if (!(c == ':' || std::isxdigit(static_cast<unsigned char>(c)))) return false;
    }
    size_t first = s.find("::");
    if (first != std::string::npos) {
        if (s.find("::", first + 2) != std::string::npos) return false;
        std::string left = s.substr(0, first);
        std::string right = s.substr(first + 2);
        std::vector<uint16_t> lg, rg;
        if (!parseIPv6Side(left, lg)) return false;
        if (!parseIPv6Side(right, rg)) return false;
        if (lg.size() + rg.size() >= 8) return false;
        groups = lg;
        groups.insert(groups.end(), 8 - (lg.size() + rg.size()), 0);
        groups.insert(groups.end(), rg.begin(), rg.end());
    } else {
        std::vector<uint16_t> all;
        if (!parseIPv6Side(s, all)) return false;
        if (all.size() != 8) return false;
        groups = all;
    }
    return groups.size() == 8;
}

bool isValidIPv6(const std::string& s) {
    std::vector<uint16_t> g;
    return parseIPv6ToGroups(s, g);
}

std::string normalizeIPv6(const std::string& s) {
    std::vector<uint16_t> g;
    if (!parseIPv6ToGroups(s, g)) return std::string();
    std::ostringstream oss;
    oss << std::nouppercase << std::hex;
    for (size_t i = 0; i < g.size(); ++i) {
        if (i) oss << ':';
        oss << std::setfill('0') << std::setw(4) << g[i];
    }
    return oss.str();
}

std::string detectIPType(const std::string& s) {
    if (!isLengthSafe(s) || !isAscii(s)) return "Invalid";
    if (isValidIPv4(s)) return "IPv4";
    if (isValidIPv6(s)) return "IPv6";
    return "Invalid";
}

std::string processIP(const std::string& s) {
    std::string t = detectIPType(s);
    if (t == "IPv4") {
        return "Type: IPv4, Normalized: " + normalizeIPv4(s);
    } else if (t == "IPv6") {
        return "Type: IPv6, Normalized: " + normalizeIPv6(s);
    }
    return "Invalid";
}

int main() {
    std::vector<std::string> tests = {
        "192.168.1.1",
        "255.255.255.255",
        "01.2.3.4",
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
        "2001:db8::8a2e:370:7334"
    };
    for (const auto& t : tests) {
        std::cout << t << " => " << processIP(t) << "\n";
    }
    return 0;
}