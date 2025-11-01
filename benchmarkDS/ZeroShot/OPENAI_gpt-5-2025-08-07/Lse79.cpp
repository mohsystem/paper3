#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

struct CopyResult {
    std::vector<uint8_t> buffer;
    size_t bytesCopied;
};

static size_t utf8_prefix_len_for_n_chars(const std::string& s, size_t max_chars) {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(s.data());
    size_t i = 0;
    size_t chars = 0;
    const size_t len = s.size();

    auto valid_cont = [&](size_t idx) -> bool {
        return idx < len && (p[idx] & 0xC0) == 0x80;
    };

    while (i < len && chars < max_chars) {
        unsigned char c = p[i];
        size_t clen = 0;
        if ((c & 0x80) == 0x00) {
            clen = 1;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= len || !valid_cont(i + 1)) break;
            clen = 2;
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 >= len || !valid_cont(i + 1) || !valid_cont(i + 2)) break;
            clen = 3;
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 >= len || !valid_cont(i + 1) || !valid_cont(i + 2) || !valid_cont(i + 3)) break;
            clen = 4;
        } else {
            break;
        }
        if (i + clen > len) break;
        i += clen;
        chars++;
    }
    return i;
}

static CopyResult allocBuf(const std::string& input) {
    CopyResult res;
    res.buffer.assign(1024, 0);
    std::string in = input;
    size_t byteCount = utf8_prefix_len_for_n_chars(in, 100);
    size_t toCopy = std::min(byteCount, res.buffer.size());
    std::copy(in.begin(), in.begin() + static_cast<std::ptrdiff_t>(toCopy), res.buffer.begin());
    res.bytesCopied = toCopy;
    return res;
}

static CopyResult allocBuf1(const std::string& input) {
    CopyResult res;
    res.buffer.assign(1024, 0);
    std::string in = input;
    size_t byteCount = utf8_prefix_len_for_n_chars(in, 100);
    size_t toCopy = std::min(byteCount, res.buffer.size());
    std::copy(in.begin(), in.begin() + static_cast<std::ptrdiff_t>(toCopy), res.buffer.begin());
    res.bytesCopied = toCopy;
    return res;
}

static std::string safePreview(const std::vector<uint8_t>& buf, size_t n) {
    size_t end = std::min(n, buf.size());
    size_t limit = 0;
    while (limit < end && buf[limit] != 0) limit++;
    return std::string(reinterpret_cast<const char*>(buf.data()), reinterpret_cast<const char*>(buf.data()) + limit);
}

int main() {
    std::vector<std::string> tests = {
        "Hello, world!",
        std::string(100, 'A'),
        std::string(150, 'B'),
        "",
        std::string("The quick brown fox jumps over the lazy dog 0123456789 Sphinx of black quartz, judge my vow. ") +
        std::string("The quick brown fox jumps over the lazy dog 0123456789 Sphinx of black quartz, judge my vow. ") +
        std::string("The quick brown fox jumps over the lazy dog 0123456789 Sphinx of black quartz, judge my vow. ")
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        CopyResult r1 = allocBuf(tests[i]);
        CopyResult r2 = allocBuf1(tests[i]);
        std::cout << "Test " << (i + 1) << ": bytes1=" << r1.bytesCopied << ", bytes2=" << r2.bytesCopied << "\n";
        std::cout << "Preview1: " << safePreview(r1.buffer, 60) << "\n";
        std::cout << "Preview2: " << safePreview(r2.buffer, 60) << "\n";
    }
    return 0;
}