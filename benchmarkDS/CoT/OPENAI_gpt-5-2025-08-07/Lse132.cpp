#include <bits/stdc++.h>
using namespace std;

static string template_path = "hello.html";

static void ensure_template_file() {
    ifstream in(template_path);
    if (!in.good()) {
        ofstream out(template_path);
        out << "<!doctype html>\n"
               "<html>\n"
               "<head><title>Hello</title></head>\n"
               "<body>\n"
               "<h1>Hello, {{ username }}!</h1>\n"
               "</body>\n"
               "</html>\n";
        out.close();
    }
}

static string read_file(const string& path) {
    ifstream in(path);
    if (!in) return "";
    stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void replace_all(string& s, const string& from, const string& to) {
    if (from.empty()) return;
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != string::npos) {
        s.replace(pos, from.length(), to);
        pos += to.length();
    }
}

string render_template(const string& path, const string& username) {
    string tpl = read_file(path);
    // Replace both "{{ username }}" and "{{username }}"
    replace_all(tpl, "{{ username }}", username);
    replace_all(tpl, "{{username}}", username);
    replace_all(tpl, "{{username }}", username);
    replace_all(tpl, "{{ username}}", username);
    return tpl;
}

// Handler corresponding to /hello/<username>
string hello(const string& username) {
    return render_template(template_path, username);
}

string handle_request(const string& url) {
    static const regex r("^/hello/([^/]+)$");
    smatch m;
    if (regex_match(url, m, r)) {
        return hello(m[1]);
    }
    return "404 Not Found";
}

int main() {
    ensure_template_file();
    vector<string> tests = {
        "/hello/alice",
        "/hello/Bob",
        "/hello/charlie_123",
        "/hello/Dana-XYZ",
        "/hello/Eve"
    };
    for (auto& url : tests) {
        cout << "URL: " << url << "\n";
        cout << handle_request(url) << "\n";
        cout << "-----\n";
    }
    return 0;
}