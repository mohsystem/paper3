// Chain-of-Through process in code generation:
// 1) Problem understanding: Read a string, process by trimming leading/trailing whitespace, collapsing internal whitespace to single spaces, and converting to uppercase.
// 2) Security requirements: Use std::string and std::getline to avoid buffer overflow; optionally cap processed size.
// 3) Secure coding generation: Carefully handle whitespace using std::isspace with unsigned char cast; avoid raw buffers.
// 4) Code review: No unsafe C-style reads; no unchecked memory writes; simple, bounded processing.
// 5) Secure code output: Final code produced with safe standard library usage.

#include <iostream>
#include <string>
#include <vector>
#include <cctype>

std::string process_input(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    bool last_was_space = true; // suppress leading spaces

    for (char ch : input) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (std::isspace(c)) {
            if (!out.empty() && !last_was_space) {
                out.push_back(' ');
                last_was_space = true;
            }
        } else {
            out.push_back(static_cast<char>(std::toupper(c)));
            last_was_space = false;
        }
    }
    if (!out.empty() && out.back() == ' ') {
        out.pop_back();
    }
    return out;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string user;
    std::getline(std::cin, user);

    // Optional cap to mitigate excessively large input (defense-in-depth, not overflow related)
    const std::size_t MAX_LEN = 1u << 20; // 1 MiB
    if (user.size() > MAX_LEN) {
        user.resize(MAX_LEN);
    }

    std::cout << process_input(user) << "\n";

    // 5 test cases
    std::vector<std::string> tests = {
        "  Hello,   world!  ",
        "",
        "Tabs\tand\nnewlines",
        "1234 abc DEF",
        "   multiple    spaces   here   "
    };
    for (const auto& t : tests) {
        std::cout << process_input(t) << "\n";
    }

    return 0;
}