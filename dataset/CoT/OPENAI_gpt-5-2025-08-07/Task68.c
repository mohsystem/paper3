#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ldap.h>

static char* escape_ldap_filter_value(const char* in) {
    if (!in) {
        char* s = (char*)malloc(1);
        if (s) s[0] = '\0';
        return s;
    }
    size_t len = strlen(in);
    // worst case expand *2 + some
    size_t cap = len * 3 + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)in[i];
        const char* rep = NULL;
        switch (c) {
            case '*': rep = "\\2a"; break;
            case '(': rep = "\\28"; break;
            case ')': rep = "\\29"; break;
            case '\\': rep = "\\5c"; break;
            case '\0': rep = "\\00"; break;
            default: rep = NULL; break;
        }
        if (rep) {
            size_t rlen = 3;
            if (j + rlen >= cap) {
                cap *= 2;
                char* tmp = (char*)realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[j++] = rep[0];
            out[j++] = rep[1];
            out[j++] = rep[2];
        } else {
            if (j + 1 >= cap) {
                cap *= 2;
                char* tmp = (char*)realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[j++] = (char)c;
        }
    }
    out[j] = '\0';
    return out;
}

static int is_valid_label(const char* s) {
    if (!s || !*s) return 0;
    size_t n = strlen(s);
    if (s[0] == '-' || s[n-1] == '-') return 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '-')) return 0;
    }
    return 1;
}

static char* str_tolower_dup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < n; ++i) {
        out[i] = (char)tolower((unsigned char)s[i]);
    }
    out[n] = '\0';
    return out;
}

static char* trim_dup(const char* s) {
    if (!s) return NULL;
    const char* b = s;
    while (*b && isspace((unsigned char)*b)) b++;
    const char* e = s + strlen(s);
    while (e > b && isspace((unsigned char)*(e - 1))) e--;
    size_t n = (size_t)(e - b);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, b, n);
    out[n] = '\0';
    return out;
}

static char* build_base_dn(const char* dc_input) {
    if (!dc_input) {
        char* s = (char*)malloc(12);
        if (s) strcpy(s, "dc=localhost");
        return s;
    }
    char* s_trim = trim_dup(dc_input);
    if (!s_trim) return NULL;
    if (s_trim[0] == '\0') {
        free(s_trim);
        char* s = (char*)malloc(12);
        if (s) strcpy(s, "dc=localhost");
        return s;
    }
    char* base = NULL;
    if (strchr(s_trim, '=')) {
        // DN-like
        // Split by ','
        char* work = strdup(s_trim);
        if (!work) { free(s_trim); return NULL; }
        size_t cap = 64;
        base = (char*)malloc(cap);
        if (!base) { free(work); free(s_trim); return NULL; }
        base[0] = '\0';
        int first = 1;
        char* saveptr = NULL;
        for (char* token = strtok_r(work, ",", &saveptr); token; token = strtok_r(NULL, ",", &saveptr)) {
            char* ttrim = trim_dup(token);
            if (!ttrim) continue;
            char* eq = strchr(ttrim, '=');
            if (!eq) { free(ttrim); continue; }
            *eq = '\0';
            char* k = ttrim;
            char* v = eq + 1;
            char* k_low = str_tolower_dup(k);
            char* v_low = str_tolower_dup(v);
            if (k_low && strcmp(k_low, "dc") == 0 && v_low && is_valid_label(v_low)) {
                // append "dc=" + v_low
                size_t addlen = (first ? 0 : 1) + 3 + strlen(v_low);
                size_t cur = strlen(base);
                if (cur + addlen + 1 >= cap) {
                    while (cur + addlen + 1 >= cap) cap *= 2;
                    char* tmp = (char*)realloc(base, cap);
                    if (!tmp) { free(base); base = NULL; }
                    else base = tmp;
                    if (!base) { free(k_low); free(v_low); free(ttrim); break; }
                }
                if (!first) strcat(base, ","); else first = 0;
                strcat(base, "dc=");
                strcat(base, v_low);
            }
            if (k_low) free(k_low);
            if (v_low) free(v_low);
            free(ttrim);
        }
        free(work);
        if (!base || base[0] == '\0') {
            if (base) { free(base); base = NULL; }
            base = (char*)malloc(12);
            if (base) strcpy(base, "dc=localhost");
        }
    } else {
        // domain-like
        char* work = strdup(s_trim);
        if (!work) { free(s_trim); return NULL; }
        size_t cap = 64;
        base = (char*)malloc(cap);
        if (!base) { free(work); free(s_trim); return NULL; }
        base[0] = '\0';
        int first = 1;
        char* saveptr = NULL;
        for (char* token = strtok_r(work, ".", &saveptr); token; token = strtok_r(NULL, ".", &saveptr)) {
            char* ttrim = trim_dup(token);
            if (!ttrim) continue;
            char* low = str_tolower_dup(ttrim);
            if (low && is_valid_label(low)) {
                size_t addlen = (first ? 0 : 1) + 3 + strlen(low);
                size_t cur = strlen(base);
                if (cur + addlen + 1 >= cap) {
                    while (cur + addlen + 1 >= cap) cap *= 2;
                    char* tmp = (char*)realloc(base, cap);
                    if (!tmp) { free(base); base = NULL; }
                    else base = tmp;
                    if (!base) { free(low); free(ttrim); break; }
                }
                if (!first) strcat(base, ","); else first = 0;
                strcat(base, "dc=");
                strcat(base, low);
            }
            if (low) free(low);
            free(ttrim);
        }
        free(work);
        if (!base || base[0] == '\0') {
            if (base) { free(base); base = NULL; }
            base = (char*)malloc(12);
            if (base) strcpy(base, "dc=localhost");
        }
    }
    free(s_trim);
    return base;
}

char* search_user(const char* dc_input, const char* username) {
    char* base_dn = build_base_dn(dc_input);
    if (!base_dn) {
        char* err = (char*)malloc(32);
        if (err) strcpy(err, "{\"error\":\"build_dn_failed\"}");
        return err;
    }
    char* user_esc = escape_ldap_filter_value(username ? username : "");
    if (!user_esc) {
        free(base_dn);
        char* err = (char*)malloc(32);
        if (err) strcpy(err, "{\"error\":\"escape_failed\"}");
        return err;
    }

    size_t flen = strlen(user_esc) * 2 + 64;
    char* filter = (char*)malloc(flen);
    if (!filter) {
        free(base_dn);
        free(user_esc);
        char* err = (char*)malloc(32);
        if (err) strcpy(err, "{\"error\":\"alloc_failed\"}");
        return err;
    }
    snprintf(filter, flen, "(|(uid=%s)(cn=%s)(sAMAccountName=%s)(userPrincipalName=%s))", user_esc, user_esc, user_esc, user_esc);

    LDAP* ld = NULL;
    int rc = ldap_initialize(&ld, "ldap://localhost:389");
    if (rc != LDAP_SUCCESS || !ld) {
        free(base_dn); free(user_esc); free(filter);
        const char* msg = ldap_err2string(rc);
        size_t bl = 30 + (msg ? strlen(msg) : 0);
        char* err = (char*)malloc(bl);
        if (err) snprintf(err, bl, "{\"error\":\"%s\"}", msg ? msg : "ldap_init_failed");
        return err;
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

    rc = ldap_simple_bind_s(ld, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        const char* msg = ldap_err2string(rc);
        ldap_unbind_ext_s(ld, NULL, NULL);
        free(base_dn); free(user_esc); free(filter);
        size_t bl = 40 + (msg ? strlen(msg) : 0);
        char* err = (char*)malloc(bl);
        if (err) snprintf(err, bl, "{\"error\":\"bind_failed: %s\"}", msg ? msg : "");
        return err;
    }

    char* attrs[] = { "cn", "sn", "givenName", "uid", "mail", "sAMAccountName", "userPrincipalName", NULL };
    LDAPMessage* res = NULL;
    rc = ldap_search_ext_s(ld, base_dn, LDAP_SCOPE_SUBTREE, filter, attrs, 0, NULL, NULL, &timeLimit, 1, &res);
    if (rc != LDAP_SUCCESS) {
        const char* msg = ldap_err2string(rc);
        if (res) ldap_msgfree(res);
        ldap_unbind_ext_s(ld, NULL, NULL);
        free(base_dn); free(user_esc); free(filter);
        size_t bl = 50 + (msg ? strlen(msg) : 0);
        char* err = (char*)malloc(bl);
        if (err) snprintf(err, bl, "{\"error\":\"search_failed: %s\"}", msg ? msg : "");
        return err;
    }

    LDAPMessage* entry = ldap_first_entry(ld, res);
    if (!entry) {
        ldap_msgfree(res);
        ldap_unbind_ext_s(ld, NULL, NULL);
        free(base_dn); free(user_esc); free(filter);
        char* out = (char*)malloc(22);
        if (out) strcpy(out, "{\"error\":\"NOT_FOUND\"}");
        return out;
    }

    char* dn = ldap_get_dn(ld, entry);
    // Build JSON string
    size_t cap = 512;
    char* json = (char*)malloc(cap);
    if (!json) {
        if (dn) ldap_memfree(dn);
        ldap_msgfree(res);
        ldap_unbind_ext_s(ld, NULL, NULL);
        free(base_dn); free(user_esc); free(filter);
        char* err = (char*)malloc(32);
        if (err) strcpy(err, "{\"error\":\"alloc_failed\"}");
        return err;
    }
    size_t len = 0;
    #define APPEND_FMT(fmt, ...) do { \
        char tmpbuf[1024]; \
        int n = snprintf(tmpbuf, sizeof(tmpbuf), fmt, __VA_ARGS__); \
        if (n < 0) n = 0; \
        if (len + (size_t)n + 1 > cap) { \
            while (len + (size_t)n + 1 > cap) cap *= 2; \
            char* t = (char*)realloc(json, cap); \
            if (!t) { free(json); json = NULL; goto cleanup; } \
            json = t; \
        } \
        memcpy(json + len, tmpbuf, (size_t)n); \
        len += (size_t)n; \
        json[len] = '\0'; \
    } while(0)

    // escape dn for JSON
    char* dn_esc = NULL;
    if (dn) {
        size_t dnl = strlen(dn);
        dn_esc = (char*)malloc(dnl * 2 + 1);
        if (dn_esc) {
            size_t j = 0;
            for (size_t i = 0; i < dnl; ++i) {
                char c = dn[i];
                if (c == '\\' || c == '\"') {
                    dn_esc[j++] = '\\';
                }
                dn_esc[j++] = c;
            }
            dn_esc[j] = '\0';
        }
    }
    APPEND_FMT("{\"dn\":\"%s\",\"attributes\":{", dn_esc ? dn_esc : "");

    BerElement* ber = NULL;
    char* attr = ldap_first_attribute(ld, entry, &ber);
    int first = 1;
    while (attr) {
        struct berval** vals = ldap_get_values_len(ld, entry, attr);
        if (vals) {
            // escape attr name
            size_t al = strlen(attr);
            char* aesc = (char*)malloc(al * 2 + 1);
            if (!aesc) { ldap_value_free_len(vals); ldap_memfree(attr); break; }
            size_t j = 0;
            for (size_t i = 0; i < al; ++i) {
                char c = attr[i];
                if (c == '\\' || c == '\"') aesc[j++] = '\\';
                aesc[j++] = c;
            }
            aesc[j] = '\0';

            if (!first) APPEND_FMT("%s", ",");
            first = 0;
            APPEND_FMT("\"%s\":", aesc);

            if (vals[1] == NULL) {
                // single value
                struct berval* v = vals[0];
                // escape JSON
                size_t vl = (size_t)v->bv_len;
                char* vesc = (char*)malloc(vl * 2 + 1);
                if (!vesc) { free(aesc); ldap_value_free_len(vals); ldap_memfree(attr); break; }
                size_t k = 0;
                for (size_t i = 0; i < vl; ++i) {
                    char c = v->bv_val[i];
                    if (c == '\\' || c == '\"') vesc[k++] = '\\';
                    vesc[k++] = c;
                }
                vesc[k] = '\0';
                APPEND_FMT("\"%s\"", vesc);
                free(vesc);
            } else {
                APPEND_FMT("%s", "[");
                for (int i = 0; vals[i] != NULL; ++i) {
                    if (i) APPEND_FMT("%s", ",");
                    struct berval* v = vals[i];
                    size_t vl = (size_t)v->bv_len;
                    char* vesc = (char*)malloc(vl * 2 + 1);
                    if (!vesc) { free(aesc); ldap_value_free_len(vals); ldap_memfree(attr); break; }
                    size_t k = 0;
                    for (size_t t = 0; t < vl; ++t) {
                        char c = v->bv_val[t];
                        if (c == '\\' || c == '\"') vesc[k++] = '\\';
                        vesc[k++] = c;
                    }
                    vesc[k] = '\0';
                    APPEND_FMT("\"%s\"", vesc);
                    free(vesc);
                }
                APPEND_FMT("%s", "]");
            }
            free(aesc);
            ldap_value_free_len(vals);
        }
        ldap_memfree(attr);
        attr = ldap_next_attribute(ld, entry, ber);
    }

    APPEND_FMT("%s", "}}");

cleanup:
    if (dn) ldap_memfree(dn);
    if (dn_esc) free(dn_esc);
    if (ber) ber_free(ber, 0);
    ldap_msgfree(res);
    ldap_unbind_ext_s(ld, NULL, NULL);
    free(base_dn);
    free(user_esc);
    free(filter);
    if (!json) {
        char* err = (char*)malloc(32);
        if (err) strcpy(err, "{\"error\":\"alloc_failed\"}");
        return err;
    }
    return json;
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        char* res = search_user(argv[1], argv[2]);
        if (res) {
            printf("%s\n", res);
            free(res);
        }
        return 0;
    }
    const char* tests[5][2] = {
        {"example.com", "alice"},
        {"dc=example,dc=com", "bob"},
        {"EXAMPLE.ORG", "charlie"},
        {"invalid$$domain", "dave"},
        {"dc=local,dc=domain", "eve"}
    };
    for (int i = 0; i < 5; ++i) {
        printf("Input: dc=%s, user=%s\n", tests[i][0], tests[i][1]);
        char* res = search_user(tests[i][0], tests[i][1]);
        if (res) {
            printf("%s\n", res);
            free(res);
        }
    }
    return 0;
}