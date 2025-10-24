#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <limits>

namespace safeio {
    // Reads a single line from 'in' up to maxLen characters. If the input exceeds maxLen,
    // the remainder of the line is discarded and 'truncated' is set to true.
    std::string readLineLimited(std::istream& in, std::size_t maxLen, bool& truncated) {
        truncated = false;
        std::string out;
        out.reserve(maxLen);
        char ch = '\0';
        std::size_t count = 0;

        while (in.get(ch)) {
            if (ch == '\n') {
                break;
            }
            if (count < maxLen) {
                out.push_back(ch);
                ++count;
            } else {
                // Keep consuming the rest of the line to avoid affecting subsequent reads
                truncated = true;
            }
        }
        return out;
    }
}

// Process input by:
// - Removing non-printable characters (except collapsing whitespace to single spaces)
// - Trimming leading/trailing whitespace
// - Collapsing internal whitespace to a single space
// - Converting to uppercase
std::string processInput(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    bool inSpace = true; // treat start as if after a space to skip leading spaces

    for (unsigned char uch : input) {
        // Normalize CR/LF to space
        if (uch == '\r' || uch == '\n') {
            uch = ' ';
        }

        // Handle whitespace
        if (std::isspace(uch)) {
            inSpace = true;
            continue;
        }

        // Skip non-printable
        if (!std::isprint(uch)) {
            continue;
        }

        if (inSpace && !out.empty()) {
            out.push_back(' ');
        }
        inSpace = false;

        // Uppercase (locale-independent, safe cast)
        char upper = static_cast<char>(std::toupper(uch));
        out.push_back(upper);
    }

    // No trailing spaces added (by design), so no need to trim here
    return out;
}

int main() {
    constexpr std::size_t MAX_INPUT = 4096;

    // Self-tests (5 cases)
    const std::vector<std::string> tests = {
        "",                                 // empty
        "   hello   world   ",              // leading/trailing + multiple spaces
        "a\tb\tc",                          // tabs -> single spaces
        "Hi\x01 there!\nNew line",          // non-printable and newline
        std::string(5000, 'x')              // very long input
    };

    std::cout << "Self-tests:\n";
    for (std::size_t i = 0; i < tests.size(); ++i) {
        std::string processed = processInput(tests[i]);
        std::cout << "Test " << (i + 1) << ": [" << processed << "]\n";
    }

    // User input
    std::cout << "\nEnter a single line to process (max " << MAX_INPUT << " characters):\n";
    bool truncated = false;
    std::string user = safeio::readLineLimited(std::cin, MAX_INPUT, truncated);

    if (truncated) {
        std::cout << "Warning: input exceeded " << MAX_INPUT << " characters and was truncated.\n";
    }

    std::string result = processInput(user);
    std::cout << "Processed: [" << result << "]\n";

    return 0;
}