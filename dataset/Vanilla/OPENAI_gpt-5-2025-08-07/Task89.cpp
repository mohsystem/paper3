#include <iostream>
#include <fstream>
#include <string>

bool comparePassword(const std::string& filePath, const std::string& enteredPassword) {
    std::ifstream in(filePath);
    if (!in.is_open()) return false;

    std::string stored;
    if (!std::getline(in, stored)) {
        stored = "";
    }

    if (stored.size() != enteredPassword.size()) return false;
    for (size_t i = 0; i < stored.size(); ++i) {
        if (stored[i] != enteredPassword[i]) return false;
    }
    return true;
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary);
    out << content;
}

int main() {
    // Prepare test files
    std::string f1 = "test_pw_1.txt"; writeFile(f1, "hunter2");
    std::string f2 = "test_pw_2.txt"; writeFile(f2, "hunter2");
    std::string f3 = "test_pw_3.txt"; writeFile(f3, "");
    std::string f4 = "test_pw_4.txt"; writeFile(f4, "pass");
    std::string f5 = "test_pw_5.txt"; writeFile(f5, "password");

    // 5 test cases
    std::cout << (comparePassword(f1, "hunter2") ? "true" : "false") << "\n";  // true
    std::cout << (comparePassword(f2, "hunter3") ? "true" : "false") << "\n";  // false
    std::cout << (comparePassword(f3, "") ? "true" : "false") << "\n";         // true
    std::cout << (comparePassword(f4, "password") ? "true" : "false") << "\n"; // false
    std::cout << (comparePassword(f5, "pass") ? "true" : "false") << "\n";     // false

    return 0;
}