#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cctype>

struct Resource {
    std::string content;
    std::unordered_set<std::string> allowedRoles;
};

static const std::string GENERIC_ERROR = "ERROR: Access denied or resource not found.";

static const std::unordered_set<std::string> ALLOWED_ROLES = {
    "guest", "user", "admin"
};

static const std::unordered_map<std::string, Resource> RESOURCES = {
    {"public", {"Welcome to the public area.", {"guest", "user", "admin"}}},
    {"dashboard", {"Admin control panel.", {"admin"}}},
    {"profile", {"User profile page.", {"user", "admin"}}},
    {"report2025", {"Confidential report 2025.", {"user", "admin"}}},
    {"help", {"Help and FAQs.", {"guest", "user", "admin"}}}
};

bool isValidRole(const std::string& role) {
    return ALLOWED_ROLES.find(role) != ALLOWED_ROLES.end();
}

bool isValidResourceId(const std::string& id) {
    if (id.empty() || id.size() > 20) return false;
    for (char c : id) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

std::string accessResource(const std::string& role, const std::string& resourceId) {
    if (!isValidRole(role) || !isValidResourceId(resourceId)) {
        return GENERIC_ERROR;
    }
    auto it = RESOURCES.find(resourceId);
    if (it == RESOURCES.end()) return GENERIC_ERROR;
    const Resource& r = it->second;
    if (r.allowedRoles.find(role) == r.allowedRoles.end()) {
        return GENERIC_ERROR;
    }
    return r.content;
}

int main() {
    const std::pair<std::string, std::string> tests[5] = {
        {"admin", "dashboard"},
        {"guest", "public"},
        {"user", "report2025"},
        {"guest", "report2025"},
        {"user", "bad$id"}
    };

    for (int i = 0; i < 5; ++i) {
        std::string result = accessResource(tests[i].first, tests[i].second);
        std::cout << "Test " << (i + 1) << " (" << tests[i].first << ", " << tests[i].second << "): " << result << "\n";
    }
    return 0;
}