#include <iostream>
#include <vector>
#include <string>
#include <cctype>

int str_to_int(const std::string& s) {
    size_t i = 0, n = s.size();
    while (i < n && std::isspace(static_cast<unsigned char>(s[i]))) i++;
    int sign = 1;
    if (i < n && (s[i] == '+' || s[i] == '-')) {
        if (s[i] == '-') sign = -1;
        i++;
    }
    long long val = 0;
    while (i < n && std::isdigit(static_cast<unsigned char>(s[i]))) {
        val = val * 10 + (s[i] - '0');
        i++;
    }
    return static_cast<int>(sign * val);
}

void printAtIndex(const std::vector<std::string>& args, int index) {
    if (index >= 0 && index < static_cast<int>(args.size())) {
        std::cout << args[index] << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::vector<std::string> args(argv, argv + argc);
        int idx = str_to_int(args[1]);
        printAtIndex(args, idx);
        return 0;
    }

    // 5 test cases (run when no CLI index provided)
    std::vector<std::string> t1 = {"Lse7", "2", "zero", "one", "two"};
    printAtIndex(t1, str_to_int(t1[1]));

    std::vector<std::string> t2 = {"Lse7", "3", "red", "green", "blue", "yellow"};
    printAtIndex(t2, str_to_int(t2[1]));

    std::vector<std::string> t3 = {"Lse7", "1", "hello"};
    printAtIndex(t3, str_to_int(t3[1]));

    std::vector<std::string> t4 = {"Lse7", "4", "a", "b", "c", "d", "e"};
    printAtIndex(t4, str_to_int(t4[1]));

    std::vector<std::string> t5 = {"Lse7", "0", "first"};
    printAtIndex(t5, str_to_int(t5[1]));

    return 0;
}