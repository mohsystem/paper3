#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <ldap.h>

static std::string escapeLdapFilterValue(const std::string& in) {
    std::string out;
    out.reserve(in.size() * 2);
    for (unsigned char c : in) {
        switch (c) {
            case '*': out += "\\2a"; break;
            case '(': out += "\\28"; break;
            case ')': out += "\\29"; break;
            case '\\': out += "\\5c"; break;
            case '\0': out += "\\00"; break;
            default: out.push_back((char)c); break;
        }
    }
    return out;
}

static bool isValidLabel(const std::string& s) {
    if (s.empty()) return false;
    if (s.front() == '-' || s.back() == '-') return false;
    for (unsigned char c : s) {
        if (!(std::isalnum(c) || c == '-')) return false;
    }
    return true;
}

static std::string toLowerStr(const std::string& s) {
    std::string o(s);
    std::transform(o.begin(), o.end(), o.begin(), [](unsigned char c){ return std::tolower(c); });
    return o;
}

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    std::istringstream iss(s);
    while (std::getline(iss, cur, delim)) {
        // trim
        size_t b = cur.find_first_not_of(" \t\r\n");
        size_t e = cur.find_last_not_of(" \t\r\n");
        if (b == std::string::npos) out.emplace_back("");
        else out.emplace_back(cur.substr(b, e - b + 1));
    }
    return out;
}

static std::string buildBaseDN(const std::string& dcInput) {
    std::string s = dcInput;
    // trim
    auto l = s.find_first_not_of(" \t\r\n");
    auto r = s.find_last_not_of(" \t\r\n");
    if (l == std::string::npos) return "dc=localhost";
    s = s.substr(l, r - l + 1);
    if (s.find('=') != std::string::npos) {
        auto parts = split(s, ',');
        std::vector<std::string> out;
        for (auto& p : parts) {
            auto kv = split(p, '=');
            if (kv.size() != 2) continue;
            std::string k = toLowerStr(kv[0]);
            std::string v = toLowerStr(kv[1]);
            if (k != "dc") continue;
            if (!isValidLabel(v)) continue;
            out.push_back("dc=" + v);
        }
        if (out.empty()) return "dc=localhost";
        std::ostringstream oss;
        for (size_t i = 0; i < out.size(); ++i) {
            if (i) oss << ",";
            oss << out[i];
        }
        return oss.str();
    } else {
        auto labels = split(s, '.');
        std::vector<std::string> out;
        for (auto& lbl : labels) {
            std::string v = toLowerStr(lbl);
            if (!isValidLabel(v)) continue;
            out.push_back("dc=" + v);
        }
        if (out.empty()) return "dc=localhost";
        std::ostringstream oss;
        for (size_t i = 0; i < out.size(); ++i) {
            if (i) oss << ",";
            oss << out[i];
        }
        return oss.str();
    }
}

std::string searchUser(const std::string& dcInput, const std::string& username) {
    std::string baseDn = buildBaseDN(dcInput);
    std::string userEsc = escapeLdapFilterValue(username);
    std::string filter = "(|(uid=" + userEsc + ")(cn=" + userEsc + ")(sAMAccountName=" + userEsc + ")(userPrincipalName=" + userEsc + "))";

    LDAP *ld = nullptr;
    int rc = ldap_initialize(&ld, "ldap://localhost:389");
    if (rc != LDAP_SUCCESS || !ld) {
        return std::string("{\"error\":\"ldap_initialize failed: ") + ldap_err2string(rc) + "\"}";
    }

    int version = LDAP_VERSION3;
    ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);
    int referrals = LDAP_OPT_OFF;
    ldap_set_option(ld, LDAP_OPT_REFERRALS, &referrals);
    struct timeval networkTimeout;
    networkTimeout.tv_sec = 3;
    networkTimeout.tv_usec = 0;
    ldap_set_option(ld, LDAP_OPT_NETWORK_TIMEOUT, &networkTimeout);
    struct timeval timeLimit;
    timeLimit.tv_sec = 5;
    timeLimit.tv_usec = 0;

    rc = ldap_simple_bind_s(ld, nullptr, nullptr);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return std::string("{\"error\":\"ldap_simple_bind_s failed: ") + ldap_err2string(rc) + "\"}";
    }

    char* attrs[] = {
        (char*)"cn",
        (char*)"sn",
        (char*)"givenName",
        (char*)"uid",
        (char*)"mail",
        (char*)"sAMAccountName",
        (char*)"userPrincipalName",
        nullptr
    };

    LDAPMessage* res = nullptr;
    rc = ldap_search_ext_s(ld, baseDn.c_str(), LDAP_SCOPE_SUBTREE, filter.c_str(), attrs, 0, nullptr, nullptr, &timeLimit, 1, &res);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        if (res) ldap_msgfree(res);
        return std::string("{\"error\":\"ldap_search_ext_s failed: ") + ldap_err2string(rc) + "\"}";
    }

    LDAPMessage* entry = ldap_first_entry(ld, res);
    if (!entry) {
        ldap_msgfree(res);
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return "{\"error\":\"NOT_FOUND\"}";
    }

    char* dn = ldap_get_dn(ld, entry);
    std::string json = "{\"dn\":\"";
    if (dn) {
        std::string dns(dn);
        // escape JSON
        for (char c : dns) {
            if (c == '\\') json += "\\\\";
            else if (c == '"') json += "\\\"";
            else json += c;
        }
        ldap_memfree(dn);
    }
    json += "\",\"attributes\":{";

    BerElement* ber = nullptr;
    char* attr = ldap_first_attribute(ld, entry, &ber);
    bool first = true;
    while (attr) {
        struct berval** vals = ldap_get_values_len(ld, entry, attr);
        if (vals) {
            if (!first) json += ",";
            first = false;
            // attribute name
            std::string an(attr);
            json += "\"";
            for (char c : an) {
                if (c == '\\') json += "\\\\";
                else if (c == '"') json += "\\\"";
                else json += c;
            }
            json += "\":";
            if (vals[1] == nullptr) {
                // single
                std::string v(vals[0]->bv_val, (size_t)vals[0]->bv_len);
                json += "\"";
                for (char c : v) {
                    if (c == '\\') json += "\\\\";
                    else if (c == '"') json += "\\\"";
                    else json += c;
                }
                json += "\"";
            } else {
                json += "[";
                for (int i = 0; vals[i] != nullptr; ++i) {
                    if (i) json += ",";
                    std::string v(vals[i]->bv_val, (size_t)vals[i]->bv_len);
                    json += "\"";
                    for (char c : v) {
                        if (c == '\\') json += "\\\\";
                        else if (c == '"') json += "\\\"";
                        else json += c;
                    }
                    json += "\"";
                }
                json += "]";
            }
            ldap_value_free_len(vals);
        }
        ldap_memfree(attr);
        attr = ldap_next_attribute(ld, entry, ber);
    }
    if (ber) ber_free(ber, 0);

    json += "}}";

    ldap_msgfree(res);
    ldap_unbind_ext_s(ld, nullptr, nullptr);

    return json;
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        std::cout << searchUser(argv[1], argv[2]) << std::endl;
        return 0;
    }
    std::vector<std::pair<std::string, std::string>> tests = {
        {"example.com", "alice"},
        {"dc=example,dc=com", "bob"},
        {"EXAMPLE.ORG", "charlie"},
        {"invalid$$domain", "dave"},
        {"dc=local,dc=domain", "eve"}
    };
    for (auto& t : tests) {
        std::cout << "Input: dc=" << t.first << ", user=" << t.second << std::endl;
        std::cout << searchUser(t.first, t.second) << std::endl;
    }
    return 0;
}