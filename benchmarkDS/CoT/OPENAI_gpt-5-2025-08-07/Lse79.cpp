// Chain-of-Through process in comments:
// 1) Understand: allocate two 1024-byte buffers (buf, buf1) and copy up to 100 chars each sequentially from a stream;
//    also allocate a 1024-byte buffer and read up to 1024 chars.
// 2) Security: bound-checked loops, handle EOF.
// 3) Secure coding: avoid raw unchecked copies; free allocated memory before return.
// 4) Review: loops capped at 100/1024, memory freed.
// 5) Output: final secure code.

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

struct Copy100Out {
    std::string first;   // data read into buf
    std::string second;  // data read into buf1
};

Copy100Out allocateAndCopy100Twice(std::istream& in) {
    char* buf = new (std::nothrow) char[1024];
    char* buf1 = new (std::nothrow) char[1024];
    if (!buf || !buf1) {
        delete[] buf;
        delete[] buf1;
        throw std::bad_alloc();
    }

    size_t n1 = 0;
    while (n1 < 100) {
        int c = in.get();
        if (c == EOF) break;
        buf[n1++] = static_cast<char>(c);
    }

    size_t n2 = 0;
    while (n2 < 100) {
        int c = in.get();
        if (c == EOF) break;
        buf1[n2++] = static_cast<char>(c);
    }

    std::string out1(buf, buf + n1);
    std::string out2(buf1, buf1 + n2);

    delete[] buf;
    delete[] buf1;
    return {out1, out2};
}

std::string allocateAndReadUpTo1024(std::istream& in) {
    char* buf = new (std::nothrow) char[1024];
    if (!buf) throw std::bad_alloc();

    size_t total = 0;
    while (total < 1024) {
        int c = in.get();
        if (c == EOF) break;
        buf[total++] = static_cast<char>(c);
    }
    std::string out(buf, buf + total);
    delete[] buf;
    return out;
}

static std::string printable(const std::string& s) {
    std::string r;
    r.reserve(s.size());
    for (char ch : s) {
        if (ch == '\n') { r += "\\n"; }
        else if (ch == '\r') { r += "\\r"; }
        else { r.push_back(ch); }
    }
    return r;
}

int main() {
    // 5 test cases
    std::vector<std::string> inputs = {
        "HelloWorld",
        std::string(100, 'A'),
        std::string(150, 'B'),
        std::string(220, 'C'),
        std::string(1500, 'D')
    };

    for (size_t i = 0; i < inputs.size(); ++i) {
        std::cout << "=== C++ Test Case " << (i + 1) << " ===\n";
        {
            std::istringstream iss(inputs[i]);
            Copy100Out r = allocateAndCopy100Twice(iss);
            std::cout << "First 100 buf len=" << r.first.size() << " data=" << printable(r.first) << "\n";
            std::cout << "Second 100 buf1 len=" << r.second.size() << " data=" << printable(r.second) << "\n";
        }
        {
            std::istringstream iss(inputs[i]);
            std::string r = allocateAndReadUpTo1024(iss);
            std::cout << "Read up to 1024 len=" << r.size() << "\n";
        }
    }
    return 0;
}