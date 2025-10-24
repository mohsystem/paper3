#include <bits/stdc++.h>
using namespace std;

class Task131 {
    static const size_t MAX_INPUT = 4096;
    static const size_t MAX_FIELD_LEN = 256;

    static bool is_ws(char c) {
        return c==' ' || c=='\n' || c=='\r' || c=='\t';
    }
    static size_t skip_ws(const string& s, size_t i, size_t end) {
        while (i < end && is_ws(s[i])) i++;
        return i;
    }
    struct StrRes {
        string value;
        size_t index;
        bool ok;
    };
    struct NumRes {
        long long value;
        size_t index;
        bool ok;
    };

    static StrRes parse_json_string(const string& s, size_t i, size_t end) {
        if (i >= end || s[i] != '"') return {"", i, false};
        i++;
        string out;
        bool esc = false;
        for (; i < end; i++) {
            char c = s[i];
            if (esc) {
                switch (c) {
                    case '"': out.push_back('"'); break;
                    case '\\': out.push_back('\\'); break;
                    case '/': out.push_back('/'); break;
                    case 'b': out.push_back('\b'); break;
                    case 'f': out.push_back('\f'); break;
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    case 'u':
                        return {"", i, false};
                    default:
                        return {"", i, false};
                }
                esc = false;
            } else {
                if (c == '\\') {
                    esc = true;
                } else if (c == '"') {
                    i++;
                    if (out.size() > MAX_FIELD_LEN) return {"", i, false};
                    return {out, i, true};
                } else {
                    if ((unsigned char)c < 0x20) return {"", i, false};
                    out.push_back(c);
                }
            }
        }
        return {"", i, false};
    }

    static NumRes parse_json_int(const string& s, size_t i, size_t end) {
        bool neg = false;
        if (i < end && (s[i] == '+' || s[i] == '-')) {
            neg = s[i] == '-';
            i++;
        }
        if (i >= end || !isdigit((unsigned char)s[i])) return {0, i, false};
        long long val = 0;
        while (i < end && isdigit((unsigned char)s[i])) {
            int d = s[i] - '0';
            val = val * 10 + d;
            if (val > INT_MAX) return {0, i, false};
            i++;
        }
        if (neg) val = -val;
        return {val, i, true};
    }

    static string json_escape(const string& s) {
        string out;
        out.push_back('"');
        for (char c : s) {
            switch (c) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\b': out += "\\b"; break;
                case '\f': out += "\\f"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:
                    if ((unsigned char)c < 0x20) {
                        char buf[7];
                        snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char)c);
                        out += buf;
                    } else {
                        out.push_back(c);
                    }
            }
        }
        out.push_back('"');
        return out;
    }

    static bool is_valid_email(const string& email) {
        if (email.size() < 3 || email.size() > MAX_FIELD_LEN) return false;
        size_t at = email.find('@');
        if (at == string::npos || at == 0 || at == email.size()-1) return false;
        if (email.find('@', at+1) != string::npos) return false;
        size_t dot = email.find('.', at+1);
        if (dot == string::npos || dot == at+1 || dot == email.size()-1) return false;
        for (unsigned char c : email) {
            if (c <= 0x1F || c >= 0x7F) return false;
            if (!(isalnum(c) || c=='.' || c=='-' || c=='_' || c=='+' || c=='@')) return false;
        }
        return true;
    }

public:
    static string secure_deserialize(const string& input) {
        if (input.size() > MAX_INPUT) return "";
        size_t n = input.size();
        size_t i = 0;
        i = skip_ws(input, i, n);
        if (i >= n || input[i] != '{') return "";
        i++;
        size_t j = n;
        if (j == 0) return "";
        j--;
        while (j < n && is_ws(input[j])) {
            if (j == 0) break;
            j--;
        }
        if (input[j] != '}') return "";
        size_t limit = j;

        bool idSet=false, nameSet=false, emailSet=false, ageSet=false;
        int idVal=0, ageVal=0;
        string nameVal, emailVal;

        i = skip_ws(input, i, limit);
        if (i < limit && input[i] == '}') {
            return "";
        }

        while (i < limit) {
            i = skip_ws(input, i, limit);
            StrRes k = parse_json_string(input, i, limit);
            if (!k.ok) return "";
            string key = k.value;
            i = k.index;
            i = skip_ws(input, i, limit);
            if (i >= limit || input[i] != ':') return "";
            i++;
            i = skip_ws(input, i, limit);

            if (key == "id" || key == "age") {
                NumRes nr = parse_json_int(input, i, limit);
                if (!nr.ok) return "";
                long long v = nr.value;
                if (v < 0 || v > INT_MAX) return "";
                if (key == "id") {
                    if (idSet) return "";
                    idVal = (int)v; idSet = true;
                } else {
                    if (ageSet) return "";
                    ageVal = (int)v; ageSet = true;
                }
                i = nr.index;
            } else if (key == "name" || key == "email") {
                StrRes vr = parse_json_string(input, i, limit);
                if (!vr.ok) return "";
                if (key == "name") {
                    if (nameSet) return "";
                    if (vr.value.empty()) return "";
                    nameVal = vr.value; nameSet = true;
                } else {
                    if (emailSet) return "";
                    if (!is_valid_email(vr.value)) return "";
                    emailVal = vr.value; emailSet = true;
                }
                i = vr.index;
            } else {
                return "";
            }

            i = skip_ws(input, i, limit);
            if (i < limit && input[i] == ',') {
                i++;
                continue;
            } else if (i == limit) {
                break;
            } else {
                return "";
            }
        }

        if (i != limit) return "";
        if (!(idSet && nameSet && emailSet)) return "";

        string out;
        out.reserve(64 + nameVal.size() + emailVal.size());
        out.push_back('{');
        out += "\"id\":" + to_string(idVal) + ",";
        out += "\"name\":" + json_escape(nameVal) + ",";
        out += "\"email\":" + json_escape(emailVal);
        if (ageSet) {
            out += ",\"age\":" + to_string(ageVal);
        }
        out.push_back('}');
        return out;
    }
};

int main() {
    vector<string> tests = {
        "{\"id\":123,\"name\":\"Alice\",\"email\":\"alice@example.com\",\"age\":30}",
        "{\"id\":1,\"name\":\"Bob\",\"email\":\"bob@example.com\"}",
        "{\"id\":2,\"name\":\"Mallory\",\"email\":\"m@ex.com\",\"role\":\"admin\"}",
        "{\"id\":3,\"name\":\"Eve\",\"email\":\"eve@example.com\",\"age\":\"30\"}",
        "{\"id\":4,\"name\":\"A \\\"quoted\\\" Name\",\"email\":\"a.q@example.com\",\"age\":45}"
    };
    for (auto& t : tests) {
        string res = Task131::secure_deserialize(t);
        if (res.empty()) cout << "INVALID" << "\n";
        else cout << res << "\n";
    }
    return 0;
}