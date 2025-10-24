#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cctype>
#include <limits>

static const size_t MAX_INPUT_BYTES = 10 * 1024 * 1024; // 10MB

static bool is_valid_utf8(const std::vector<unsigned char>& data) {
    size_t i = 0;
    while (i < data.size()) {
        unsigned char c = data[i];
        if (c <= 0x7F) {
            i++;
        } else if ((c >> 5) == 0x6) { // 110xxxxx
            if (i + 1 >= data.size()) return false;
            unsigned char c1 = data[i + 1];
            if ((c1 & 0xC0) != 0x80) return false;
            unsigned int code = ((c & 0x1F) << 6) | (c1 & 0x3F);
            if (code < 0x80) return false; // overlong
            i += 2;
        } else if ((c >> 4) == 0xE) { // 1110xxxx
            if (i + 2 >= data.size()) return false;
            unsigned char c1 = data[i + 1], c2 = data[i + 2];
            if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80) return false;
            unsigned int code = ((c & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
            if (code < 0x800) return false; // overlong
            if (code >= 0xD800 && code <= 0xDFFF) return false; // surrogate
            i += 3;
        } else if ((c >> 3) == 0x1E) { // 11110xxx
            if (i + 3 >= data.size()) return false;
            unsigned char c1 = data[i + 1], c2 = data[i + 2], c3 = data[i + 3];
            if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) return false;
            unsigned int code = ((c & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
            if (code < 0x10000 || code > 0x10FFFF) return false; // overlong or out of range
            i += 4;
        } else {
            return false;
        }
    }
    return true;
}

static std::string base64_encode(const std::string &in) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((in.size() + 2) / 3) * 4);

    size_t i = 0;
    while (i + 2 < in.size()) {
        unsigned int n = ((unsigned char)in[i] << 16) | ((unsigned char)in[i+1] << 8) | (unsigned char)in[i+2];
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back(tbl[(n >> 6) & 63]);
        out.push_back(tbl[n & 63]);
        i += 3;
    }
    if (i + 1 < in.size()) {
        unsigned int n = ((unsigned char)in[i] << 16) | ((unsigned char)in[i+1] << 8);
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back(tbl[(n >> 6) & 63]);
        out.push_back('=');
    } else if (i < in.size()) {
        unsigned int n = ((unsigned char)in[i] << 16);
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back('=');
        out.push_back('=');
    }
    return out;
}

static std::vector<unsigned char> base64_decode(const std::string &in) {
    static int8_t map[256];
    static bool init = false;
    if (!init) {
        for (int i = 0; i < 256; ++i) map[i] = -1;
        const std::string idx = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int i = 0; i < 64; ++i) map[(unsigned char)idx[i]] = (int8_t)i;
        map[(unsigned char)'='] = -2;
        init = true;
    }

    std::vector<unsigned char> out;
    out.reserve((in.size() / 4) * 3);

    int val[4];
    int valCount = 0;

    auto flush_quad = [&](int v0, int v1, int v2, int v3) {
        if (v2 == -2 && v3 != -2) throw std::runtime_error("Invalid padding");
        if (v3 == -2) {
            if (v2 == -2) {
                // 2 paddings -> 1 byte
                unsigned int n = ((unsigned int)v0 << 18) | ((unsigned int)v1 << 12);
                out.push_back((unsigned char)((n >> 16) & 0xFF));
            } else {
                // 1 padding -> 2 bytes
                unsigned int n = ((unsigned int)v0 << 18) | ((unsigned int)v1 << 12) | ((unsigned int)v2 << 6);
                out.push_back((unsigned char)((n >> 16) & 0xFF));
                out.push_back((unsigned char)((n >> 8) & 0xFF));
            }
        } else {
            unsigned int n = ((unsigned int)v0 << 18) | ((unsigned int)v1 << 12) | ((unsigned int)v2 << 6) | (unsigned int)v3;
            out.push_back((unsigned char)((n >> 16) & 0xFF));
            out.push_back((unsigned char)((n >> 8) & 0xFF));
            out.push_back((unsigned char)(n & 0xFF));
        }
    };

    for (char ch : in) {
        unsigned char c = (unsigned char)ch;
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue; // skip whitespace
        int m = map[c];
        if (m == -1) throw std::runtime_error("Invalid Base64 character");
        val[valCount++] = m;
        if (valCount == 4) {
            flush_quad(val[0], val[1], val[2], val[3]);
            valCount = 0;
        }
    }
    if (valCount != 0) throw std::runtime_error("Base64 length not multiple of 4");
    return out;
}

std::string process_request(const std::vector<unsigned char>& raw_data) {
    if (raw_data.size() > MAX_INPUT_BYTES) {
        throw std::runtime_error("Input too large");
    }
    // Validate raw_data as UTF-8 (base64 string should be ASCII valid UTF-8)
    if (!is_valid_utf8(raw_data)) {
        throw std::runtime_error("raw_data is not valid UTF-8");
    }
    std::string b64_str(raw_data.begin(), raw_data.end());
    std::vector<unsigned char> decoded = base64_decode(b64_str);
    if (!is_valid_utf8(decoded)) {
        throw std::runtime_error("Decoded payload is not valid UTF-8");
    }
    std::string stored_data(decoded.begin(), decoded.end());
    return stored_data;
}

int main() {
    std::vector<std::string> originals = {
        "{\"name\":\"Alice\",\"age\":30}",
        "<person><name>Alice</name><age>30</age></person>",
        "[\"x\",1,true,null]",
        "<root/>",
        "Hello, 世界"
    };
    for (size_t i = 0; i < originals.size(); ++i) {
        try {
            std::string b64 = base64_encode(originals[i]);
            std::vector<unsigned char> raw(b64.begin(), b64.end());
            std::string result = process_request(raw);
            std::cout << "Test " << (i+1) << " OK: " << result << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Test " << (i+1) << " Error: " << ex.what() << "\n";
        }
    }
    return 0;
}