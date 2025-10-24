
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>

using namespace std;

vector<string> split(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool isValidIPv4(const string& ip) {
    if (ip.empty()) {
        return false;
    }
    
    vector<string> parts = split(ip, '.');
    if (parts.size() != 4) {
        return false;
    }
    
    for (const string& part : parts) {
        if (part.empty() || part.length() > 3) {
            return false;
        }
        
        // Check for leading zeros
        if (part.length() > 1 && part[0] == '0') {
            return false;
        }
        
        // Check if all characters are digits
        for (char c : part) {
            if (!isdigit(c)) {
                return false;
            }
        }
        
        try {
            int num = stoi(part);
            if (num < 0 || num > 255) {
                return false;
            }
        } catch (...) {
            return false;
        }
    }
    return true;
}

bool isValidIPv6(const string& ip) {
    if (ip.empty()) {
        return false;
    }
    
    vector<string> parts = split(ip, ':');
    if (parts.size() > 8 || parts.size() < 3) {
        return false;
    }
    
    bool hasDoubleColon = ip.find("::") != string::npos;
    if (hasDoubleColon) {
        if (ip.find("::") != ip.rfind("::")) {
            return false;
        }
    }
    
    for (const string& part : parts) {
        if (part.empty() && hasDoubleColon) {
            continue;
        }
        if (part.length() > 4) {
            return false;
        }
        for (char c : part) {
            if (!isxdigit(c)) {
                return false;
            }
        }
    }
    return true;
}

string getIPVersion(const string& ip) {
    if (isValidIPv4(ip)) {
        return "IPv4";
    } else if (isValidIPv6(ip)) {
        return "IPv6";
    } else {
        return "Invalid";
    }
}

string normalizeIPv4(const string& ip) {
    if (!isValidIPv4(ip)) {
        return "Invalid IP";
    }
    return ip;
}

int main() {
    // Test case 1: Valid IPv4
    string test1 = "192.168.1.1";
    cout << "Test 1: " << test1 << endl;
    cout << "Valid IPv4: " << (isValidIPv4(test1) ? "true" : "false") << endl;
    cout << "IP Version: " << getIPVersion(test1) << endl;
    cout << endl;
    
    // Test case 2: Invalid IPv4 (out of range)
    string test2 = "256.168.1.1";
    cout << "Test 2: " << test2 << endl;
    cout << "Valid IPv4: " << (isValidIPv4(test2) ? "true" : "false") << endl;
    cout << "IP Version: " << getIPVersion(test2) << endl;
    cout << endl;
    
    // Test case 3: Valid IPv6
    string test3 = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
    cout << "Test 3: " << test3 << endl;
    cout << "Valid IPv6: " << (isValidIPv6(test3) ? "true" : "false") << endl;
    cout << "IP Version: " << getIPVersion(test3) << endl;
    cout << endl;
    
    // Test case 4: IPv6 with double colon
    string test4 = "2001:db8::8a2e:370:7334";
    cout << "Test 4: " << test4 << endl;
    cout << "Valid IPv6: " << (isValidIPv6(test4) ? "true" : "false") << endl;
    cout << "IP Version: " << getIPVersion(test4) << endl;
    cout << endl;
    
    // Test case 5: Invalid IP
    string test5 = "invalid.ip.address";
    cout << "Test 5: " << test5 << endl;
    cout << "Valid IPv4: " << (isValidIPv4(test5) ? "true" : "false") << endl;
    cout << "Valid IPv6: " << (isValidIPv6(test5) ? "true" : "false") << endl;
    cout << "IP Version: " << getIPVersion(test5) << endl;
    cout << endl;
    
    return 0;
}
