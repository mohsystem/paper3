#include <stdio.h>
#include <ctype.h>

const char* get_json_root_type(const char* s) {
    if (!s) return "invalid";
    const char* p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (!*p) return "invalid";
    char c = *p;
    if (c == '{') return "object";
    if (c == '[') return "array";
    if (c == '"') return "string";
    if (c == 't' || c == 'f') return "boolean";
    if (c == 'n') return "null";
    if (c == '-' || (c >= '0' && c <= '9')) return "number";
    return "invalid";
}

int main() {
    const char* tests[5] = {
        "{\"name\":\"Alice\",\"age\":30}",
        "[{\"id\":1},{\"id\":2}]",
        "\"sample\"",
        "-42.0e10",
        "true"
    };
    for (int i = 0; i < 5; ++i) {
        printf("%s\n", get_json_root_type(tests[i]));
    }
    return 0;
}