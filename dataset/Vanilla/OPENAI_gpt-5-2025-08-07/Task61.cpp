#include <bits/stdc++.h>
using namespace std;

static bool isNameChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c=='_' || c=='-' || c=='.' || c==':';
}

string getRootElement(const string& xml) {
    if (xml.empty()) return "";
    size_t n = xml.size();
    size_t i = 0;
    while (i < n) {
        size_t lt = xml.find('<', i);
        if (lt == string::npos) return "";
        i = lt + 1;
        if (i >= n) return "";
        if (xml[i] == '?') {
            size_t end = xml.find("?>", i+1);
            if (end == string::npos) return "";
            i = end + 2;
            continue;
        }
        if (xml[i] == '!') {
            // Comment
            if (i + 2 < n && xml.compare(i, 2, "--") == 0) {
                size_t end = xml.find("-->", i + 2);
                if (end == string::npos) return "";
                i = end + 3;
                continue;
            }
            // CDATA
            if (i + 7 < n && xml.compare(i, 7, "[CDATA[") == 0) {
                size_t end = xml.find("]]>", i + 7);
                if (end == string::npos) return "";
                i = end + 3;
                continue;
            }
            // DOCTYPE or other declaration
            if (i + 7 <= n && xml.compare(i, 7, "DOCTYPE") == 0) {
                size_t j = i + 7;
                int bracketDepth = 0;
                while (j < n) {
                    if (xml[j] == '[') { bracketDepth++; j++; continue; }
                    if (xml[j] == ']') { if (bracketDepth > 0) bracketDepth--; j++; continue; }
                    if (xml[j] == '>' && bracketDepth == 0) { j++; break; }
                    j++;
                }
                i = j;
                continue;
            } else {
                // Generic declaration, skip until next '>'
                size_t end = xml.find('>', i + 1);
                if (end == string::npos) return "";
                i = end + 1;
                continue;
            }
        }
        if (xml[i] == '/') {
            // Closing tag before root not expected; skip
            size_t end = xml.find('>', i + 1);
            if (end == string::npos) return "";
            i = end + 1;
            continue;
        }
        // Start tag for root
        size_t j = i;
        if (j >= n || !isNameChar(xml[j])) return "";
        while (j < n && isNameChar(xml[j])) j++;
        return xml.substr(i, j - i);
    }
    return "";
}

int main() {
    vector<string> tests = {
        "<root></root>",
        "<?xml version=\"1.0\"?><a><b/></a>",
        "  \n<!-- comment -->\n<ns:catalog attr='1'><item/></ns:catalog>",
        "<!DOCTYPE note [<!ELEMENT note (to)>]><note><to>Tove</to></note>",
        "<unclosed"
    };
    for (size_t k = 0; k < tests.size(); ++k) {
        string res = getRootElement(tests[k]);
        cout << "Test " << (k+1) << ": " << (res.empty() ? "null" : res) << "\n";
    }
    return 0;
}