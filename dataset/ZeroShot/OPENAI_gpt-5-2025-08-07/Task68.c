#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ldap.h>
#include <ctype.h>

static char* escape_ldap_filter(const char* input) {
    if (!input) {
        char* e = (char*)calloc(1,1);
        return e;
    }
    size_t len = strlen(input);
    // Worst case every char becomes \xx (3 chars)
    size_t cap = len * 3 + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)input[i];
        const char* rep = NULL;
        char buf[4] = {0};
        switch (c) {
            case '\\': rep = "\\5c"; break;
            case '*':  rep = "\\2a"; break;
            case '(':  rep = "\\28"; break;
            case ')':  rep = "\\29"; break;
            case '\0': rep = "\\00"; break;
            default:
                if (c < 0x20 || c == 0x7F) {
                    snprintf(buf, sizeof(buf), "\\%02x", c);
                    rep = buf;
                }
        }
        if (rep) {
            size_t rlen = strlen(rep);
            memcpy(out + pos, rep, rlen);
            pos += rlen;
        } else {
            out[pos++] = c;
        }
    }
    out[pos] = '\0';
    return out;
}

static char* escape_dn_value(const char* value) {
    if (!value) {
        char* e = (char*)calloc(1,1);
        return e;
    }
    size_t len = strlen(value);
    // Worst case every char is escaped with backslash -> 2x
    size_t cap = len * 2 + 3;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        char c = value[i];
        int must = 0;
        if (i == 0 && (c == ' ' || c == '#')) must = 1;
        if (i + 1 == len && c == ' ') must = 1;
        if (c == ',' || c == '+' || c == '"' || c == '\\' || c == '<' || c == '>' || c == ';' || c == '=') must = 1;
        if (must) {
            out[pos++] = '\\';
            out[pos++] = c;
        } else {
            out[pos++] = c;
        }
    }
    out[pos] = '\0';
    return out;
}

static int plausible_domain(const char* domain) {
    if (!domain) return 0;
    size_t n = strlen(domain);
    if (n == 0 || n > 253) return 0;
    if (strstr(domain, "..")) return 0;
    char* copy = strdup(domain);
    if (!copy) return 0;
    char* save = NULL;
    char* tok = strtok_r(copy, ".", &save);
    int ok = 1;
    while (tok) {
        size_t l = strlen(tok);
        if (l == 0 || l > 63) { ok = 0; break; }
        if (tok[0] == '-' || tok[l-1] == '-') { ok = 0; break; }
        for (size_t i = 0; i < l; i++) {
            char c = tok[i];
            if (!(isalnum((unsigned char)c) || c == '-')) { ok = 0; break; }
        }
        if (!ok) break;
        tok = strtok_r(NULL, ".", &save);
    }
    free(copy);
    return ok;
}

static int plausible_username(const char* username) {
    if (!username) return 0;
    size_t n = strlen(username);
    if (n == 0 || n > 128) return 0;
    for (size_t i = 0; i < n; i++) {
        char c = username[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static char* base_dn_from_domain(const char* domain) {
    // Estimate capacity
    size_t len = strlen(domain);
    size_t cap = len * 3 + 10;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    out[0] = '\0';
    char* copy = strdup(domain);
    if (!copy) { free(out); return NULL; }
    char* save = NULL;
    char* tok = strtok_r(copy, ".", &save);
    int first = 1;
    while (tok) {
        char* esc = escape_dn_value(tok);
        if (!esc) { free(copy); free(out); return NULL; }
        if (!first) strcat(out, ",");
        first = 0;
        strcat(out, "dc=");
        strcat(out, esc);
        free(esc);
        tok = strtok_r(NULL, ".", &save);
    }
    free(copy);
    return out;
}

char* findUser(const char* domain, const char* username) {
    if (!plausible_domain(domain)) {
        char* r = strdup("{\"error\":\"invalid_domain\"}");
        return r;
    }
    if (!plausible_username(username)) {
        char* r = strdup("{\"error\":\"invalid_username\"}");
        return r;
    }

    char* baseDN = base_dn_from_domain(domain);
    if (!baseDN) {
        char* r = strdup("{\"error\":\"memory\"}");
        return r;
    }
    char* escUser = escape_ldap_filter(username);
    if (!escUser) {
        free(baseDN);
        char* r = strdup("{\"error\":\"memory\"}");
        return r;
    }
    char filter[512];
    snprintf(filter, sizeof(filter), "(uid=%s)", escUser);

    LDAP* ld = NULL;
    int rc = ldap_initialize(&ld, "ldap://127.0.0.1:389");
    if (rc != LDAP_SUCCESS || !ld) {
        free(baseDN);
        free(escUser);
        char* r = strdup("{\"error\":\"ldap_init_failed\"}");
        return r;
    }

    struct timeval nettimeout;
    nettimeout.tv_sec = 5;
    nettimeout.tv_usec = 0;
    ldap_set_option(ld, LDAP_OPT_NETWORK_TIMEOUT, &nettimeout);

    rc = ldap_simple_bind_s(ld, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, NULL, NULL);
        free(baseDN);
        free(escUser);
        char* r = strdup("{\"error\":\"bind_failed\"}");
        return r;
    }

    LDAPMessage* res = NULL;
    rc = ldap_search_ext_s(ld, baseDN, LDAP_SCOPE_SUBTREE, filter, NULL, 0, NULL, NULL, &nettimeout, 1, &res);
    free(baseDN);
    free(escUser);

    if (rc != LDAP_SUCCESS) {
        if (res) ldap_msgfree(res);
        ldap_unbind_ext_s(ld, NULL, NULL);
        char* r = strdup("{\"error\":\"search_failed\"}");
        return r;
    }

    LDAPMessage* entry = ldap_first_entry(ld, res);
    if (!entry) {
        ldap_msgfree(res);
        ldap_unbind_ext_s(ld, NULL, NULL);
        char* r = strdup("{\"result\":null}");
        return r;
    }

    char* dn = ldap_get_dn(ld, entry);
    const char* dnStr = dn ? dn : "";

    char cnBuf[512] = {0};
    char uidBuf[512] = {0};
    char mailBuf[512] = {0};

    BerElement* ber = NULL;
    for (char* attr = ldap_first_attribute(ld, entry, &ber); attr != NULL; attr = ldap_next_attribute(ld, entry, ber)) {
        struct berval** vals = ldap_get_values_len(ld, entry, attr);
        if (vals && vals[0]) {
            size_t l = vals[0]->bv_len;
            if (strcmp(attr, "cn") == 0) {
                size_t copy = l < sizeof(cnBuf)-1 ? l : sizeof(cnBuf)-1;
                memcpy(cnBuf, vals[0]->bv_val, copy);
                cnBuf[copy] = '\0';
            } else if (strcmp(attr, "uid") == 0) {
                size_t copy = l < sizeof(uidBuf)-1 ? l : sizeof(uidBuf)-1;
                memcpy(uidBuf, vals[0]->bv_val, copy);
                uidBuf[copy] = '\0';
            } else if (strcmp(attr, "mail") == 0) {
                size_t copy = l < sizeof(mailBuf)-1 ? l : sizeof(mailBuf)-1;
                memcpy(mailBuf, vals[0]->bv_val, copy);
                mailBuf[copy] = '\0';
            }
        }
        if (vals) ldap_value_free_len(vals);
        ldap_memfree(attr);
    }
    if (ber) ber_free(ber, 0);

    ldap_msgfree(res);
    ldap_unbind_ext_s(ld, NULL, NULL);

    // JSON escape minimal
    auto json_escape = [](const char* s) {
        size_t len = strlen(s);
        // worst case escape each char with \u00XX
        size_t cap = len * 6 + 1;
        char* out = (char*)malloc(cap);
        if (!out) return (char*)NULL;
        size_t pos = 0;
        for (size_t i = 0; i < len; i++) {
            unsigned char c = (unsigned char)s[i];
            if (c == '\\' || c == '"') {
                out[pos++] = '\\';
                out[pos++] = c;
            } else if (c == '\b') { memcpy(out+pos, "\\b", 2); pos+=2; }
            else if (c == '\f') { memcpy(out+pos, "\\f", 2); pos+=2; }
            else if (c == '\n') { memcpy(out+pos, "\\n", 2); pos+=2; }
            else if (c == '\r') { memcpy(out+pos, "\\r", 2); pos+=2; }
            else if (c == '\t') { memcpy(out+pos, "\\t", 2); pos+=2; }
            else if (c < 0x20) {
                char buf[7];
                snprintf(buf, sizeof(buf), "\\u%04x", c);
                size_t bl = strlen(buf);
                memcpy(out + pos, buf, bl);
                pos += bl;
            } else {
                out[pos++] = c;
            }
        }
        out[pos] = '\0';
        return out;
    };

    char* dnEsc = json_escape(dnStr);
    char* cnEsc = json_escape(cnBuf);
    char* uidEsc = json_escape(uidBuf);
    char* mailEsc = json_escape(mailBuf);

    size_t total = 20 + (dnEsc ? strlen(dnEsc) : 0) + (cnEsc ? strlen(cnEsc) : 0)
                   + (uidEsc ? strlen(uidEsc) : 0) + (mailEsc ? strlen(mailEsc) : 0) + 64;
    char* json = (char*)malloc(total);
    if (!json) {
        if (dn) ldap_memfree(dn);
        free(dnEsc); free(cnEsc); free(uidEsc); free(mailEsc);
        char* r = strdup("{\"error\":\"memory\"}");
        return r;
    }

    snprintf(json, total, "{\"dn\":\"%s\",\"cn\":\"%s\",\"uid\":\"%s\",\"mail\":\"%s\"}",
             dnEsc ? dnEsc : "", cnEsc ? cnEsc : "", uidEsc ? uidEsc : "", mailEsc ? mailEsc : "");

    if (dn) ldap_memfree(dn);
    free(dnEsc); free(cnEsc); free(uidEsc); free(mailEsc);
    return json;
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        char* res = findUser(argv[1], argv[2]);
        if (res) {
            puts(res);
            free(res);
        }
        return 0;
    }
    const char* tests[5][2] = {
        {"example.com","jdoe"},
        {"example.com","alice"},
        {"sub.example.com","bob_smith"},
        {"invalid_domain!","user"},
        {"example.com","bad*user"}
    };
    for (int i = 0; i < 5; i++) {
        char* res = findUser(tests[i][0], tests[i][1]);
        if (res) {
            puts(res);
            free(res);
        }
    }
    return 0;
}