#include <bits/stdc++.h>
using namespace std;

static string read_file(const string& path) {
    ifstream in(path, ios::in | ios::binary);
    if (!in) throw runtime_error("Cannot open file: " + path);
    ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void write_file(const string& path, const string& content) {
    ofstream out(path, ios::out | ios::binary);
    if (!out) throw runtime_error("Cannot write file: " + path);
    out << content;
}

static string trim(const string& s) {
    size_t a = 0, b = s.size();
    while (a < b && isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && isspace(static_cast<unsigned char>(s[b-1]))) --b;
    return s.substr(a, b-a);
}

static string unescape_xml(const string& s) {
    string r = s;
    auto replace_all = [&](const string& from, const string& to){
        size_t pos = 0;
        while ((pos = r.find(from, pos)) != string::npos) {
            r.replace(pos, from.size(), to);
            pos += to.size();
        }
    };
    replace_all("&quot;", "\"");
    replace_all("&apos;", "'");
    replace_all("&amp;", "&");
    replace_all("&lt;", "<");
    replace_all("&gt;", ">");
    return r;
}

static bool starts_with(const string& s, const string& p) {
    return s.size() >= p.size() && equal(p.begin(), p.end(), s.begin());
}

static vector<string> queryById(const string& xmlPath, const string& idValue) {
    string content = read_file(xmlPath);
    if (content.find("<!DOCTYPE") != string::npos) {
        throw runtime_error("DOCTYPE is disallowed for security reasons.");
    }
    // Remove BOM if present
    if (content.rfind("\xEF\xBB\xBF", 0) == 0) content = content.substr(3);

    size_t i = 0, n = content.size();
    // Skip XML declaration
    if (i + 5 <= n && (content.substr(i, 5) == "<?xml" || content.substr(i,5) == "<?XML")) {
        size_t end_decl = content.find("?>", i+5);
        if (end_decl != string::npos) i = end_decl + 2;
    }
    // Skip whitespace
    while (i < n && isspace(static_cast<unsigned char>(content[i]))) i++;

    if (i >= n || content[i] != '<') return {};
    size_t name_start = i + 1;
    size_t j = name_start;
    while (j < n && (isalnum(static_cast<unsigned char>(content[j])) || content[j]=='_' || content[j]==':' || content[j]=='-')) j++;
    string tagName = content.substr(name_start, j - name_start);
    if (tagName != "tag") return {};

    // Parse attributes until '>' considering quotes
    size_t k = j;
    string idAttr;
    bool selfClosing = false;
    while (k < n) {
        if (content[k] == '>') { k++; break; }
        if (content[k] == '/' && k+1 < n && content[k+1] == '>') { selfClosing = true; k += 2; break; }
        if (isspace(static_cast<unsigned char>(content[k]))) { k++; continue; }
        // read attribute name
        size_t an_start = k;
        while (k < n && (isalnum(static_cast<unsigned char>(content[k])) || content[k]=='_' || content[k]==':' || content[k]=='-')) k++;
        string attrName = content.substr(an_start, k - an_start);
        while (k < n && isspace(static_cast<unsigned char>(content[k]))) k++;
        if (k < n && content[k] == '=') k++; else continue;
        while (k < n && isspace(static_cast<unsigned char>(content[k]))) k++;
        if (k >= n || (content[k] != '"' && content[k] != '\'')) continue;
        char quote = content[k++];
        size_t vstart = k;
        while (k < n && content[k] != quote) k++;
        string attrVal = content.substr(vstart, k - vstart);
        if (k < n && content[k] == quote) k++;
        if (attrName == "id") {
            idAttr = unescape_xml(attrVal);
        }
    }

    vector<string> result;
    if (idAttr != idValue) return result;

    size_t start_elem = i;
    if (selfClosing) {
        result.push_back(content.substr(start_elem, k - start_elem));
        return result;
    }

    // find closing </tag>
    string closeTag = "</tag>";
    size_t end_close = content.find(closeTag, k);
    if (end_close == string::npos) return result;
    end_close += closeTag.size();
    result.push_back(content.substr(start_elem, end_close - start_elem));
    return result;
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc >= 3) {
        try {
            vector<string> res = queryById(argv[1], argv[2]);
            for (auto& s : res) cout << s << "\n";
        } catch (const exception& ex) {
            cerr << ex.what() << "\n";
            return 1;
        }
        return 0;
    }

    try {
        vector<string> files = {
            "sample97_1.xml",
            "sample97_2.xml",
            "sample97_3.xml",
            "sample97_4.xml",
            "sample97_5.xml"
        };
        vector<string> contents = {
            "<tag id=\"1\">Alpha</tag>",
            "<tag id=\"abc\">Bravo</tag>",
            "<tag id=\"he said &quot;hi&quot; and 'yo'\">Charlie</tag>",
            "<tag id=\"no-match\">Delta</tag>",
            "<tag id=\"5\"><child>Echo</child></tag>"
        };
        for (size_t i = 0; i < files.size(); ++i) write_file(files[i], contents[i]);

        auto printv = [](const vector<string>& v){ 
            cout << "[";
            for (size_t i=0;i<v.size();++i){ if(i) cout<<", "; cout<<v[i]; }
            cout << "]\n";
        };

        printv(queryById(files[0], "1"));
        printv(queryById(files[1], "abc"));
        printv(queryById(files[2], "he said \"hi\" and 'yo'"));
        printv(queryById(files[3], "x"));
        printv(queryById(files[4], "5"));
    } catch (const exception& ex) {
        cerr << ex.what() << "\n";
        return 1;
    }
    return 0;
}