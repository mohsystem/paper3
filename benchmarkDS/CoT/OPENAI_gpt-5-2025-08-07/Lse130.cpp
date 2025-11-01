#include <bits/stdc++.h>
using namespace std;

struct Request {
    unordered_map<string, string> params;
    string getParam(const string& key) const {
        auto it = params.find(key);
        return it == params.end() ? "" : it->second;
    }
};

struct Env {
    Request request;
    string templ;
};

struct Response {
    string body;
};

static string replaceAll(string s, const string& from, const string& to) {
    if (from.empty()) return s;
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}

static string renderTemplate(const string& templ, const string& name) {
    string safeName = name;
    string out = replaceAll(templ, "{{name}}", safeName);
    out = replaceAll(out, "{name}", safeName);
    return out;
}

Response render_response_from_env(const Env& env) {
    string name = env.request.getParam("name");
    string templ = env.templ;
    string rendered = renderTemplate(templ, name);
    return Response{rendered};
}

Response home(const Env& env) {
    return render_response_from_env(env);
}

int main() {
    vector<Env> tests;
    tests.push_back(Env{Request{{{"name","Alice"}}}, "Hello, {{name}}!"});
    tests.push_back(Env{Request{{{"name","Bob"}}}, "Welcome, {name}."});
    tests.push_back(Env{Request{{{"name","Charlie"}}}, "Hi {name}, this is a test."});
    tests.push_back(Env{Request{{{"name","Dana"}}}, "Dear {name}, your order is ready."});
    tests.push_back(Env{Request{{{"name","Eve"}}}, "Goodbye, {{name}}."});

    for (size_t i = 0; i < tests.size(); ++i) {
        Response r = home(tests[i]);
        cout << "Test " << (i+1) << ": " << r.body << "\n";
    }
    return 0;
}