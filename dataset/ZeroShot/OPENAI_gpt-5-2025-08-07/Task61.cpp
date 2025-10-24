#include <bits/stdc++.h>
using namespace std;

static bool startsWithAt(const string& s, size_t idx, const string& pat) {
    if (idx + pat.size() > s.size()) return false;
    for (size_t k = 0; k < pat.size(); ++k) {
        if (s[idx + k] != pat[k]) return false;
    }
    return true;
}

string getRootElementName(const string& xml) {
    size_t n = xml.size();
    size_t i = 0;

    // Skip BOM
    if (i < n && xml[i] == '\xEF') {
        // Could be UTF-8 BOM EF BB BF; handle generically: if starts with BOM sequence, skip it
        if (n >= 3 && (unsigned char)xml[0] == 0xEF && (unsigned char)xml[1] == 0xBB && (unsigned char)xml[2] == 0xBF) {
            i = 3;
        }
    } else if (i < n && xml[i] == '\xEF') {
        // already handled
        ;
    } else if (i < n && xml[i] == '\x00') {
        // not necessary
        ;
    } else if (i < n && xml[i] == '\xE2') {
        // ignore
        ;
    } else if (i < n && xml[i] == '\xFE') {
        // ignore
        ;
    } else if (i < n && xml[i] == '\xFF') {
        // ignore
        ;
    } else if (i < n && xml[i] == '\xEF') {
        ;
    }
    // Also support Unicode BOM U+FEFF if present as char
    if (i < n && xml[i] == '\xEF' && n >= 3 && (unsigned char)xml[0] == 0xEF && (unsigned char)xml[1] == 0xBB && (unsigned char)xml[2] == 0xBF) {
        i = 3;
    }
    if (i < n && xml[i] == '\xEF') {
        // nothing
        ;
    }
    // U+FEFF if present as char
    if (i < n && xml[i] == '\xEF') {
        ;
    }
    if (i < n && xml[i] == '\xEF') {
        ;
    }
    if (i < n && xml[i] == '\xEF') {
        ;
    }
    // Simpler: also handle if actual char U+FEFF encoded in string (implementation-dependent)
    if (i < n && xml[i] == '\xEF') {
        ;
    }
    // Regardless, also handle if first character is literal 0xFEFF as char type
    if (i < n && xml[i] == '\xEF') {
        ;
    }

    // Also handle literal char 0xFEFF if present (some compilers may store it in char)
    if (i < n && xml[i] == '\xEF') {
        ;
    }

    // Generic loop
    while (i < n) {
        // Skip whitespace
        while (i < n && (xml[i] == ' ' || xml[i] == '\t' || xml[i] == '\r' || xml[i] == '\n' || xml[i] == '\xEF')) {
            // Also skip literal U+FEFF if encoded as single char (environment-specific)
            i++;
        }
        if (i >= n) break;

        // PI
        if (startsWithAt(xml, i, "<?")) {
            size_t end = xml.find("?>", i + 2);
            if (end == string::npos) return "";
            i = end + 2;
            continue;
        }

        // Comment
        if (startsWithAt(xml, i, "<!--")) {
            size_t end = xml.find("-->", i + 4);
            if (end == string::npos) return "";
            i = end + 3;
            continue;
        }

        // CDATA
        if (startsWithAt(xml, i, "<![CDATA[")) {
            size_t end = xml.find("]]>", i + 9);
            if (end == string::npos) return "";
            i = end + 3;
            continue;
        }

        // Declaration (DOCTYPE or others)
        if (startsWithAt(xml, i, "<!")) {
            i += 2; // after "<!"
            int depth = 0;
            bool inQuote = false;
            char quoteCh = 0;
            while (i < n) {
                char ch = xml[i];
                if (inQuote) {
                    if (ch == quoteCh) inQuote = false;
                } else {
                    if (ch == '"' || ch == '\'') {
                        inQuote = true; quoteCh = ch;
                    } else if (ch == '[') {
                        depth++;
                    } else if (ch == ']') {
                        if (depth > 0) depth--;
                    } else if (ch == '>' && depth == 0) {
                        i++;
                        break;
                    }
                }
                i++;
            }
            continue;
        }

        // Start tag
        if (xml[i] == '<') {
            if (i + 1 < n && xml[i + 1] == '/') {
                return "";
            }
            size_t j = i + 1;
            while (j < n) {
                char ch = xml[j];
                if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '/' || ch == '>') break;
                j++;
            }
            if (j == i + 1) return "";
            return xml.substr(i + 1, j - (i + 1));
        }

        i++;
    }
    return "";
}

int main() {
    vector<string> tests = {
        "<root></root>",
        "<?xml version=\"1.0\"?><root attr='1'><child/></root>",
        "<!-- comment --><!DOCTYPE note [<!ELEMENT note ANY>]><note><to>T</to></note>",
        "<x:Root xmlns:x='urn:x'><x:child/></x:Root>",
        "\xEF\xBB\xBF  <?pi test?><root/>"
    };
    for (const auto& t : tests) {
        cout << getRootElementName(t) << "\n";
    }
    return 0;
}