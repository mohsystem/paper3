#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

struct Entry {
    std::string dn;
    std::string uid;
    std::string cn;
    std::string mail;
};

enum class Status {
    Ok = 0,
    InvalidInput = 1,
    NotFound = 2
};

struct Result {
    Status status;
    std::string info;   // User info on success
    std::string error;  // Error message on failure
};

static bool isLabelValid(const std::string& s) {
    if (s.empty() || s.size() > 63) return false;
    if (s.front() == '-' || s.back() == '-') return false;
    for (char c : s) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '-')) {
            return false;
        }
    }
    return true;
}

static std::string toLower(const std::string& s) {
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return out;
}

static bool parseDcToBaseDn(const std::string& input, std::string& outBaseDn) {
    outBaseDn.clear();
    if (input.empty() || input.size() > 253) return false;

    auto startsWithDC = [](const std::string& part) -> bool {
        if (part.size() < 3) return false;
        return (part[0] == 'd' || part[0] == 'D') &&
               (part[1] == 'c' || part[1] == 'C') &&
               part[2] == '=';
    };

    std::vector<std::string> labels;

    if (startsWithDC(input)) {
        // Expect format: dc=example,dc=com[,dc=sub]...
        size_t pos = 0;
        while (pos < input.size()) {
            size_t comma = input.find(',', pos);
            std::string part = input.substr(pos, (comma == std::string::npos) ? std::string::npos : (comma - pos));
            if (!startsWithDC(part)) return false;
            std::string val = part.substr(3);
            if (!isLabelValid(val)) return false;
            labels.push_back(val);
            if (comma == std::string::npos) break;
            pos = comma + 1;
        }
    } else {
        // Domain form: example.com or sub.example.com
        size_t pos = 0;
        while (pos <= input.size()) {
            size_t dot = input.find('.', pos);
            std::string label = input.substr(pos, (dot == std::string::npos) ? std::string::npos : (dot - pos));
            if (!isLabelValid(label)) return false;
            labels.push_back(label);
            if (dot == std::string::npos) break;
            pos = dot + 1;
        }
    }

    // Build base DN
    std::string base;
    for (size_t i = 0; i < labels.size(); ++i) {
        if (i) base += ",";
        base += "dc=" + labels[i];
    }
    if (base.size() > 512) return false;
    outBaseDn = base;
    return true;
}

static bool validateUsername(const std::string& username) {
    if (username.empty() || username.size() > 64) return false;
    for (char c : username) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.')) {
            return false;
        }
    }
    return true;
}

// RFC 4515 escaping for LDAP filter values
static std::string escapeLdapFilterValue(const std::string& val) {
    std::string out;
    out.reserve(val.size());
    auto hex = [](unsigned char c) -> std::string {
        const char* digits = "0123456789ABCDEF";
        std::string s;
        s.push_back('\\');
        s.push_back(digits[(c >> 4) & 0xF]);
        s.push_back(digits[c & 0xF]);
        return s;
    };
    for (unsigned char c : val) {
        switch (c) {
            case '*': out += "\\2A"; break;
            case '(': out += "\\28"; break;
            case ')': out += "\\29"; break;
            case '\\': out += "\\5C"; break;
            case '\0': out += "\\00"; break;
            default: out.push_back(static_cast<char>(c)); break;
        }
    }
    return out;
}

static std::string buildFilter(const std::string& username) {
    return "(uid=" + escapeLdapFilterValue(username) + ")";
}

static std::vector<Entry> makeDirectory() {
    std::vector<Entry> dir;
    dir.push_back({"uid=alice,ou=People,dc=example,dc=com", "alice", "Alice Example", "alice@example.com"});
    dir.push_back({"uid=bob,ou=People,dc=example,dc=com", "bob", "Bob Builder", "bob@example.com"});
    dir.push_back({"uid=john.doe,ou=People,dc=example,dc=com", "john.doe", "John Doe", "john.doe@example.com"});
    dir.push_back({"uid=admin,ou=Admins,dc=example,dc=com", "admin", "Administrator", "admin@example.com"});
    dir.push_back({"uid=carol,ou=Users,dc=test,dc=org", "carol", "Carol Test", "carol@test.org"});
    return dir;
}

static bool endsWithCaseInsensitive(const std::string& s, const std::string& suffix) {
    if (s.size() < suffix.size()) return false;
    std::string tail = toLower(s.substr(s.size() - suffix.size()));
    return tail == toLower(suffix);
}

static bool parseUidEqualsFilter(const std::string& filter, std::string& outValue) {
    outValue.clear();
    if (filter.size() < 7) return false; // "(uid=x)"
    if (filter.front() != '(' || filter.back() != ')') return false;
    std::string inner = filter.substr(1, filter.size() - 2);
    const std::string prefix = "uid=";
    if (inner.compare(0, prefix.size(), prefix) != 0) return false;
    outValue = inner.substr(prefix.size());
    return !outValue.empty();
}

static Result searchLdapLocalhost(const std::string& baseDn, const std::string& filter) {
    std::string uidValue;
    if (!parseUidEqualsFilter(filter, uidValue)) {
        return {Status::InvalidInput, "", "Unsupported or invalid LDAP filter format"};
    }

    const auto dir = makeDirectory();
    for (const auto& e : dir) {
        if (endsWithCaseInsensitive(e.dn, baseDn) && e.uid == uidValue) {
            std::string info;
            info += "dn: " + e.dn + "\n";
            info += "cn: " + e.cn + "\n";
            info += "mail: " + e.mail + "\n";
            info += "uid: " + e.uid + "\n";
            return {Status::Ok, info, ""};
        }
    }
    return {Status::NotFound, "", "User not found"};
}

static Result processRequest(const std::string& dcInput, const std::string& usernameInput) {
    // Validate inputs
    std::string baseDn;
    if (!parseDcToBaseDn(dcInput, baseDn)) {
        return {Status::InvalidInput, "", "Invalid domain component (dc) format"};
    }
    if (!validateUsername(usernameInput)) {
        return {Status::InvalidInput, "", "Invalid username format"};
    }
    // Build filter
    const std::string filter = buildFilter(usernameInput);
    // Simulated LDAP search on localhost (no network I/O)
    return searchLdapLocalhost(baseDn, filter);
}

int main(int argc, char* argv[]) {
    // If command-line args provided, run once with those, else run predefined 5 tests
    std::vector<std::pair<std::string, std::string>> tests;
    if (argc >= 3) {
        tests.emplace_back(argv[1], argv[2]);
    } else {
        tests.push_back({"example.com", "alice"});              // Found
        tests.push_back({"dc=example,dc=com", "john.doe"});     // Found
        tests.push_back({"test.org", "carol"});                 // Found
        tests.push_back({"example.com", "doesnotexist"});       // Not found
        tests.push_back({"example.com", "alice*)(uid=*"});      // Invalid username (rejected)
    }

    for (size_t i = 0; i < tests.size(); ++i) {
        const auto& dc = tests[i].first;
        const auto& user = tests[i].second;
        Result res = processRequest(dc, user);
        std::cout << "Test " << (i + 1) << " dc=\"" << dc << "\" user=\"" << user << "\"\n";
        if (res.status == Status::Ok) {
            std::cout << "OK\n" << res.info;
        } else {
            std::cout << "ERROR: " << res.error << "\n";
        }
        if (i + 1 < tests.size()) std::cout << "-----\n";
    }
    return 0;
}