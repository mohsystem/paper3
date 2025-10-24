#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int is_valid_name(const char* name) {
    if (name == NULL) return 0;
    size_t len = strlen(name);
    if (len < 1 || len > 64) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.' || c == '/')) {
            return 0;
        }
    }
    return 1;
}

static char** jndi_lookup(const char** keys, const char** values, size_t kv_count,
                          const char** names, size_t names_count) {
    // Simple linear search registry; small for demo
    char** results = (char**)calloc(names_count, sizeof(char*));
    if (!results) return NULL;

    for (size_t i = 0; i < names_count; i++) {
        const char* n = names[i];
        if (!is_valid_name(n)) {
            results[i] = (char*)malloc(20);
            if (results[i]) snprintf(results[i], 20, "ERROR: invalid-name");
            continue;
        }
        int found = 0;
        for (size_t j = 0; j < kv_count; j++) {
            if (is_valid_name(keys[j]) && strcmp(keys[j], n) == 0) {
                const char* v = values[j] ? values[j] : "null";
                size_t vlen = strlen(v);
                if (vlen > 256) vlen = 256;
                results[i] = (char*)malloc(vlen + 1);
                if (results[i]) {
                    memcpy(results[i], v, vlen);
                    results[i][vlen] = '\0';
                }
                found = 1;
                break;
            }
        }
        if (!found) {
            results[i] = (char*)malloc(10);
            if (results[i]) snprintf(results[i], 10, "NOT_FOUND");
        }
    }
    return results;
}

static void print_case(const char* title, char** res, size_t count) {
    printf("%s: [", title);
    for (size_t i = 0; i < count; i++) {
        printf("%s", res[i] ? res[i] : "(null)");
        if (i + 1 < count) printf(", ");
    }
    printf("]\n");
}

static void free_results(char** res, size_t count) {
    if (!res) return;
    for (size_t i = 0; i < count; i++) free(res[i]);
    free(res);
}

int main(void) {
    const char* keys[] = {
        "config/db/url",
        "config/maxConnections",
        "service/cache",
        "feature/enabled",
        "metrics.endpoint"
    };
    const char* values[] = {
        "jdbc:postgresql://localhost:5432/app",
        "20",
        "SafeResource(name=CacheService,capacity=128)",
        "true",
        "http://localhost:8080/metrics"
    };
    size_t kv_count = sizeof(keys) / sizeof(keys[0]);

    // Test 1
    const char* n1[] = {"config/db/url", "config/maxConnections", "service/cache"};
    char** r1 = jndi_lookup(keys, values, kv_count, n1, 3);
    print_case("Test1", r1, 3);
    free_results(r1, 3);

    // Test 2
    const char* n2[] = {"config/missing", "feature/enabled", "service/unknown"};
    char** r2 = jndi_lookup(keys, values, kv_count, n2, 3);
    print_case("Test2", r2, 3);
    free_results(r2, 3);

    // Test 3
    const char* n3[] = {"bad name", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "valid.name-OK/1"};
    char** r3 = jndi_lookup(keys, values, kv_count, n3, 3);
    print_case("Test3", r3, 3);
    free_results(r3, 3);

    // Test 4 (override max connections by using alternate set)
    const char* keys2[] = {
        "config/db/url",
        "config/maxConnections",
        "service/cache",
        "feature/enabled",
        "metrics.endpoint"
    };
    const char* values2[] = {
        "jdbc:postgresql://localhost:5432/app",
        "50",
        "SafeResource(name=CacheService,capacity=128)",
        "true",
        "http://localhost:8080/metrics"
    };
    const char* n4[] = {"config/maxConnections", "metrics.endpoint"};
    char** r4 = jndi_lookup(keys2, values2, kv_count, n4, 2);
    print_case("Test4", r4, 2);
    free_results(r4, 2);

    // Test 5
    const char* n5[] = {"metrics.endpoint", "valid.name-OK/1"};
    char** r5 = jndi_lookup(keys, values, kv_count, n5, 2);
    print_case("Test5", r5, 2);
    free_results(r5, 2);

    return 0;
}