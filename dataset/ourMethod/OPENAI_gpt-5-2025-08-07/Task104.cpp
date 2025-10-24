#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

struct BufferResult {
    bool success;
    std::string content;
    bool truncated;
    std::string message;
};

static std::string sanitize(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    for (unsigned char ch : input) {
        if (ch == '\n' || ch == '\t' || (ch >= 32 && ch <= 126)) {
            out.push_back(static_cast<char>(ch));
        }
    }
    return out;
}

BufferResult handleInputToFixedBuffer(const std::string& input, std::size_t bufferSize) {
    if (bufferSize < 1 || bufferSize > 4096) {
        return BufferResult{false, "", false, "Invalid buffer size"};
    }
    std::string sanitized = sanitize(input);
    std::size_t copyLen = std::min(sanitized.size(), bufferSize);
    std::vector<char> buffer(bufferSize);
    for (std::size_t i = 0; i < copyLen; ++i) {
        buffer[i] = sanitized[i];
    }
    std::string content(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(copyLen));
    bool truncated = sanitized.size() > copyLen;
    return BufferResult{true, content, truncated, "OK"};
}

int main() {
    std::size_t bufSize = 16;
    std::string tests[5] = {
        "Hello World",                       // fits
        "",                                  // empty
        "This is a very long input string!!!", // long
        std::string("Hi") + char(1) + char(2) + " there", // control chars
        "1234567890ABCDEF"                   // boundary
    };

    for (int i = 0; i < 5; ++i) {
        BufferResult res = handleInputToFixedBuffer(tests[i], bufSize);
        std::cout << "Test " << (i + 1) << ": "
                  << "BufferResult{success=" << (res.success ? "true" : "false")
                  << ", content=\"" << res.content
                  << "\", truncated=" << (res.truncated ? "true" : "false")
                  << ", message=\"" << res.message << "\"}" << std::endl;
    }
    return 0;
}