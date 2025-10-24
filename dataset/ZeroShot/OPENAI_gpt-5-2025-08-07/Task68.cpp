#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <ldap.h>
#include <cstring>

static std::string escape_ldap_filter(const std::string& input) {
    std::ostringstream oss;
    for (unsigned char c : input) {
        switch (c) {
            case '\\': oss << "\\5c"; break;
            case '*':  oss << "\\2a"; break;
            case '(':  oss << "\\28"; break;
            case ')':  oss << "\\29"; break;
            case '\0': oss << "\\00"; break;
            default:
                if (c < 0x20 || c == 0x7F) {
                    char buf[5];
                    snprintf(buf, sizeof(buf), "\\%02x", (unsigned int)c);
                    oss << buf;
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

static std::string escape_dn_value(const std::string& value) {
    std::ostringstream oss;
    for (size_t i = 0; i < value.size(); ++i) {
        char c = value[i];
        bool must = false;
        if (i == 0 && (c == ' ' || c == '#')) must = true;
        if (i + 1 == value.size() && c == ' ') must = true;
        if (c == ',' || c == '+' || c == '"' || c == '\\' || c == '<' || c == '>' || c == ';' || c == '=') must = true;
        if (must) oss << '\\' << c; else oss << c;
    }
    return oss.str();
}

static bool plausible_domain(const std::string& domain) {
    if (domain.empty() || domain.size() > 253) return false;
    if (domain.find("..") != std::string::npos) return false;
    std::istringstream ss(domain);
    std::string label;
    while (std::getline(ss, label, '.')) {
        if (label.empty() || label.size() > 63) return false;
        if (label.front() == '-' || label.back() == '-') return false;
        for (char c : label) {
            if (!((c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') ||
                  c == '-')) return false;
        }
    }
    return true;
}

static bool plausible_username(const std::string& username) {
    if (username.empty() || username.size() > 128) return false;
    for (char c : username) {
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}

static std::string base_dn_from_domain(const std::string& domain) {
    std::ostringstream oss;
    std::istringstream ss(domain);
    std::string part;
    bool first = true;
    while (std::getline(ss, part, '.')) {
        if (!first) oss << ",";
        first = false;
        oss << "dc=" << escape_dn_value(part);
    }
    return oss.str();
}

std::string findUser(const std::string& domain, const std::string& username) {
    if (!plausible_domain(domain)) {
        return std::string("{\"error\":\"invalid_domain\"}");
    }
    if (!plausible_username(username)) {
        return std::string("{\"error\":\"invalid_username\"}");
    }

    std::string baseDN = base_dn_from_domain(domain);
    std::string filter = "(uid=" + escape_ldap_filter(username) + ")";

    LDAP* ld = nullptr;
    int rc = ldap_initialize(&ld, "ldap://127.0.0.1:389");
    if (rc != LDAP_SUCCESS || ld == nullptr) {
        return std::string("{\"error\":\"ldap_init_failed\"}");
    }

    // Timeouts
    struct timeval network_timeout;
    network_timeout.tv_sec = 5;
    network_timeout.tv_usec = 0;
    ldap_set_option(ld, LDAP_OPT_NETWORK_TIMEOUT, &network_timeout);

    // Anonymous simple bind
    rc = ldap_simple_bind_s(ld, nullptr, nullptr);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return std::string("{\"error\":\"bind_failed\"}");
    }

    LDAPMessage* res = nullptr;
    rc = ldap_search_ext_s(ld, baseDN.c_str(), LDAP_SCOPE_SUBTREE, filter.c_str(), nullptr, 0, nullptr, nullptr, &network_timeout, 1, &res);
    if (rc != LDAP_SUCCESS) {
        if (res) ldap_msgfree(res);
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return std::string("{\"error\":\"search_failed\"}");
    }

    LDAPMessage* entry = ldap_first_entry(ld, res);
    if (!entry) {
        ldap_msgfree(res);
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return std::string("{\"result\":null}");
    }

    char* dn = ldap_get_dn(ld, entry);
    std::string dnStr = dn ? dn : "";
    if (dn) ldap_memfree(dn);

    std::string cnStr, uidStr, mailStr;

    BerElement* ber = nullptr;
    for (char* attr = ldap_first_attribute(ld, entry, &ber); attr != nullptr; attr = ldap_next_attribute(ld, entry, ber)) {
        struct berval** vals = ldap_get_values_len(ld, entry, attr);
        if (vals && vals[0]) {
            std::string val(vals[0]->bv_val, (size_t)vals[0]->bv_len);
            if (std::strcmp(attr, "cn") == 0) cnStr = val;
            else if (std::strcmp(attr, "uid") == 0) uidStr = val;
            else if (std::strcmp(attr, "mail") == 0) mailStr = val;
        }
        if (vals) ldap_value_free_len(vals);
        ldap_memfree(attr);
    }
    if (ber) ber_free(ber, 0);

    ldap_msgfree(res);
    ldap_unbind_ext_s(ld, nullptr, nullptr);

    auto json_escape = [](const std::string& s) {
        std::ostringstream o;
        for (char c : s) {
            switch (c) {
                case '\\': o << "\\\\"; break;
                case '"': o << "\\\""; break;
                case '\b': o << "\\b"; break;
                case '\f': o << "\\f"; break;
                case '\n': o << "\\n"; break;
                case '\r': o << "\\r"; break;
                case '\t': o << "\\t"; break;
                default:
                    if ((unsigned char)c < 0x20) {
                        char buf[7];
                        snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char)c);
                        o << buf;
                    } else {
                        o << c;
                    }
            }
        }
        return o.str();
    };

    std::ostringstream out;
    out << "{"
        << "\"dn\":\"" << json_escape(dnStr) << "\","
        << "\"cn\":\"" << json_escape(cnStr) << "\","
        << "\"uid\":\"" << json_escape(uidStr) << "\","
        << "\"mail\":\"" << json_escape(mailStr) << "\""
        << "}";
    return out.str();
}

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        std::cout << findUser(argv[1], argv[2]) << std::endl;
        return 0;
    }
    std::cout << findUser("example.com", "jdoe") << std::endl;
    std::cout << findUser("example.com", "alice") << std::endl;
    std::cout << findUser("sub.example.com", "bob_smith") << std::endl;
    std::cout << findUser(std::string("invalid_domain!"), "user") << std::endl;
    std::cout << findUser("example.com", "bad*user") << std::endl;
    return 0;
}