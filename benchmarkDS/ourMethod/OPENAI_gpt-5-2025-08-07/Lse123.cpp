#include <iostream>
#include <string>
#include <vector>

static constexpr size_t MAX_LEN = 1u << 20; // 1 MiB

// Returns true on success, false on validation failure.
// Checks exactly one argument, validates string, trims trailing whitespace then newlines, and ensures null termination in a buffer before assigning to out.
bool processArgs(const std::vector<std::string>& args, std::string& out) {
    if (args.size() != 1) {
        return false; // invalid args
    }
    const std::string& s = args[0];
    if (s.size() > MAX_LEN) {
        return false; // too long
    }

    // Work on a mutable copy
    std::string buf = s;

    // First loop: trim trailing whitespace excluding CR/LF (space, tab, vertical tab, form feed)
    size_t end1 = buf.size();
    while (end1 > 0) {
        char c = buf[end1 - 1];
        if (c == ' ' || c == '\t' || c == '\v' || c == '\f') {
            --end1;
        } else {
            break;
        }
    }

    // Second loop: trim trailing newlines (LF, CR)
    size_t end2 = end1;
    while (end2 > 0) {
        char c = buf[end2 - 1];
        if (c == '\n' || c == '\r') {
            --end2;
        } else {
            break;
        }
    }

    // "Null terminate": create an explicit char buffer with '\0'
    std::vector<char> terminated(end2 + 1, '\0');
    if (end2 > 0) {
        std::copy(buf.data(), buf.data() + static_cast<std::ptrdiff_t>(end2), terminated.begin());
    }
    out.assign(terminated.data(), end2);
    return true;
}

int main() {
    std::vector<std::vector<std::string>> tests = {
        { "hello   " },              // trailing spaces
        { "world\t\t\n" },           // tabs then newline
        { "line with spaces \r\n\n" }, // spaces then CRLF+LF
        { "no-trim" },               // no trailing whitespace/newlines
        { "invalid", "extra" }       // invalid: more than one argument
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::string out;
        if (processArgs(tests[i], out)) {
            std::cout << "Test " << (i + 1) << " -> [" << out << "] len=" << out.size() << "\n";
        } else {
            std::cout << "Test " << (i + 1) << " -> error\n";
        }
    }
    return 0;
}