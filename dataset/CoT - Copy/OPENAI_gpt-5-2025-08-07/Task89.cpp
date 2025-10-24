#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

bool constantTimeEquals(const std::string& a, const std::string& b) {
    size_t alen = a.size();
    size_t blen = b.size();
    size_t maxlen = (alen > blen) ? alen : blen;
    unsigned int diff = static_cast<unsigned int>(alen ^ blen);
    for (size_t i = 0; i < maxlen; ++i) {
        unsigned char ac = (i < alen) ? static_cast<unsigned char>(a[i]) : 0;
        unsigned char bc = (i < blen) ? static_cast<unsigned char>(b[i]) : 0;
        diff |= static_cast<unsigned int>(ac ^ bc);
    }
    return diff == 0;
}

bool comparePasswordFromFile(const std::string& filePath, const std::string& userInput) {
    std::ifstream in(filePath, std::ios::in);
    if (!in.is_open()) {
        return false;
    }
    std::string stored;
    if (!std::getline(in, stored)) {
        stored.clear();
    }
    in.close();
    bool result = constantTimeEquals(stored, userInput);
    // zeroize stored
    std::fill(stored.begin(), stored.end(), '\0');
    return result;
}

static void writeToFile(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    out << content;
    out.close();
}

int main() {
    // Test case 1: exact match
    std::string f1 = "cpp_pw_test1.txt";
    writeToFile(f1, "Secret123!");
    std::cout << (comparePasswordFromFile(f1, "Secret123!") ? "1" : "0") << std::endl;

    // Test case 2: mismatch
    std::string f2 = "cpp_pw_test2.txt";
    writeToFile(f2, "Secret123!");
    std::cout << (comparePasswordFromFile(f2, "secret123!") ? "1" : "0") << std::endl;

    // Test case 3: different length
    std::string f3 = "cpp_pw_test3.txt";
    writeToFile(f3, "Short");
    std::cout << (comparePasswordFromFile(f3, "Shorter") ? "1" : "0") << std::endl;

    // Test case 4: trailing newline in file
    std::string f4 = "cpp_pw_test4.txt";
    writeToFile(f4, "WithNewline\n");
    std::cout << (comparePasswordFromFile(f4, "WithNewline") ? "1" : "0") << std::endl;

    // Test case 5: file not found
    std::cout << (comparePasswordFromFile("cpp_pw_nonexistent_abcdefg.txt", "anything") ? "1" : "0") << std::endl;

    // Cleanup
    std::remove(f1.c_str());
    std::remove(f2.c_str());
    std::remove(f3.c_str());
    std::remove(f4.c_str());
    return 0;
}