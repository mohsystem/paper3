#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ldap.h>

static char* dc_to_base_dn(const char* dc) {
    if (!dc) {
        char* empty = (char*)malloc(1);
        empty[0] = '\0';
        return empty;
    }
    const char* p = dc;
    while (isspace((unsigned char)*p)) p++;
    size_t len = strlen(p);
    while (len > 0 && isspace((unsigned char)p[len-1])) len--;
    char* t = (char*)malloc(len + 1);
    memcpy(t, p, len);
    t[len] = '\0';

    char* lower = (char*)malloc(len + 1);
    for (size_t i = 0; i < len; ++i) lower[i] = (char)tolower((unsigned char)t[i]);
    lower[len] = '\0';

    if (strstr(lower, "dc=") != NULL) {
        free(lower);
        return t; // already formatted
    }
    free(lower);

    // replace '.' with ',' then split by ','
    for (size_t i = 0; i < len; ++i) if (t[i] == '.') t[i] = ',';

    // estimate size: worst-case add "dc=" + commas
    size_t out_cap = len * 4 + 8;
    char* out = (char*)malloc(out_cap);
    out[0] = '\0';

    char* saveptr = NULL;
    char* token = strtok_r(t, ",", &saveptr);
    int first = 1;
    while (token) {
        while (*token && isspace((unsigned char)*token)) token++;
        char* end = token + strlen(token);
        while (end > token && isspace((unsigned char)end[-1])) end--;
        *end = '\0';
        if (*token) {
            if (!first) strcat(out, ",");
            first = 0;
            strcat(out, "dc=");
            strcat(out, token);
        }
        token = strtok_r(NULL, ",", &saveptr);
    }
    free(t);
    return out;
}

static char* ldap_escape(const char* s) {
    if (!s) {
        char* empty = (char*)malloc(1);
        empty[0] = '\0';
        return empty;
    }
    size_t len = strlen(s);
    // Worst expansion: every char becomes 3 chars (e.g., \2a), allocate 3x + 1
    char* out = (char*)malloc(len * 3 + 1);
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c == '\\') { memcpy(out + j, "\\5c", 3); j += 3; }
        else if (c == '*') { memcpy(out + j, "\\2a", 3); j += 3; }
        else if (c == '(') { memcpy(out + j, "\\28", 3); j += 3; }
        else if (c == ')') { memcpy(out + j, "\\29", 3); j += 3; }
        else if (c == '\0') { memcpy(out + j, "\\00", 3); j += 3; }
        else { out[j++] = (char)c; }
    }
    out[j] = '\0';
    return out;
}

char* query_ldap(const char* dc, const char* username) {
    char* baseDN = dc_to_base_dn(dc);
    char* escUser = ldap_escape(username);

    size_t filter_len = strlen(escUser) * 3 + 64;
    char* filter = (char*)malloc(filter_len);
    snprintf(filter, filter_len, "(|(uid=%s)(sAMAccountName=%s)(cn=%s))", escUser, escUser, escUser);

    LDAP* ld = NULL;
    int rc = ldap_initialize(&ld, "ldap://localhost:389");
    if (rc != LDAP_SUCCESS || ld == NULL) {
        size_t sz = 256;
        char* err = (char*)malloc(sz);
        snprintf(err, sz, "ERROR: ldap_initialize failed: %s", ldap_err2string(rc));
        free(baseDN); free(escUser); free(filter);
        return err;
    }

    int version = LDAP_VERSION3;
    ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);

    rc = ldap_simple_bind_s(ld, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        size_t sz = 256;
        char* err = (char*)malloc(sz);
        snprintf(err, sz, "ERROR: bind failed: %s", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        free(baseDN); free(escUser); free(filter);
        return err;
    }

    LDAPMessage* res = NULL;
    struct timeval tv;
    tv.tv_sec = 10; tv.tv_usec = 0;
    rc = ldap_search_ext_s(ld, baseDN, LDAP_SCOPE_SUBTREE, filter, NULL, 0, NULL, NULL, &tv, 0, &res);
    if (rc != LDAP_SUCCESS) {
        size_t sz = 256;
        char* err = (char*)malloc(sz);
        snprintf(err, sz, "ERROR: search failed: %s", ldap_err2string(rc));
        if (res) ldap_msgfree(res);
        ldap_unbind_ext_s(ld, NULL, NULL);
        free(baseDN); free(escUser); free(filter);
        return err;
    }

    LDAPMessage* entry = ldap_first_entry(ld, res);
    if (!entry) {
        char* msg = (char*)malloc(10);
        strcpy(msg, "NOT_FOUND");
        ldap_msgfree(res);
        ldap_unbind_ext_s(ld, NULL, NULL);
        free(baseDN); free(escUser); free(filter);
        return msg;
    }

    char* dn = ldap_get_dn(ld, entry);

    // Prepare large buffer
    size_t buf_cap = 65536;
    char* out = (char*)malloc(buf_cap);
    size_t off = 0;
    off += snprintf(out + off, buf_cap - off, "{dn:\"%s\",attributes:{", dn ? dn : "");

    BerElement* ber = NULL;
    char* attr = ldap_first_attribute(ld, entry, &ber);
    int firstAttr = 1;
    while (attr != NULL) {
        struct berval** vals = ldap_get_values_len(ld, entry, attr);
        int count = vals ? ldap_count_values_len(vals) : 0;

        if (!firstAttr) off += snprintf(out + off, buf_cap - off, ",");
        firstAttr = 0;
        off += snprintf(out + off, buf_cap - off, "\"%s\":[", attr);
        for (int i = 0; i < count; ++i) {
            if (i) off += snprintf(out + off, buf_cap - off, ",");
            // Copy value, replace quotes
            char* tmp = (char*)malloc(vals[i]->bv_len + 1);
            memcpy(tmp, vals[i]->bv_val, vals[i]->bv_len);
            tmp[vals[i]->bv_len] = '\0';
            for (size_t k = 0; k < vals[i]->bv_len; ++k) if (tmp[k] == '"') tmp[k] = '\'';
            off += snprintf(out + off, buf_cap - off, "\"%s\"", tmp);
            free(tmp);
        }
        off += snprintf(out + off, buf_cap - off, "]");
        if (vals) ldap_value_free_len(vals);
        ldap_memfree(attr);
        attr = ldap_next_attribute(ld, entry, ber);
    }
    if (ber) ber_free(ber, 0);

    off += snprintf(out + off, buf_cap - off, "}}");

    if (dn) ldap_memfree(dn);
    ldap_msgfree(res);
    ldap_unbind_ext_s(ld, NULL, NULL);

    free(baseDN); free(escUser); free(filter);
    return out;
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        char* res = query_ldap(argv[1], argv[2]);
        printf("%s\n", res);
        free(res);
        return 0;
    }
    const char* tests[5][2] = {
        {"example.com", "alice"},
        {"example.com", "bob"},
        {"dc=example,dc=com", "carol"},
        {"example,org", "dave"},
        {"test.local", "eve"}
    };
    for (int i = 0; i < 5; ++i) {
        printf("DC=%s USER=%s\n", tests[i][0], tests[i][1]);
        char* res = query_ldap(tests[i][0], tests[i][1]);
        printf("%s\n", res);
        free(res);
    }
    return 0;
}