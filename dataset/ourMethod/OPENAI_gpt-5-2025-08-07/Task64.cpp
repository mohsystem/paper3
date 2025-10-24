#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <limits>
#include <curl/curl.h>

enum class JsonType { Null, Bool, Number, String, Array, Object };

struct JsonValue;

using JsonArray = std::vector<JsonValue>;
using JsonObject = std::map<std::string, JsonValue>;

struct JsonValue {
    JsonType type{JsonType::Null};
    bool boolean{false};
    double number{0.0};
    std::string string;
    JsonArray array;
    JsonObject object;

    static JsonValue makeNull() {
        return JsonValue{};
    }
    static JsonValue makeBool(bool b) {
        JsonValue v; v.type = JsonType::Bool; v.boolean = b; return v;
    }
    static JsonValue makeNumber(double n) {
        JsonValue v; v.type = JsonType::Number; v.number = n; return v;
    }
    static JsonValue makeString(const std::string& s) {
        JsonValue v; v.type = JsonType::String; v.string = s; return v;
    }
    static JsonValue makeArray(JsonArray a) {
        JsonValue v; v.type = JsonType::Array; v.array = std::move(a); return v;
    }
    static JsonValue makeObject(JsonObject o) {
        JsonValue v; v.type = JsonType::Object; v.object = std::move(o); return v;
    }
};

static std::string json_type_name(JsonType t) {
    switch (t) {
        case JsonType::Null: return "null";
        case JsonType::Bool: return "bool";
        case JsonType::Number: return "number";
        case JsonType::String: return "string";
        case JsonType::Array: return "array";
        case JsonType::Object: return "object";
    }
    return "unknown";
}

static void print_json_summary(const JsonValue& v, int indent = 0, int max_depth = 2) {
    auto pad = [indent]() { for (int i=0;i<indent;i++) std::cout << ' '; };
    pad();
    std::cout << json_type_name(v.type);
    switch (v.type) {
        case JsonType::Null: std::cout << "\n"; break;
        case JsonType::Bool: std::cout << ": " << (v.boolean ? "true" : "false") << "\n"; break;
        case JsonType::Number: std::cout << ": " << v.number << "\n"; break;
        case JsonType::String:
            std::cout << ": \"" << (v.string.size() > 60 ? v.string.substr(0,57) + "..." : v.string) << "\"\n";
            break;
        case JsonType::Array:
            std::cout << " size=" << v.array.size() << "\n";
            if (indent/2 < max_depth) {
                for (const auto& e : v.array) print_json_summary(e, indent+2, max_depth);
            }
            break;
        case JsonType::Object:
            std::cout << " size=" << v.object.size() << "\n";
            if (indent/2 < max_depth) {
                for (const auto& kv : v.object) {
                    for (int i=0;i<indent+2;i++) std::cout << ' ';
                    std::cout << "\"" << kv.first << "\": ";
                    std::cout << json_type_name(kv.second.type) << "\n";
                }
            }
            break;
    }
}

struct JsonParser {
    const std::string& s;
    size_t i{0};
    std::string error;

    explicit JsonParser(const std::string& input) : s(input) {}

    void skip_ws() {
        while (i < s.size() && (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t')) i++;
    }

    bool consume(char c) {
        if (i < s.size() && s[i] == c) { i++; return true; }
        return false;
    }

    bool match(const char* lit) {
        size_t j = 0;
        size_t start = i;
        while (lit[j] != '\0') {
            if (i >= s.size() || s[i] != lit[j]) { i = start; return false; }
            i++; j++;
        }
        return true;
    }

    static bool is_hex(char c) {
        return (c >= '0' && c <= '9') || (c>='a' && c<='f') || (c>='A' && c<='F');
    }

    static int hex_val(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    }

    static void append_utf8_from_codepoint(uint32_t cp, std::string& out) {
        if (cp <= 0x7F) {
            out.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FF) {
            out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }

    std::optional<std::string> parse_string() {
        if (!consume('"')) { error = "Expected '\"' at pos " + std::to_string(i); return std::nullopt; }
        std::string out;
        while (i < s.size()) {
            char c = s[i++];
            if (c == '"') return out;
            if (static_cast<unsigned char>(c) < 0x20) {
                error = "Unescaped control char in string at pos " + std::to_string(i-1);
                return std::nullopt;
            }
            if (c == '\\') {
                if (i >= s.size()) { error = "Invalid escape at end of input"; return std::nullopt; }
                char e = s[i++];
                switch (e) {
                    case '"': out.push_back('"'); break;
                    case '\\': out.push_back('\\'); break;
                    case '/': out.push_back('/'); break;
                    case 'b': out.push_back('\b'); break;
                    case 'f': out.push_back('\f'); break;
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    case 'u': {
                        if (i + 4 > s.size()) { error = "Invalid \\u escape (too short)"; return std::nullopt; }
                        uint32_t code = 0;
                        for (int k=0;k<4;k++) {
                            if (!is_hex(s[i+k])) { error = "Invalid hex in \\u escape"; return std::nullopt; }
                            code = (code << 4) | static_cast<uint32_t>(hex_val(s[i+k]));
                        }
                        i += 4;
                        // Handle surrogate pairs
                        if (code >= 0xD800 && code <= 0xDBFF) {
                            if (i + 6 <= s.size() && s[i] == '\\' && s[i+1] == 'u') {
                                i += 2;
                                if (i + 4 > s.size()) { error = "Invalid low surrogate"; return std::nullopt; }
                                uint32_t low = 0;
                                for (int k=0;k<4;k++) {
                                    if (!is_hex(s[i+k])) { error = "Invalid hex in low surrogate"; return std::nullopt; }
                                    low = (low << 4) | static_cast<uint32_t>(hex_val(s[i+k]));
                                }
                                i += 4;
                                if (low >= 0xDC00 && low <= 0xDFFF) {
                                    uint32_t cp = 0x10000 + (((code - 0xD800) << 10) | (low - 0xDC00));
                                    append_utf8_from_codepoint(cp, out);
                                } else {
                                    error = "Expected low surrogate after high surrogate";
                                    return std::nullopt;
                                }
                            } else {
                                error = "Missing low surrogate after high surrogate";
                                return std::nullopt;
                            }
                        } else {
                            append_utf8_from_codepoint(code, out);
                        }
                        break;
                    }
                    default:
                        error = "Invalid escape sequence \\" + std::string(1, e);
                        return std::nullopt;
                }
            } else {
                out.push_back(c);
            }
        }
        error = "Unterminated string";
        return std::nullopt;
    }

    std::optional<double> parse_number() {
        size_t start = i;
        if (i < s.size() && (s[i] == '-')) i++;
        if (i < s.size() && s[i] == '0') {
            i++;
        } else {
            if (i >= s.size() || !std::isdigit(static_cast<unsigned char>(s[i]))) { error = "Invalid number"; return std::nullopt; }
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) i++;
        }
        if (i < s.size() && s[i] == '.') {
            i++;
            if (i >= s.size() || !std::isdigit(static_cast<unsigned char>(s[i]))) { error = "Invalid fraction"; return std::nullopt; }
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) i++;
        }
        if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
            i++;
            if (i < s.size() && (s[i] == '+' || s[i] == '-')) i++;
            if (i >= s.size() || !std::isdigit(static_cast<unsigned char>(s[i]))) { error = "Invalid exponent"; return std::nullopt; }
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) i++;
        }
        char* endptr = nullptr;
        std::string num = s.substr(start, i - start);
        errno = 0;
        double val = std::strtod(num.c_str(), &endptr);
        if (errno == ERANGE) { error = "Number out of range"; return std::nullopt; }
        return val;
    }

    std::optional<JsonValue> parse_value() {
        skip_ws();
        if (i >= s.size()) { error = "Unexpected end of input"; return std::nullopt; }
        char c = s[i];
        if (c == 'n') {
            if (match("null")) return JsonValue::makeNull();
            error = "Invalid token at pos " + std::to_string(i);
            return std::nullopt;
        }
        if (c == 't') {
            if (match("true")) return JsonValue::makeBool(true);
            error = "Invalid token at pos " + std::to_string(i);
            return std::nullopt;
        }
        if (c == 'f') {
            if (match("false")) return JsonValue::makeBool(false);
            error = "Invalid token at pos " + std::to_string(i);
            return std::nullopt;
        }
        if (c == '"') {
            auto str = parse_string();
            if (!str) return std::nullopt;
            return JsonValue::makeString(*str);
        }
        if (c == '[') {
            return parse_array();
        }
        if (c == '{') {
            return parse_object();
        }
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
            auto num = parse_number();
            if (!num) return std::nullopt;
            return JsonValue::makeNumber(*num);
        }
        error = std::string("Unexpected character '") + c + "' at pos " + std::to_string(i);
        return std::nullopt;
    }

    std::optional<JsonValue> parse_array() {
        if (!consume('[')) { error = "Expected '['"; return std::nullopt; }
        JsonArray arr;
        skip_ws();
        if (consume(']')) return JsonValue::makeArray(std::move(arr));
        while (true) {
            auto val = parse_value();
            if (!val) return std::nullopt;
            arr.push_back(std::move(*val));
            skip_ws();
            if (consume(']')) break;
            if (!consume(',')) { error = "Expected ',' or ']' in array at pos " + std::to_string(i); return std::nullopt; }
            skip_ws();
        }
        return JsonValue::makeArray(std::move(arr));
    }

    std::optional<JsonValue> parse_object() {
        if (!consume('{')) { error = "Expected '{'"; return std::nullopt; }
        JsonObject obj;
        skip_ws();
        if (consume('}')) return JsonValue::makeObject(std::move(obj));
        while (true) {
            skip_ws();
            auto key = parse_string();
            if (!key) { if (error.empty()) error = "Expected string key in object"; return std::nullopt; }
            skip_ws();
            if (!consume(':')) { error = "Expected ':' after key at pos " + std::to_string(i); return std::nullopt; }
            skip_ws();
            auto val = parse_value();
            if (!val) return std::nullopt;
            obj[*key] = std::move(*val);
            skip_ws();
            if (consume('}')) break;
            if (!consume(',')) { error = "Expected ',' or '}' in object at pos " + std::to_string(i); return std::nullopt; }
            skip_ws();
        }
        return JsonValue::makeObject(std::move(obj));
    }

    std::optional<JsonValue> parse() {
        auto v = parse_value();
        if (!v) return std::nullopt;
        skip_ws();
        if (i != s.size()) {
            error = "Trailing data after JSON at pos " + std::to_string(i);
            return std::nullopt;
        }
        return v;
    }
};

struct FetchResult {
    bool ok{false};
    std::string data;
    std::string error;
    long http_status{0};
};

struct WriteCtx {
    std::string* out;
    size_t max;
};

static size_t curl_write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t bytes = size * nmemb;
    WriteCtx* ctx = static_cast<WriteCtx*>(userdata);
    if (!ctx || !ctx->out) return 0;
    if (ctx->out->size() + bytes > ctx->max) {
        return 0; // signal error to avoid excessive memory
    }
    ctx->out->append(ptr, bytes);
    return bytes;
}

static bool validate_url(const std::string& url, std::string& err) {
    if (url.empty() || url.size() > 2048) { err = "URL length invalid"; return false; }
    auto lower = url;
    for (auto& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (!(lower.rfind("https://", 0) == 0 || lower.rfind("http://", 0) == 0)) {
        err = "URL must start with http:// or https://";
        return false;
    }
    return true;
}

static FetchResult https_get(const std::string& url, size_t max_bytes = 5 * 1024 * 1024) {
    FetchResult r;
    std::string vErr;
    if (!validate_url(url, vErr)) { r.ok = false; r.error = vErr; return r; }

    CURL* curl = curl_easy_init();
    if (!curl) { r.ok = false; r.error = "Failed to initialize CURL"; return r; }

    char errbuf[CURL_ERROR_SIZE];
    errbuf[0] = '\0';
    std::string body;
    WriteCtx ctx{ &body, max_bytes };

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task64/1.0 (libcurl)");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    CURLcode code = curl_easy_perform(curl);
    long http_status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
    r.http_status = http_status;

    if (code != CURLE_OK) {
        r.ok = false;
        std::string emsg = errbuf[0] ? errbuf : curl_easy_strerror(code);
        if (code == CURLE_WRITE_ERROR && body.size() >= max_bytes) {
            r.error = "Response exceeds maximum size";
        } else {
            r.error = "CURL error: " + emsg;
        }
        curl_easy_cleanup(curl);
        return r;
    }

    if (http_status >= 400) {
        r.ok = false;
        r.error = "HTTP error status " + std::to_string(http_status);
        curl_easy_cleanup(curl);
        return r;
    }

    r.ok = true;
    r.data = std::move(body);
    curl_easy_cleanup(curl);
    return r;
}

static bool fetch_and_parse_json(const std::string& url, JsonValue& out, std::string& error) {
    auto res = https_get(url);
    if (!res.ok) { error = res.error; return false; }
    JsonParser p(res.data);
    auto v = p.parse();
    if (!v) { error = p.error.empty() ? "Invalid JSON" : p.error; return false; }
    out = std::move(*v);
    return true;
}

int main(int argc, char** argv) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    if (argc > 1) {
        std::string url(argv[1]);
        JsonValue v;
        std::string err;
        bool ok = fetch_and_parse_json(url, v, err);
        if (!ok) {
            std::cerr << "Error: " << err << "\n";
            curl_global_cleanup();
            return 1;
        }
        std::cout << "Successfully parsed JSON from " << url << "\n";
        print_json_summary(v);
        curl_global_cleanup();
        return 0;
    }

    // 5 test cases
    const std::vector<std::string> tests = {
        "https://jsonplaceholder.typicode.com/todos/1",
        "https://httpbin.org/json",
        "https://api.github.com",          // Requires User-Agent, we set one
        "https://example.com",             // Likely HTML -> parse failure expected
        "notaurl"                          // Invalid URL
    };
    for (const auto& url : tests) {
        std::cout << "Test URL: " << url << "\n";
        JsonValue v;
        std::string err;
        bool ok = fetch_and_parse_json(url, v, err);
        if (ok) {
            std::cout << "OK\n";
            print_json_summary(v);
        } else {
            std::cout << "Failed: " << err << "\n";
        }
        std::cout << "-------------------------\n";
    }

    curl_global_cleanup();
    return 0;
}