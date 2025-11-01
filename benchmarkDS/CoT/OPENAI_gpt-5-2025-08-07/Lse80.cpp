#include <iostream>
#include <sstream>
#include <string>

static const std::size_t BUFFER_SIZE = 1024; // store up to 1023 chars

// Reads from provided istream until newline or EOF, consumes newline but does not store it.
// Stores at most 1023 characters into 'out'. Returns number of characters stored.
int scan(std::istream& in, std::string& out) {
    out.clear();
    out.reserve(std::min<std::size_t>(64, BUFFER_SIZE - 1));
    char ch;
    std::size_t stored = 0;
    while (in.get(ch)) {
        if (ch == '\n') {
            break; // consume newline
        }
        if (stored < BUFFER_SIZE - 1) {
            out.push_back(ch);
            ++stored;
        } else {
            // discard
        }
    }
    if (stored >= BUFFER_SIZE - 1) {
        // drain until newline/EOF
        while (in.get(ch)) {
            if (ch == '\n') break;
        }
    }
    return static_cast<int>(out.size());
}

int main() {
    // 5 test cases using istringstream
    std::string t1 = "Hello\n";
    std::string t2 = "World";
    std::string t3 = "\n";
    std::string t4(1023, 'A'); t4.push_back('\n');
    std::string t5(1500, 'B'); t5.push_back('\n');

    std::string out;
    {
        std::istringstream ss(t1);
        int c = scan(ss, out);
        std::cout << "T1 count=" << c << " data=\"" << out << "\"\n";
    }
    {
        std::istringstream ss(t2);
        int c = scan(ss, out);
        std::cout << "T2 count=" << c << " data=\"" << out << "\"\n";
    }
    {
        std::istringstream ss(t3);
        int c = scan(ss, out);
        std::cout << "T3 count=" << c << " data=\"" << out << "\"\n";
    }
    {
        std::istringstream ss(t4);
        int c = scan(ss, out);
        std::cout << "T4 count=" << c << " data length=" << out.size()
                  << " first5=\"" << out.substr(0, std::min<std::size_t>(5, out.size()))
                  << "\" last5=\"" << out.substr(out.size() >= 5 ? out.size()-5 : 0) << "\"\n";
    }
    {
        std::istringstream ss(t5);
        int c = scan(ss, out);
        std::cout << "T5 count=" << c << " data length=" << out.size()
                  << " first5=\"" << out.substr(0, std::min<std::size_t>(5, out.size()))
                  << "\" last5=\"" << out.substr(out.size() >= 5 ? out.size()-5 : 0) << "\"\n";
    }

    return 0;
}