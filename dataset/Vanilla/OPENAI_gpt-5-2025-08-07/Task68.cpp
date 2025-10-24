#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <ldap.h>

static std::string dcToBaseDN(const std::string& dc) {
    std::string t = dc;
    // trim
    size_t start = t.find_first_not_of(" \t\r\n");
    size_t end = t.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    t = t.substr(start, end - start + 1);
    std::string lower = t;
    for (auto &c : lower) c = (char)tolower(c);
    if (lower.find("dc=") != std::string::npos) return t;
    std::vector<std::string> parts;
    std::string cur;
    for (char c : t) {
        if (c == '.' || c == ',') {
            if (!cur.empty()) {
                parts.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) parts.push_back(cur);
    std::ostringstream oss;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) oss << ",";
        oss << "dc=" << parts[i];
    }
    return oss.str();
}

static std::string ldapEscape(const std::string& s) {
    std::ostringstream oss;
    for (unsigned char c : s) {
        switch (c) {
            case '\\': oss << "\\5c"; break;
            case '*':  oss << "\\2a"; break;
            case '(':  oss << "\\28"; break;
            case ')':  oss << "\\29"; break;
            case '\0': oss << "\\00"; break;
            default:   oss << c; break;
        }
    }
    return oss.str();
}

std::string queryLdap(const std::string& dc, const std::string& username) {
    std::string baseDN = dcToBaseDN(dc);
    std::string escUser = ldapEscape(username);
    std::string filter = "(|(uid=" + escUser + ")(sAMAccountName=" + escUser + ")(cn=" + escUser + "))";

    LDAP* ld = nullptr;
    int rc = ldap_initialize(&ld, "ldap://localhost:389");
    if (rc != LDAP_SUCCESS || ld == nullptr) {
        return std::string("ERROR: ldap_initialize failed: ") + ldap_err2string(rc);
    }

    int version = LDAP_VERSION3;
    ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);

    rc = ldap_simple_bind_s(ld, nullptr, nullptr);
    if (rc != LDAP_SUCCESS) {
        std::string err = std::string("ERROR: bind failed: ") + ldap_err2string(rc);
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return err;
    }

    LDAPMessage* res = nullptr;
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    rc = ldap_search_ext_s(ld, baseDN.c_str(), LDAP_SCOPE_SUBTREE, filter.c_str(), nullptr, 0, nullptr, nullptr, &tv, 0, &res);
    if (rc != LDAP_SUCCESS) {
        std::string err = std::string("ERROR: search failed: ") + ldap_err2string(rc);
        if (res) ldap_msgfree(res);
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return err;
    }

    LDAPMessage* entry = ldap_first_entry(ld, res);
    if (!entry) {
        ldap_msgfree(res);
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return "NOT_FOUND";
    }

    char* dn = ldap_get_dn(ld, entry);
    std::ostringstream out;
    out << "{dn:\"" << (dn ? dn : "") << "\",attributes:{";

    BerElement* ber = nullptr;
    char* attr = ldap_first_attribute(ld, entry, &ber);
    bool firstAttr = true;
    while (attr != nullptr) {
        struct berval** vals = ldap_get_values_len(ld, entry, attr);
        int count = vals ? ldap_count_values_len(vals) : 0;

        if (!firstAttr) out << ",";
        firstAttr = false;
        out << "\"" << attr << "\":[";
        for (int i = 0; i < count; ++i) {
            if (i) out << ",";
            std::string val(vals[i]->bv_val, vals[i]->bv_len);
            for (auto &ch : val) {
                if (ch == '"') ch = '\'';
            }
            out << "\"" << val << "\"";
        }
        out << "]";

        if (vals) ldap_value_free_len(vals);
        ldap_memfree(attr);
        attr = ldap_next_attribute(ld, entry, ber);
    }
    if (ber) ber_free(ber, 0);

    out << "}}";

    if (dn) ldap_memfree(dn);
    ldap_msgfree(res);
    ldap_unbind_ext_s(ld, nullptr, nullptr);

    return out.str();
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        std::cout << queryLdap(argv[1], argv[2]) << std::endl;
        return 0;
    }
    std::vector<std::pair<std::string, std::string>> tests = {
        {"example.com", "alice"},
        {"example.com", "bob"},
        {"dc=example,dc=com", "carol"},
        {"example,org", "dave"},
        {"test.local", "eve"}
    };
    for (auto &t : tests) {
        std::cout << "DC=" << t.first << " USER=" << t.second << std::endl;
        std::cout << queryLdap(t.first, t.second) << std::endl;
    }
    return 0;
}