#include <iostream>
#include <sstream>
#include <string>

static constexpr std::size_t MAX_BUFFER_SIZE = 1024; // bytes; will store at most 1023 chars

// Reads from the provided std::istream up to (bufsize-1) characters or until a newline.
// The newline (if encountered during reading) is consumed but not stored.
// Returns the number of characters stored in 'out'.
std::size_t scan(std::istream& in, std::string& out, std::size_t bufsize = MAX_BUFFER_SIZE) {
    out.clear();
    if (bufsize == 0) {
        return 0;
    }
    std::size_t limit = (bufsize > 0) ? (bufsize - 1) : 0;

    while (out.size() < limit) {
        int c = in.get();
        if (c == EOF) {
            break;
        }
        if (c == '\n') {
            break; // consume newline; do not store
        }
        out.push_back(static_cast<char>(c));
    }
    // Do not attempt to read further if limit reached; leave any newline for the next consumer
    return out.size();
}

static std::string visible(const std::string& s) {
    std::string r;
    r.reserve(s.size() * 2);
    for (unsigned char ch : s) {
        if (ch == '\n') r += "\\n";
        else if (ch == '\r') r += "\\r";
        else if (ch == '\t') r += "\\t";
        else if (ch < 0x20 || ch == 0x7F) {
            char buf[5];
            std::snprintf(buf, sizeof(buf), "\\x%02X", static_cast<unsigned>(ch));
            r += buf;
        } else r.push_back(static_cast<char>(ch));
    }
    return r;
}

int main() {
    // Test case 1: Simple line with newline
    {
        std::istringstream iss("Hello\nWorld");
        std::string out;
        std::size_t n = scan(iss, out, MAX_BUFFER_SIZE);
        std::cout << "T1 count=" << n << " text=\"" << visible(out) << "\"\n";
    }

    // Test case 2: Empty line (just newline)
    {
        std::istringstream iss("\n");
        std::string out;
        std::size_t n = scan(iss, out, MAX_BUFFER_SIZE);
        std::cout << "T2 count=" << n << " text=\"" << visible(out) << "\"\n";
    }

    // Test case 3: Longer than buffer (1200 'A's and newline)
    {
        std::string longLine(1200, 'A');
        longLine.push_back('\n');
        std::istringstream iss(longLine);
        std::string out;
        std::size_t n = scan(iss, out, MAX_BUFFER_SIZE);
        std::cout << "T3 count=" << n << " (expected 1023) text_len=" << out.size() << "\n";
    }

    // Test case 4: No newline before EOF
    {
        std::istringstream iss("NoNewlineEOF");
        std::string out;
        std::size_t n = scan(iss, out, MAX_BUFFER_SIZE);
        std::cout << "T4 count=" << n << " text=\"" << visible(out) << "\"\n";
    }

    // Test case 5: CRLF before next data
    {
        std::string s = "CR\rLF\r\nNext";
        std::istringstream iss(s);
        std::string out;
        std::size_t n = scan(iss, out, MAX_BUFFER_SIZE);
        std::cout << "T5 count=" << n << " text=\"" << visible(out) << "\"\n";
    }

    return 0;
}