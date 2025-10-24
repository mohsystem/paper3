#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* read_file(const char* path, size_t* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[rd] = '\0';
    if (out_len) *out_len = rd;
    return buf;
}

static int write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    size_t len = strlen(content);
    if (fwrite(content, 1, len, f) != len) { fclose(f); return 0; }
    fclose(f);
    return 1;
}

static char* str_replace_all(const char* s, const char* from, const char* to) {
    size_t s_len = strlen(s), from_len = strlen(from), to_len = strlen(to);
    size_t count = 0;
    for (const char* p = s; (p = strstr(p, from)); p += from_len) count++;
    size_t new_len = s_len + count * (to_len - from_len);
    char* out = (char*)malloc(new_len + 1);
    if (!out) return NULL;
    char* w = out;
    const char* p = s;
    const char* q;
    while ((q = strstr(p, from))) {
        memcpy(w, p, (size_t)(q - p));
        w += (q - p);
        memcpy(w, to, to_len);
        w += to_len;
        p = q + from_len;
    }
    strcpy(w, p);
    return out;
}

static char* unescape_xml(const char* s) {
    char* r = str_replace_all(s, "&quot;", "\"");
    char* t = str_replace_all(r, "&apos;", "'");
    free(r);
    r = str_replace_all(t, "&amp;", "&");
    free(t);
    t = str_replace_all(r, "&lt;", "<");
    free(r);
    r = str_replace_all(t, "&gt;", ">");
    free(t);
    return r;
}

static char** queryById(const char* xmlPath, const char* idValue, int* outCount) {
    *outCount = 0;
    size_t len = 0;
    char* content = read_file(xmlPath, &len);
    if (!content) return NULL;
    if (strstr(content, "<!DOCTYPE")) {
        free(content);
        return NULL;
    }

    size_t i = 0;
    // Remove BOM
    if (len >= 3 && (unsigned char)content[0] == 0xEF && (unsigned char)content[1] == 0xBB && (unsigned char)content[2] == 0xBF) {
        i = 3;
    }
    // Skip XML declaration
    if (i + 5 <= len && strncmp(content + i, "<?xml", 5) == 0) {
        char* decl_end = strstr(content + i, "?>");
        if (!decl_end) { free(content); return NULL; }
        i = (size_t)(decl_end - content) + 2;
    }
    while (i < len && isspace((unsigned char)content[i])) i++;
    if (i >= len || content[i] != '<') { free(content); return NULL; }
    size_t name_start = i + 1;
    size_t j = name_start;
    while (j < len && (isalnum((unsigned char)content[j]) || content[j]=='_' || content[j]==':' || content[j]=='-')) j++;
    size_t name_len = j - name_start;
    if (name_len != 3 || strncmp(content + name_start, "tag", 3) != 0) {
        free(content);
        return NULL;
    }

    size_t k = j;
    char* idAttr = NULL;
    int selfClosing = 0;
    while (k < len) {
        if (content[k] == '>') { k++; break; }
        if (content[k] == '/' && k+1 < len && content[k+1] == '>') { selfClosing = 1; k += 2; break; }
        if (isspace((unsigned char)content[k])) { k++; continue; }
        size_t an_start = k;
        while (k < len && (isalnum((unsigned char)content[k]) || content[k]=='_' || content[k]==':' || content[k]=='-')) k++;
        size_t an_len = k - an_start;
        while (k < len && isspace((unsigned char)content[k])) k++;
        if (k < len && content[k] == '=') k++; else continue;
        while (k < len && isspace((unsigned char)content[k])) k++;
        if (k >= len || (content[k] != '"' && content[k] != '\'')) continue;
        char quote = content[k++];
        size_t vstart = k;
        while (k < len && content[k] != quote) k++;
        size_t vlen = k - vstart;
        if (k < len && content[k] == quote) k++;
        if (an_len == 2 && strncmp(content + an_start, "id", 2) == 0) {
            char* val = (char*)malloc(vlen + 1);
            if (!val) { free(content); return NULL; }
            memcpy(val, content + vstart, vlen);
            val[vlen] = '\0';
            char* unesc = unescape_xml(val);
            free(val);
            idAttr = unesc;
        }
    }

    char** results = NULL;
    if (idAttr && strcmp(idAttr, idValue) == 0) {
        size_t start_elem = i;
        size_t end_elem = 0;
        if (selfClosing) {
            end_elem = k;
        } else {
            const char* closeTag = "</tag>";
            char* pos = strstr(content + k, closeTag);
            if (!pos) {
                free(idAttr);
                free(content);
                return NULL;
            }
            end_elem = (size_t)(pos - content) + strlen(closeTag);
        }
        size_t elen = end_elem - i + (i - i); // just end_elem - start_elem
        elen = end_elem - (size_t)i + 0;
        elen = end_elem - (size_t)i;
        char* elem = (char*)malloc(elen + (size_t)(i - i) + 1);
        if (!elem) { free(idAttr); free(content); return NULL; }
        memcpy(elem, content + i, elen);
        elem[elen] = '\0';

        results = (char**)malloc(sizeof(char*));
        if (!results) { free(elem); free(idAttr); free(content); return NULL; }
        results[0] = elem;
        *outCount = 1;
    }
    if (idAttr) free(idAttr);
    free(content);
    return results;
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        int count = 0;
        char** res = queryById(argv[1], argv[2], &count);
        if (res) {
            for (int i = 0; i < count; ++i) {
                printf("%s\n", res[i]);
                free(res[i]);
            }
            free(res);
        }
        return 0;
    }

    const char* files[5] = {
        "sample97_1.xml",
        "sample97_2.xml",
        "sample97_3.xml",
        "sample97_4.xml",
        "sample97_5.xml"
    };
    const char* contents[5] = {
        "<tag id=\"1\">Alpha</tag>",
        "<tag id=\"abc\">Bravo</tag>",
        "<tag id=\"he said &quot;hi&quot; and 'yo'\">Charlie</tag>",
        "<tag id=\"no-match\">Delta</tag>",
        "<tag id=\"5\"><child>Echo</child></tag>"
    };
    for (int i = 0; i < 5; ++i) write_file(files[i], contents[i]);

    int c1=0,c2=0,c3=0,c4=0,c5=0;
    char** r1 = queryById(files[0], "1", &c1);
    char** r2 = queryById(files[1], "abc", &c2);
    char** r3 = queryById(files[2], "he said \"hi\" and 'yo'", &c3);
    char** r4 = queryById(files[3], "x", &c4);
    char** r5 = queryById(files[4], "5", &c5);

    printf("Test1: [");
    for (int i=0;i<c1;i++){ if(i) printf(", "); printf("%s", r1[i]); }
    printf("]\n");
    printf("Test2: [");
    for (int i=0;i<c2;i++){ if(i) printf(", "); printf("%s", r2[i]); }
    printf("]\n");
    printf("Test3: [");
    for (int i=0;i<c3;i++){ if(i) printf(", "); printf("%s", r3[i]); }
    printf("]\n");
    printf("Test4: [");
    for (int i=0;i<c4;i++){ if(i) printf(", "); printf("%s", r4[i]); }
    printf("]\n");
    printf("Test5: [");
    for (int i=0;i<c5;i++){ if(i) printf(", "); printf("%s", r5[i]); }
    printf("]\n");

    for (int i=0;i<c1;i++) free(r1[i]); free(r1);
    for (int i=0;i<c2;i++) free(r2[i]); free(r2);
    for (int i=0;i<c3;i++) free(r3[i]); free(r3);
    for (int i=0;i<c4;i++) free(r4[i]); free(r4);
    for (int i=0;i<c5;i++) free(r5[i]); free(r5);
    return 0;
}