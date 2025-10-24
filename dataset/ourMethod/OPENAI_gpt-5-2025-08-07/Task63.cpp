#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <limits>

struct ProcessResult {
    bool ok;
    std::string data;   // Deserialized (validated) UTF-8 text (JSON or XML)
    std::string error;  // Error message when ok == false
};

static bool is_valid_utf8(const std::string& s) {
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(s.data());
    size_t len = s.size();
    size_t i = 0;
    while (i < len) {
        unsigned char c = bytes[i];
        if ((c & 0x80u) == 0) { // ASCII
            i++;
        } else if ((c & 0xE0u) == 0xC0u) { // 2-byte
            if (i + 1 >= len) return false;
            unsigned char c1 = bytes[i+1];
            if ((c & 0xFEu) == 0xC0u) return false; // overlong
            if ((c1 & 0xC0u) != 0x80u) return false;
            i += 2;
        } else if ((c & 0xF0u) == 0xE0u) { // 3-byte
            if (i + 2 >= len) return false;
            unsigned char c1 = bytes[i+1];
            unsigned char c2 = bytes[i+2];
            if ((c1 & 0xC0u) != 0x80u || (c2 & 0xC0u) != 0x80u) return false;
            // Check for overlongs and surrogates
            if (c == 0xE0u && (c1 & 0xE0u) == 0x80u) return false;
            if (c == 0xEDu && (c1 & 0xE0u) == 0xA0u) return false;
            i += 3;
        } else if ((c & 0xF8u) == 0xF0u) { // 4-byte
            if (i + 3 >= len) return false;
            unsigned char c1 = bytes[i+1];
            unsigned char c2 = bytes[i+2];
            unsigned char c3 = bytes[i+3];
            if ((c1 & 0xC0u) != 0x80u || (c2 & 0xC0u) != 0x80u || (c3 & 0xC0u) != 0x80u) return false;
            // Check for overlongs and > U+10FFFF
            if (c == 0xF0u && (c1 & 0xF0u) == 0x80u) return false;
            if (c > 0xF4u || (c == 0xF4u && c1 > 0x8Fu)) return false;
            i += 4;
        } else {
            return false;
        }
    }
    return true;
}

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

static bool iequals_prefix(const std::string& s, size_t pos, const std::string& pat) {
    if (pos + pat.size() > s.size()) return false;
    for (size_t i = 0; i < pat.size(); ++i) {
        char a = s[pos + i], b = pat[i];
        if (std::tolower(static_cast<unsigned char>(a)) != std::tolower(static_cast<unsigned char>(b))) return false;
    }
    return true;
}

static bool contains_no_doctype(const std::string& s) {
    for (size_t i = 0; i + 8 <= s.size(); ++i) {
        if (iequals_prefix(s, i, "<!doctype")) return false;
    }
    return true;
}

static bool looks_like_json(const std::string& s) {
    std::string t = trim(s);
    if (t.empty()) return false;
    char first = t.front();
    char last = t.back();
    if (first == '{' && last == '}') return true;
    if (first == '[' && last == ']') return true;
    return false;
}

static bool looks_like_xml(const std::string& s) {
    std::string t = trim(s);
    if (t.empty()) return false;
    if (!contains_no_doctype(t)) return false;
    // Allow optional XML declaration
    size_t pos = 0;
    if (t.size() >= 5 && iequals_prefix(t, 0, "<?xml")) {
        size_t endDecl = t.find("?>", 5);
        if (endDecl == std::string::npos) return false;
        pos = endDecl + 2;
        while (pos < t.size() && std::isspace(static_cast<unsigned char>(t[pos]))) pos++;
    }
    if (pos >= t.size()) return false;
    if (t[pos] != '<') return false;
    if (t.back() != '>') return false;
    return true;
}

static bool base64_decode(const std::string& in, std::vector<unsigned char>& out, std::string& error) {
    static const int8_t T[256] = {
        -1
    };
    static bool table_init = false;
    static int8_t D[256];
    if (!table_init) {
        std::fill(std::begin(D), std::end(D), -1);
        const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int i = 0; i < 64; ++i) D[static_cast<unsigned char>(chars[i])] = static_cast<int8_t>(i);
        D[static_cast<unsigned char>('=')] = -2; // padding
        table_init = true;
    }

    // Sanitize and validate input: allow only base64 characters and whitespace
    std::string s;
    s.reserve(in.size());
    for (unsigned char c : in) {
        if (c > 0x7Fu) { error = "Input not ASCII/UTF-8."; return false; }
        if (std::isspace(c)) continue;
        if (D[c] == -1 && c != '=') { error = "Invalid Base64 character."; return false; }
        s.push_back(static_cast<char>(c));
    }
    if (s.size() == 0 || s.size() % 4 != 0) {
        error = "Base64 length is invalid.";
        return false;
    }
    // Count padding
    size_t pad = 0;
    if (!s.empty() && s[s.size()-1] == '=') pad++;
    if (s.size() >= 2 && s[s.size()-2] == '=') pad++;
    size_t groups = s.size() / 4;
    size_t out_len = groups * 3 - pad;
    if (out_len > (1u << 26)) { // ~64MB cap
        error = "Decoded data too large.";
        return false;
    }
    out.clear();
    out.reserve(out_len);

    for (size_t i = 0; i < s.size(); i += 4) {
        int8_t a = D[static_cast<unsigned char>(s[i])];
        int8_t b = D[static_cast<unsigned char>(s[i+1])];
        int8_t c = D[static_cast<unsigned char>(s[i+2])];
        int8_t d = D[static_cast<unsigned char>(s[i+3])];
        if (a < 0 || b < 0) { error = "Invalid Base64 quartet."; return false; }
        uint32_t triple = (static_cast<uint32_t>(a) << 18) |
                          (static_cast<uint32_t>(b) << 12) |
                          ((c >= 0 ? static_cast<uint32_t>(c) : 0) << 6) |
                          ((d >= 0 ? static_cast<uint32_t>(d) : 0));
        out.push_back(static_cast<unsigned char>((triple >> 16) & 0xFFu));
        if (c >= 0) out.push_back(static_cast<unsigned char>((triple >> 8) & 0xFFu));
        if (d >= 0) out.push_back(static_cast<unsigned char>(triple & 0xFFu));
    }
    // Trim possible extra due to padding calc
    if (out.size() > out_len) out.resize(out_len);
    return true;
}

// Base64 encoder for test cases only (no line breaks).
static std::string base64_encode(const std::string& input) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const unsigned char* data = reinterpret_cast<const unsigned char*>(input.data());
    size_t len = input.size();
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t triple = data[i] << 16;
        if (i + 1 < len) triple |= (data[i+1] << 8);
        if (i + 2 < len) triple |= data[i+2];
        out.push_back(chars[(triple >> 18) & 0x3F]);
        out.push_back(chars[(triple >> 12) & 0x3F]);
        if (i + 1 < len)
            out.push_back(chars[(triple >> 6) & 0x3F]);
        else
            out.push_back('=');
        if (i + 2 < len)
            out.push_back(chars[triple & 0x3F]);
        else
            out.push_back('=');
    }
    return out;
}

ProcessResult process_request(const std::string& raw_b64, const std::string& format) {
    ProcessResult res{false, "", ""};

    // Rule#1: Validate input size and UTF-8
    const size_t MAX_RAW = 1u << 20; // 1 MiB
    if (raw_b64.size() == 0 || raw_b64.size() > MAX_RAW) {
        res.error = "Invalid input size.";
        return res;
    }
    if (!is_valid_utf8(raw_b64)) {
        res.error = "raw_data is not valid UTF-8.";
        return res;
    }

    // Decode Base64
    std::vector<unsigned char> decoded;
    std::string dec_err;
    if (!base64_decode(raw_b64, decoded, dec_err)) {
        res.error = dec_err;
        return res;
    }
    std::string decoded_text(decoded.begin(), decoded.end());

    // Validate decoded text is UTF-8
    if (!is_valid_utf8(decoded_text)) {
        res.error = "Decoded data is not valid UTF-8.";
        return res;
    }

    // "Deserialize": validate format and ensure safe XML (no external entities)
    std::string f = format;
    std::transform(f.begin(), f.end(), f.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    if (f == "json") {
        if (!looks_like_json(decoded_text)) {
            res.error = "Decoded text does not look like valid JSON.";
            return res;
        }
    } else if (f == "xml") {
        if (!looks_like_xml(decoded_text)) {
            res.error = "Decoded text does not look like safe XML.";
            return res;
        }
    } else {
        res.error = "Unsupported format. Use 'json' or 'xml'.";
        return res;
    }

    // Store and return
    res.ok = true;
    res.data = decoded_text;
    return res;
}

int main() {
    // Prepare 5 test cases
    std::vector<std::pair<std::string, std::string>> tests;

    std::string json1 = R"({"name":"Alice","age":30})";
    tests.emplace_back(base64_encode(json1), "json");

    std::string xml1 = R"(<root><a>1</a><b>2</b></root>)";
    tests.emplace_back(base64_encode(xml1), "xml");

    std::string invalid_b64 = "!!!@@@"; // invalid base64
    tests.emplace_back(invalid_b64, "json");

    std::string json2 = R"(["x", 1, true, null])";
    tests.emplace_back(base64_encode(json2), "json");

    std::string xml_doctype = R"(<?xml version="1.0"?><!DOCTYPE foo [ <!ENTITY xxe SYSTEM "file:///etc/passwd"> ]><root>&xxe;</root>)";
    tests.emplace_back(base64_encode(xml_doctype), "xml");

    for (size_t i = 0; i < tests.size(); ++i) {
        ProcessResult r = process_request(tests[i].first, tests[i].second);
        std::cout << "Test " << (i+1) << " (" << tests[i].second << "): ";
        if (r.ok) {
            std::cout << "OK | Data: " << r.data << "\n";
        } else {
            std::cout << "ERROR | " << r.error << "\n";
        }
    }

    return 0;
}