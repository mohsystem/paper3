#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cctype>

// Chain-of-Through Step 1: Problem understanding
// Securely map user requests to in-memory resources with strict validation and authorization.

// Chain-of-Through Step 2: Security requirements
// - Token validation (chars and length).
// - Whitelist roles.
// - No external IO.
// - Deterministic outputs.

struct Resource {
    std::string id;
    std::string content;
    std::unordered_set<std::string> roles; // normalized (uppercase)
};

static const std::unordered_set<std::string> ROLES = {"ADMIN","USER","GUEST"};

static const std::unordered_map<std::string, Resource> RESOURCES = {
    {"doc_public", {"doc_public", "Public Document", {"ADMIN","USER","GUEST"}}},
    {"doc_user",   {"doc_user",   "User Document",   {"ADMIN","USER"}}},
    {"doc_admin",  {"doc_admin",  "Admin Secrets",   {"ADMIN"}}},
    {"img_banner", {"img_banner", "Image Banner",    {"ADMIN","USER","GUEST"}}},
    {"report_q3",  {"report_q3",  "Q3 Report",       {"ADMIN","USER"}}},
};

static bool validToken(const std::string& s) {
    if (s.empty() || s.size() > 32) return false;
    for (unsigned char c : s) {
        if (!(std::isalnum(c) || c=='_' || c=='-')) return false;
    }
    return true;
}

static std::string toUpper(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) out.push_back(std::toupper(c));
    return out;
}

// Chain-of-Through Step 3: Secure coding generation
std::string accessResource(const std::string& userId, const std::string& role, const std::string& resourceId) {
    if (!validToken(userId) || !validToken(resourceId)) return "ERROR:INVALID_INPUT";
    std::string normRole = toUpper(role);
    if (ROLES.find(normRole) == ROLES.end()) return "ERROR:INVALID_INPUT";

    auto it = RESOURCES.find(resourceId);
    if (it == RESOURCES.end()) return "ERROR:NOT_FOUND";

    const Resource& r = it->second;
    if (r.roles.find(normRole) == r.roles.end()) return "ERROR:ACCESS_DENIED";

    return "OK:" + r.id + ":" + r.content;
}

// Chain-of-Through Step 4: Code review
// - Validations are strict; role normalization consistent; no dynamic dangerous behavior.

// Chain-of-Through Step 5: Secure code output
int main() {
    // 5 test cases
    std::cout << accessResource("alice","user","doc_user") << "\n";
    std::cout << accessResource("bob","guest","doc_admin") << "\n";
    std::cout << accessResource("carol","ADMIN","report_q3") << "\n";
    std::cout << accessResource("","user","doc_public") << "\n";
    std::cout << accessResource("dave","user","unknown") << "\n";
    return 0;
}