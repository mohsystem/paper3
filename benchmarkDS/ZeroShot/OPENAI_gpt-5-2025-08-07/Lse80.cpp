#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstddef>

int scan(std::istream& in, char* buf, std::size_t bufsize) {
    if (buf == nullptr || bufsize == 0) {
        return 0;
    }
    std::size_t max = (bufsize > 0) ? (bufsize - 1) : 0;
    if (max > 1023) {
        max = 1023; // enforce 1023 character limit
    }

    std::size_t count = 0;
    int c;
    while (true) {
        c = in.get();
        if (!in) {
            break; // EOF or error
        }
        if (c == '\n') {
            break; // consume newline
        }
        if (count < max) {
            buf[count++] = static_cast<char>(c);
        } else {
            // buffer full; discard until newline or EOF
            while (true) {
                c = in.get();
                if (!in || c == '\n') break;
            }
            break;
        }
    }
    buf[(count <= max) ? count : max] = '\0';
    return static_cast<int>(count);
}

int main() {
    auto runTest = [](const std::string& label, const std::string& input) {
        char buf[1024];
        std::istringstream iss(input);
        int n = scan(iss, buf, sizeof(buf));
        std::cout << label << " -> count=" << n << ", out=\"" << std::string(buf, buf + n) << "\"\n";
    };

    runTest("Test1", std::string("Hello, World!\nTrailing"));
    runTest("Test2", std::string(""));
    runTest("Test3", std::string("NoNewline"));
    runTest("Test4", std::string(1500, 'A') + "\nBBBB");
    runTest("Test5", std::string("Line1\nLine2\n"));

    return 0;
}