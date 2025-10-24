#include <bits/stdc++.h>
using namespace std;

struct ParseResult {
    bool ok;
    string rootName;
    vector<string> structureLines;
    string error;
};

static bool isNameStart(char c) {
    return (c == ':' || c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}
static bool isNameChar(char c) {
    return isNameStart(c) || c == '-' || c == '.' || (c >= '0' && c <= '9');
}

static bool startsWith(const string& s, size_t pos, const string& pfx) {
    return pos + pfx.size() <= s.size() && equal(pfx.begin(), pfx.end(), s.begin() + pos);
}

static void appendLine(vector<string>& lines, size_t depth, const string& name) {
    string line;
    line.reserve(depth * 2 + name.size());
    for (size_t i = 0; i < depth; ++i) line += "  ";
    line += name;
    lines.push_back(line);
}

static ParseResult parseXMLStructureInternal(const string& xmlInput) {
    ParseResult res{false, "", {}, ""};

    // Baseline input validation
    const size_t MAX_LEN = 5 * 1024 * 1024; // 5 MB cap
    if (xmlInput.size() == 0 || xmlInput.size() > MAX_LEN) {
        res.error = "Invalid input size.";
        return res;
    }

    // Work on a copy to allow safe indexing and possibly normalization checks
    const string& xml = xmlInput;
    size_t n = xml.size();
    size_t i = 0;

    // Skip UTF-8 BOM if present
    if (n >= 3 && (unsigned char)xml[0] == 0xEF && (unsigned char)xml[1] == 0xBB && (unsigned char)xml[2] == 0xBF) {
        i = 3;
    }

    auto skipWhitespace = [&](size_t& j) {
        while (j < n) {
            char c = xml[j];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') j++;
            else break;
        }
    };

    auto skipComment = [&](size_t& j) -> bool {
        // assumes xml[j] starts with <!--
        j += 4;
        while (j + 2 < n) {
            if (xml[j] == '-' && xml[j + 1] == '-' && xml[j + 2] == '>') {
                j += 3;
                return true;
            }
            j++;
        }
        return false;
    };

    auto skipPI = [&](size_t& j) -> bool {
        // assumes xml[j] starts with <?
        j += 2;
        while (j + 1 < n) {
            if (xml[j] == '?' && xml[j + 1] == '>') {
                j += 2;
                return true;
            }
            j++;
        }
        return false;
    };

    auto skipCDATA = [&](size_t& j) -> bool {
        // assumes xml[j] starts with <![CDATA[
        j += 9;
        while (j + 2 < n) {
            if (xml[j] == ']' && xml[j + 1] == ']' && xml[j + 2] == '>') {
                j += 3;
                return true;
            }
            j++;
        }
        return false;
    };

    auto skipDoctype = [&](size_t& j) -> bool {
        // assumes xml[j] starts with <!DOCTYPE
        j += 9;
        int bracketDepth = 0;
        bool inQuotes = false;
        char quoteChar = 0;
        while (j < n) {
            char c = xml[j];
            if (inQuotes) {
                if (c == quoteChar) {
                    inQuotes = false;
                }
                j++;
                continue;
            }
            if (c == '"' || c == '\'') {
                inQuotes = true;
                quoteChar = c;
                j++;
                continue;
            }
            if (c == '[') {
                bracketDepth++;
                j++;
                continue;
            }
            if (c == ']') {
                if (bracketDepth > 0) bracketDepth--;
                j++;
                continue;
            }
            if (c == '>' && bracketDepth == 0) {
                j++;
                return true;
            }
            j++;
        }
        return false;
    };

    auto parseName = [&](size_t& j) -> string {
        size_t start = j;
        if (j < n && isNameStart(xml[j])) {
            j++;
            while (j < n && isNameChar(xml[j])) j++;
            return xml.substr(start, j - start);
        }
        return "";
    };

    vector<string> stack;
    const size_t MAX_DEPTH = 4096;
    string root;
    vector<string> lines;

    while (i < n) {
        size_t lt = xml.find('<', i);
        if (lt == string::npos) break;
        i = lt;

        if (i + 4 <= n && startsWith(xml, i, "<!--")) {
            if (!skipComment(i)) { res.error = "Unterminated comment."; return res; }
            continue;
        }
        if (i + 2 <= n && startsWith(xml, i, "<?")) {
            if (!skipPI(i)) { res.error = "Unterminated processing instruction."; return res; }
            continue;
        }
        if (i + 9 <= n && startsWith(xml, i, "<![CDATA[")) {
            if (!skipCDATA(i)) { res.error = "Unterminated CDATA section."; return res; }
            continue;
        }
        if (i + 9 <= n && startsWith(xml, i, "<!DOCTYPE")) {
            if (!skipDoctype(i)) { res.error = "Unterminated DOCTYPE."; return res; }
            continue;
        }

        if (i + 2 <= n && xml[i + 1] == '/') {
            // closing tag
            i += 2;
            skipWhitespace(i);
            string name = parseName(i);
            if (name.empty()) { res.error = "Invalid closing tag name."; return res; }
            skipWhitespace(i);
            if (i >= n || xml[i] != '>') { res.error = "Malformed closing tag."; return res; }
            i++; // consume '>'

            if (stack.empty() || stack.back() != name) {
                res.error = "Mismatched closing tag: </" + name + ">.";
                return res;
            }
            stack.pop_back();
            continue;
        }

        if (i + 1 < n && xml[i + 1] == '!') {
            // Some other declaration not handled specifically
            res.error = "Unsupported declaration.";
            return res;
        }

        // start or empty-element tag
        if (i + 1 >= n || xml[i + 1] == '>' || xml[i + 1] == '/') {
            res.error = "Malformed tag start.";
            return res;
        }

        i++; // consume '<'
        skipWhitespace(i);
        string tagName = parseName(i);
        if (tagName.empty()) { res.error = "Invalid start tag name."; return res; }

        // First start tag defines root if not yet set
        if (stack.empty() && root.empty()) {
            root = tagName;
        }

        // parse attributes and determine whether self-closing
        bool selfClose = false;
        bool inQuotes = false;
        char quoteChar = 0;
        while (i < n) {
            char c = xml[i];
            if (inQuotes) {
                if (c == quoteChar) {
                    inQuotes = false;
                }
                i++;
                continue;
            }
            if (c == '"' || c == '\'') {
                inQuotes = true;
                quoteChar = c;
                i++;
                continue;
            }
            if (c == '>') {
                i++;
                break;
            }
            if (c == '/' && (i + 1 < n) && xml[i + 1] == '>') {
                selfClose = true;
                i += 2;
                break;
            }
            i++;
        }
        if (i > n) { res.error = "Unterminated start tag."; return res; }

        // Record structure
        appendLine(lines, stack.size(), tagName);

        if (!selfClose) {
            if (stack.size() + 1 > MAX_DEPTH) {
                res.error = "Exceeded maximum XML depth.";
                return res;
            }
            stack.push_back(tagName);
        }
    }

    if (!stack.empty()) {
        res.error = "Unclosed tags remain.";
        return res;
    }
    if (root.empty()) {
        res.error = "No root element found.";
        return res;
    }

    res.ok = true;
    res.rootName = root;
    res.structureLines = std::move(lines);
    return res;
}

ParseResult parseXMLStructure(const string& xml) {
    return parseXMLStructureInternal(xml);
}

// Returns the root element name if well-formed; empty string otherwise.
string findRootElement(const string& xml) {
    auto r = parseXMLStructure(xml);
    return r.ok ? r.rootName : string();
}

int main() {
    vector<string> tests = {
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?><root><child/><child2><x/></child2></root>",
        "  <!-- leading comment -->\n<a id=\"1\"><b></b><c/></a>",
        "<!DOCTYPE note [ <!ELEMENT note (to,from)> ]><note><![CDATA[<fake>]]><to>T</to><from>F</from></note>",
        "<?xml version='1.0'?><?pi test?><x y='1'><y1><z/></y1></x>",
        "<r><a></r>" // malformed
    };

    for (size_t idx = 0; idx < tests.size(); ++idx) {
        cout << "Test " << (idx + 1) << ":\n";
        auto result = parseXMLStructure(tests[idx]);
        if (result.ok) {
            cout << "Root: " << result.rootName << "\n";
            cout << "Structure:\n";
            for (const auto& line : result.structureLines) {
                cout << line << "\n";
            }
        } else {
            cout << "Error: " << result.error << "\n";
        }
        cout << "----\n";
    }
    return 0;
}