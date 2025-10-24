#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <sstream>

static bool isSafeAscii(const std::string& s) {
    for (unsigned char ch : s) {
        if (ch > 0x7F) return false;
        if (!(std::isalnum(ch) || ch == ' ' || ch == '_' || ch == '-')) return false;
    }
    return true;
}

std::string processData(int repeatCount, const std::string& text) {
    if (repeatCount < 1 || repeatCount > 20) {
        return "ERROR: repeatCount out of range [1,20]";
    }
    std::string trimmed = text;
    // trim leading
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.front()))) trimmed.erase(trimmed.begin());
    // trim trailing
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back()))) trimmed.pop_back();

    if (trimmed.size() < 1 || trimmed.size() > 100) {
        return "ERROR: text length must be 1..100 after trimming";
    }
    if (!isSafeAscii(trimmed)) {
        return "ERROR: text contains disallowed characters (allowed: A-Z, a-z, 0-9, space, underscore, hyphen)";
    }

    std::string normalized = trimmed;
    for (char& c : normalized) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    int checksum = 0;
    for (unsigned char c : normalized) checksum = (checksum + (c & 0xFF)) % 1000;

    std::ostringstream repeated;
    for (int i = 0; i < repeatCount; ++i) {
        if (i > 0) repeated << '-';
        repeated << normalized;
    }

    std::ostringstream out;
    out << "{\"original\":\"" << trimmed
        << "\", \"normalized\":\"" << normalized
        << "\", \"repeatCount\":" << repeatCount
        << ", \"repeated\":\"" << repeated.str()
        << "\", \"length\":" << normalized.size()
        << ", \"checksum\":" << checksum << "}";
    return out.str();
}

int main(int argc, char* argv[]) {
    // 5 test cases
    std::cout << processData(3, "hello world") << "\n";
    std::cout << processData(1, "Safe_Input-123") << "\n";
    std::cout << processData(5, "   trim  me   ") << "\n";
    std::cout << processData(0, "abc") << "\n";           // invalid repeatCount
    std::cout << processData(2, "bad!char") << "\n";      // invalid character '!'

    // Optional user input via command-line args: first arg is repeatCount, the rest form the text
    if (argc >= 3) {
        try {
            int rc = std::stoi(argv[1]);
            std::string text;
            for (int i = 2; i < argc; ++i) {
                if (i > 2) text.push_back(' ');
                text += argv[i];
            }
            std::cout << processData(rc, text) << "\n";
        } catch (...) {
            std::cout << "ERROR: first argument must be an integer repeatCount\n";
        }
    }
    return 0;
}