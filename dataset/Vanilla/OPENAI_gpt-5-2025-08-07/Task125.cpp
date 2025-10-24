#include <bits/stdc++.h>
using namespace std;

static unordered_map<string, int> ROLE_LEVEL = {
    {"guest", 0},
    {"user", 1},
    {"editor", 2},
    {"admin", 3}
};

static unordered_map<string, int> RESOURCE_REQUIRED_LEVEL = {
    {"public:news", 0},
    {"content:view", 1},
    {"user:profile", 1},
    {"content:edit", 2},
    {"admin:dashboard", 3},
    {"reports:financial", 3}
};

static unordered_map<string, string> RESOURCE_CONTENT = {
    {"public:news", "Top headlines for everyone"},
    {"content:view", "Catalog of articles"},
    {"user:profile", "Personal profile details"},
    {"content:edit", "Editor panel for articles"},
    {"admin:dashboard", "System metrics and controls"},
    {"reports:financial", "Quarterly financial report"}
};

string accessResource(const string& user, const string& role, const string& resourceId) {
    auto rlIt = ROLE_LEVEL.find(string(role.begin(), role.end()));
    string roleLower = role;
    transform(roleLower.begin(), roleLower.end(), roleLower.begin(), ::tolower);
    rlIt = ROLE_LEVEL.find(roleLower);
    if (rlIt == ROLE_LEVEL.end()) {
        return "ACCESS DENIED: Unknown role";
    }
    auto reqIt = RESOURCE_REQUIRED_LEVEL.find(resourceId);
    if (reqIt == RESOURCE_REQUIRED_LEVEL.end()) {
        return "ACCESS DENIED: Unknown resource";
    }
    int userLevel = rlIt->second;
    int requiredLevel = reqIt->second;
    if (userLevel >= requiredLevel) {
        string content = "(no content)";
        auto cIt = RESOURCE_CONTENT.find(resourceId);
        if (cIt != RESOURCE_CONTENT.end()) content = cIt->second;
        return "ACCESS GRANTED: " + resourceId + " -> " + content;
    }
    return "ACCESS DENIED: Insufficient permissions";
}

int main() {
    vector<tuple<string,string,string>> tests = {
        {"alice","guest","public:news"},
        {"bob","user","content:view"},
        {"cindy","user","admin:dashboard"},
        {"dave","editor","content:edit"},
        {"erica","guest","user:profile"}
    };
    for (auto &t : tests) {
        cout << accessResource(get<0>(t), get<1>(t), get<2>(t)) << "\n";
    }
    return 0;
}