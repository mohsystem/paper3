#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>

std::string validateIPv4(const std::string& ip) {
    if (ip.empty() || ip.back() == '.') {
        return "Neither";
    }
    std::stringstream ss(ip);
    std::string part;
    int count = 0;
    while (std::getline(ss, part, '.')) {
        count++;
        if (part.empty() || (part.length() > 1 && part[0] == '0')) {
            return "Neither";
        }
        if (part.length() > 3) {
            return "Neither";
        }
        for (char c : part) {
            if (!isdigit(c)) {
                return "Neither";
            }
        }
        try {
            int num = std::stoi(part);
            if (num < 0 || num > 255) {
                return "Neither";
            }
        } catch (const std::invalid_argument& ia) {
            return "Neither";
        } catch (const std::out_of_range& oor) {
            return "Neither";
        }
    }
    return (count == 4 && ss.eof()) ? "IPv4" : "Neither";
}

std::string validateIPv6(const std::string& ip) {
    if (ip.empty() || ip.back() == ':') {
        return "Neither";
    }
    std::stringstream ss(ip);
    std::string part;
    int count = 0;
    while (std::getline(ss, part, ':')) {
        count++;
        if (part.empty() || part.length() > 4) {
            return "Neither";
        }
        for (char c : part) {
            if (!isxdigit(c)) {
                return "Neither";
            }
        }
    }
    return (count == 8 && ss.eof()) ? "IPv6" : "Neither";
}

std::string validateIPAddress(const std::string& IP) {
    if (IP.find('.') != std::string::npos) {
        return validateIPv4(IP);
    } else if (IP.find(':') != std::string::npos) {
        return validateIPv6(IP);
    }
    return "Neither";
}

int main() {
    std::vector<std::string> testCases = {
        "172.16.254.1",
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
        "256.256.256.256",
        "192.168.01.1",
        "2001:db8::8a2e"
    };

    for (const auto& ip : testCases) {
        std::cout << "IP: " << ip << " -> " << validateIPAddress(ip) << std::endl;
    }
    return 0;
}