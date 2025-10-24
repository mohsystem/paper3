#include <bits/stdc++.h>
using namespace std;

static string read_file(const string& path) {
    ifstream in(path, ios::in | ios::binary);
    if (!in) return "";
    ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static bool is_alnum_char(char c) {
    return std::isalnum(static_cast<unsigned char>(c));
}

static int index_of_word(const string& s, const string& w, int start = 0) {
    size_t pos = start;
    while (true) {
        pos = s.find(w, pos);
        if (pos == string::npos) return -1;
        bool leftOk = (pos == 0) || !is_alnum_char(s[pos - 1]);
        size_t end = pos + w.size();
        bool rightOk = (end >= s.size()) || !is_alnum_char(s[end]);
        if (leftOk && rightOk) return static_cast<int>(pos);
        pos = pos + 1;
    }
}

static string extract_attr_value(const string& attrs, const string& name) {
    int idx = index_of_word(attrs, name, 0);
    while (idx != -1) {
        int j = idx + (int)name.size();
        while (j < (int)attrs.size() && isspace((unsigned char)attrs[j])) j++;
        if (j < (int)attrs.size() && attrs[j] == '=') {
            j++;
            while (j < (int)attrs.size() && isspace((unsigned char)attrs[j])) j++;
            if (j < (int)attrs.size()) {
                char q = attrs[j];
                if (q == '"' || q == '\'') {
                    int k = attrs.find(q, j + 1);
                    if (k != -1) return attrs.substr(j + 1, k - (j + 1));
                } else {
                    int k = j;
                    while (k < (int)attrs.size() && !isspace((unsigned char)attrs[k])) k++;
                    return attrs.substr(j, k - j);
                }
            }
        }
        idx = index_of_word(attrs, name, idx + 1);
    }
    return "";
}

static int find_matching_end(const string& s, int from) {
    int depth = 1;
    int search = from;
    while (depth > 0) {
        int nextOpen = s.find("<tag", search);
        int nextClose = s.find("</tag>", search);
        if (nextClose == -1) return -1;
        if (nextOpen != -1 && nextOpen < nextClose) {
            int gt2 = s.find(">", nextOpen);
            if (gt2 == -1) return -1;
            bool selfClosing = gt2 > nextOpen && s[gt2 - 1] == '/';
            if (!selfClosing) depth++;
            search = gt2 + 1;
        } else {
            depth--;
            search = nextClose + 6;
        }
    }
    return search;
}

string executeXPathLike(const string& xmlFilePath, const string& idValue) {
    string content = read_file(xmlFilePath);
    if (content.empty()) return "";
    string result;
    size_t pos = 0;
    while (true) {
        size_t start = content.find("<tag", pos);
        if (start == string::npos) break;
        size_t gt = content.find(">", start);
        if (gt == string::npos) break;
        bool selfClosing = gt > start && content[gt - 1] == '/';
        string attrs = content.substr(start + 4, gt - (start + 4));
        string attrVal = extract_attr_value(attrs, "id");
        if (!attrVal.empty() && attrVal == idValue) {
            if (!result.empty()) result.push_back('\n');
            if (selfClosing) {
                result += content.substr(start, gt - start + 1);
            } else {
                int endIdx = find_matching_end(content, (int)gt + 1);
                if (endIdx != -1) {
                    result += content.substr(start, endIdx - (int)start);
                }
            }
        }
        pos = start + 1;
    }
    return result;
}

static void write_sample_file(const string& path) {
    string xml;
    xml += "<root>\n";
    xml += "  <tag id=\"1\">Content one</tag>\n";
    xml += "  <tag id=\"2\">\n";
    xml += "     Two\n";
    xml += "     <child>Alpha</child>\n";
    xml += "  </tag>\n";
    xml += "  <tag id=\"3\"/>\n";
    xml += "  <other id=\"1\">Not a tag</other>\n";
    xml += "  <tag id=\"nested\">\n";
    xml += "     <tag id=\"inner\">Inner content</tag>\n";
    xml += "  </tag>\n";
    xml += "  <tag id=\"spaces\">  spaced  </tag>\n";
    xml += "</root>\n";
    ofstream out(path);
    out << xml;
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        string idVal = argv[1];
        string xmlPath = argv[2];
        string xpath = "/tag[@id='" + idVal + "']";
        string out = executeXPathLike(xmlPath, idVal);
        cout << out << endl;
        return 0;
    }
    string samplePath = "sample_task97.xml";
    write_sample_file(samplePath);
    vector<string> tests = {"1", "2", "3", "missing", "nested"};
    for (auto& id : tests) {
        string xpath = "/tag[@id='" + id + "']";
        string res = executeXPathLike(samplePath, id);
        cout << "XPath: " << xpath << "\n";
        cout << res << "\n";
        cout << "-----\n";
    }
    return 0;
}