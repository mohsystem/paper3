#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int is_regular_file_no_symlink(const char* path) {
    struct stat st;
    if (lstat(path, &st) != 0) return 0;
    if (S_ISLNK(st.st_mode)) return 0;
    if (!S_ISREG(st.st_mode)) return 0;
    return 1;
}

static char* read_file(const char* path, size_t* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[rd] = '\0';
    if (out_len) *out_len = rd;
    return buf;
}

static void skip_spaces_c(const char* s, size_t len, size_t* i) {
    while (*i < len) {
        char c = s[*i];
        if (c==' '||c=='\t'||c=='\n'||c=='\r') (*i)++;
        else break;
    }
}

static int parse_id_from_header_c(const char* header, size_t hlen, char** outId) {
    size_t start = 0;
    const char* tag = "<tag";
    size_t taglen = 4;
    int found = 0;
    for (size_t k=0; k+taglen<=hlen; ++k) {
        if (memcmp(header+k, tag, taglen)==0) { start = k+taglen; found = 1; break; }
    }
    if (!found) return 0;
    size_t i = start;
    while (i < hlen) {
        skip_spaces_c(header, hlen, &i);
        if (i >= hlen) break;
        if (header[i] == '>' || (header[i] == '/' && i+1<hlen && header[i+1] == '>')) break;

        size_t nameStart = i;
        while (i < hlen) {
            char c = header[i];
            if (c=='='||c==' '||c=='\t'||c=='\n'||c=='\r'||c=='/'||c=='>') break;
            i++;
        }
        size_t nameLen = (i > nameStart) ? (i - nameStart) : 0;
        char namebuf[32];
        size_t copyLen = nameLen < sizeof(namebuf)-1 ? nameLen : sizeof(namebuf)-1;
        memcpy(namebuf, header + nameStart, copyLen);
        namebuf[copyLen] = '\0';

        skip_spaces_c(header, hlen, &i);
        if (i >= hlen || header[i] != '=') {
            while (i < hlen && header[i] != '>' && header[i] != ' ') i++;
            continue;
        }
        i++; // '='
        skip_spaces_c(header, hlen, &i);
        if (i >= hlen) break;
        char quote = header[i];
        if (quote != '"' && quote != '\'') {
            while (i < hlen && header[i] != '>' && header[i] != ' ') i++;
            continue;
        }
        i++;
        size_t valStart = i;
        size_t valEnd = valStart;
        while (valEnd < hlen && header[valEnd] != quote) valEnd++;
        if (valEnd >= hlen) break;
        size_t valLen = valEnd - valStart;

        if (strcmp(namebuf, "id") == 0) {
            char* val = (char*)malloc(valLen + 1);
            if (!val) return 0;
            memcpy(val, header + valStart, valLen);
            val[valLen] = '\0';
            *outId = val;
            return 1;
        }
        i = valEnd + 1;
    }
    return 0;
}

char** query_by_id(const char* xmlFilePath, const char* idValue, size_t* outCount) {
    if (!xmlFilePath || !idValue || !outCount) return NULL;
    *outCount = 0;
    if (!is_regular_file_no_symlink(xmlFilePath)) {
        return NULL;
    }
    size_t xmlLen = 0;
    char* xml = read_file(xmlFilePath, &xmlLen);
    if (!xml) return NULL;

    size_t cap = 4;
    char** results = (char**)malloc(cap * sizeof(char*));
    if (!results) { free(xml); return NULL; }

    size_t pos = 0;
    const char* openTag = "<tag";
    const char* closeTag = "</tag>";
    while (1) {
        char* startPtr = strstr(xml + pos, openTag);
        if (!startPtr) break;
        size_t start = (size_t)(startPtr - xml);
        char* gtPtr = strchr(startPtr, '>');
        if (!gtPtr) break;
        size_t gt = (size_t)(gtPtr - xml);
        int selfClosing = (gt > start && xml[gt - 1] == '/');

        size_t headerLen = gt - start + 1;
        char* header = (char*)malloc(headerLen + 1);
        if (!header) break;
        memcpy(header, xml + start, headerLen);
        header[headerLen] = '\0';

        char* foundId = NULL;
        int hasId = parse_id_from_header_c(header, headerLen, &foundId);
        if (!hasId) {
            free(header);
            pos = gt + 1;
            continue;
        }
        int match = (strcmp(foundId, idValue) == 0);
        free(foundId);

        if (!match) {
            free(header);
            pos = gt + 1;
            continue;
        }

        if (selfClosing) {
            if (*outCount >= cap) {
                cap *= 2;
                char** tmp = (char**)realloc(results, cap * sizeof(char*));
                if (!tmp) { free(header); break; }
                results = tmp;
            }
            results[*outCount] = header; // already allocated
            (*outCount)++;
            pos = gt + 1;
            continue;
        }

        // Find matching closing tag considering nested <tag>
        size_t searchPos = gt + 1;
        int depth = 1;
        while (depth > 0) {
            char* nextOpenPtr = strstr(xml + searchPos, openTag);
            char* nextClosePtr = strstr(xml + searchPos, closeTag);
            if (!nextClosePtr) break;
            size_t nextOpen = nextOpenPtr ? (size_t)(nextOpenPtr - xml) : (size_t)(-1);
            size_t nextClose = (size_t)(nextClosePtr - xml);

            if (nextOpenPtr && nextOpen < nextClose) {
                char* nextGtPtr = strchr(xml + nextOpen, '>');
                if (!nextGtPtr) break;
                searchPos = (size_t)(nextGtPtr - xml) + 1;
                depth++;
            } else {
                depth--;
                searchPos = nextClose + strlen(closeTag);
                char* endGt = strchr(xml + searchPos, '>');
                if (!endGt) break;
                searchPos = (size_t)(endGt - xml) + 1;
            }
        }

        if (depth == 0) {
            size_t elemLen = searchPos - start;
            char* element = (char*)malloc(elemLen + 1);
            if (!element) { free(header); break; }
            memcpy(element, xml + start, elemLen);
            element[elemLen] = '\0';
            free(header);

            if (*outCount >= cap) {
                cap *= 2;
                char** tmp = (char**)realloc(results, cap * sizeof(char*));
                if (!tmp) { free(element); break; }
                results = tmp;
            }
            results[*outCount] = element;
            (*outCount)++;
            pos = searchPos;
        } else {
            free(header);
            break;
        }
    }

    free(xml);
    return results;
}

int main(int argc, char** argv) {
    if (argc >= 3) {
        size_t count = 0;
        char** res = query_by_id(argv[1], argv[2], &count);
        if (res) {
            for (size_t i = 0; i < count; ++i) {
                puts(res[i]);
                free(res[i]);
            }
            free(res);
        }
        return 0;
    }

    const char* sample =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<root>\n"
        "  <tag id=\"1\">Alpha</tag>\n"
        "  <tag id=\"2\"><inner>Beta</inner></tag>\n"
        "  <tag id=\"3\">Gamma</tag>\n"
        "  <tag id=\"42\">FortyTwo</tag>\n"
        "  <tag id=\"x\">X-Value</tag>\n"
        "</root>\n";
    const char* path = "sample97_c.xml";
    FILE* f = fopen(path, "wb");
    if (f) {
        fwrite(sample, 1, strlen(sample), f);
        fclose(f);
    }

    const char* testIds[5] = { "1", "2", "3", "42", "nope" };
    for (int t = 0; t < 5; ++t) {
        size_t count = 0;
        char** res = query_by_id(path, testIds[t], &count);
        printf("ID=%s count=%zu\n", testIds[t], count);
        if (res) {
            for (size_t i = 0; i < count; ++i) {
                puts(res[i]);
                free(res[i]);
            }
            free(res);
        }
    }
    return 0;
}