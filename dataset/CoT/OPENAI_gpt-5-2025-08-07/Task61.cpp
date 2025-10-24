#include <bits/stdc++.h>
using namespace std;

struct Node {
    string name;
    vector<Node*> children;
    explicit Node(const string& n) : name(n) {}
};

static inline bool isNameChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == ':' || c == '.';
}

static inline size_t skipSpaces(const string& s, size_t i) {
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r')) i++;
    return i;
}

static inline bool startsWith(const string& s, size_t i, const string& p) {
    return i + p.size() <= s.size() && s.compare(i, p.size(), p) == 0;
}

static inline long indexOfSeq2(const string& s, size_t from, char x, char y) {
    for (size_t i = from; i + 1 < s.size(); ++i) {
        if (s[i] == x && s[i + 1] == y) return static_cast<long>(i + 1);
    }
    return -1;
}

static inline long indexOfSeq3(const string& s, size_t from, char x, char y, char z) {
    for (size_t i = from; i + 2 < s.size(); ++i) {
        if (s[i] == x && s[i + 1] == y && s[i + 2] == z) return static_cast<long>(i + 2);
    }
    return -1;
}

static inline long skipDeclaration(const string& s, size_t from) {
    int depth = 0;
    for (size_t i = from; i < s.size(); ++i) {
        char c = s[i];
        if (c == '[') depth++;
        else if (c == ']') { if (depth > 0) depth--; }
        else if (c == '>' && depth == 0) return static_cast<long>(i);
    }
    return -1;
}

static inline long skipToTagEnd(const string& s, size_t from) {
    char quote = 0;
    for (size_t i = from; i < s.size(); ++i) {
        char c = s[i];
        if (quote) {
            if (c == quote) quote = 0;
        } else {
            if (c == '\'' || c == '"') quote = c;
            else if (c == '>') return static_cast<long>(i);
        }
    }
    return -1;
}

string getRootElementName(const string& xml) {
    const string& a = xml;
    size_t n = a.size();
    size_t i = 0;
    string rootName;
    vector<Node*> stack;

    while (i < n) {
        size_t lt = a.find('<', i);
        if (lt == string::npos) break;
        i = lt;
        if (i + 1 >= n) break;

        if (startsWith(a, i, "<?")) {
            long j = indexOfSeq2(a, i + 2, '?', '>');
            if (j < 0) break;
            i = static_cast<size_t>(j) + 1;
            continue;
        }

        if (startsWith(a, i, "<!--")) {
            long j = indexOfSeq3(a, i + 4, '-', '-', '>');
            if (j < 0) break;
            i = static_cast<size_t>(j) + 1;
            continue;
        }

        if (startsWith(a, i, "<![CDATA[")) {
            long j = indexOfSeq3(a, i + 9, ']', ']', '>');
            if (j < 0) break;
            i = static_cast<size_t>(j) + 1;
            continue;
        }

        if (startsWith(a, i, "<!")) {
            long j = skipDeclaration(a, i + 2);
            if (j < 0) break;
            i = static_cast<size_t>(j) + 1;
            continue;
        }

        if (a[i + 1] == '/') {
            size_t j = i + 2;
            j = skipSpaces(a, j);
            size_t start = j;
            while (j < n && isNameChar(a[j])) j++;
            (void)start;
            long end = skipToTagEnd(a, j);
            if (end < 0) break;
            if (!stack.empty()) {
                stack.pop_back();
            }
            i = static_cast<size_t>(end) + 1;
            continue;
        }

        size_t j = i + 1;
        j = skipSpaces(a, j);
        size_t start = j;
        while (j < n && isNameChar(a[j])) j++;
        if (j == start) {
            i++;
            continue;
        }
        string name = a.substr(start, j - start);
        Node* node = new Node(name);
        if (stack.empty()) {
            rootName = name;
        } else {
            stack.back()->children.push_back(node);
        }

        bool selfClose = false;
        size_t k = j;
        char quote = 0;
        while (k < n) {
            char c = a[k];
            if (quote) {
                if (c == quote) quote = 0;
                k++;
                continue;
            } else {
                if (c == '\'' || c == '"') {
                    quote = c;
                    k++;
                    continue;
                }
                if (c == '/') {
                    if (k + 1 < n && a[k + 1] == '>') {
                        selfClose = true;
                        k += 2;
                        break;
                    }
                }
                if (c == '>') {
                    k++;
                    break;
                }
                k++;
            }
        }
        if (!selfClose) {
            stack.push_back(node);
        } else {
            // self-closed, no push
        }
        i = k;
    }
    // cleanup allocated nodes (prevent leaks)
    // Note: In this simplified parser we don't retain references; skipping deep cleanup for brevity.
    return rootName;
}

int main() {
    vector<string> tests = {
        "<root><child/></root>",
        "<?xml version='1.0'?><!-- comment --><a id='1'><b></b><c/></a>",
        "  \n <root>text<inner>more</inner></root> ",
        "<x><y></x>",
        "<!DOCTYPE note SYSTEM 'Note.dtd'><note><![CDATA[<notatag>]]><to>T</to></note>"
    };
    for (auto& t : tests) {
        cout << getRootElementName(t) << "\n";
    }
    return 0;
}