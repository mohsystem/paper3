#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_READ_BYTES 2048
#define MAX_NAME_CHARS 100

static int is_allowed_char(unsigned char c) {
    return (isalnum(c) || c == ' ' || c == '_' || c == '-' || c == '.' || c == '\'');
}

char* http_hello_page(FILE* request, FILE* response) {
    if (request == NULL || response == NULL) {
        return NULL;
    }

    // Read up to MAX_READ_BYTES
    char rawBuf[MAX_READ_BYTES];
    size_t total = 0;
    while (total < MAX_READ_BYTES) {
        size_t n = fread(rawBuf + total, 1, MAX_READ_BYTES - total, request);
        if (n == 0) break;
        total += n;
    }

    // Sanitize: remove CR/LF, trim, allow only allowed chars, limit length
    char temp[MAX_READ_BYTES + 1];
    size_t tlen = 0;
    for (size_t i = 0; i < total; i++) {
        unsigned char c = (unsigned char)rawBuf[i];
        if (c == '\r' || c == '\n') continue;
        temp[tlen++] = (char)c;
    }
    temp[tlen] = '\0';

    // Trim spaces
    size_t start = 0;
    while (start < tlen && temp[start] == ' ') start++;
    size_t end = tlen;
    while (end > start && temp[end - 1] == ' ') end--;

    char name[MAX_NAME_CHARS + 1];
    size_t nlen = 0;
    for (size_t i = start; i < end && nlen < MAX_NAME_CHARS; i++) {
        unsigned char c = (unsigned char)temp[i];
        if (is_allowed_char(c)) {
            name[nlen++] = (char)c;
        }
    }
    // Trim again
    size_t nstart = 0;
    while (nstart < nlen && name[nstart] == ' ') nstart++;
    size_t nend = nlen;
    while (nend > nstart && name[nend - 1] == ' ') nend--;

    char finalName[MAX_NAME_CHARS + 1];
    size_t finalLen = 0;
    if (nend > nstart) {
        finalLen = nend - nstart;
        if (finalLen > MAX_NAME_CHARS) finalLen = MAX_NAME_CHARS;
        memcpy(finalName, name + nstart, finalLen);
        finalName[finalLen] = '\0';
    } else {
        strcpy(finalName, "Guest");
        finalLen = strlen(finalName);
    }

    // Build message
    const char* prefix = "Hello ";
    size_t msgLen = strlen(prefix) + finalLen;
    char* message = (char*)malloc(msgLen + 1);
    if (!message) {
        return NULL;
    }
    memcpy(message, prefix, strlen(prefix));
    memcpy(message + strlen(prefix), finalName, finalLen);
    message[msgLen] = '\0';

    // Write to response
    fwrite(message, 1, msgLen, response);
    fflush(response);

    return message;
}

// 5 test cases
int main(void) {
    const char* tests[5] = {
        "Alice",
        "  Bob  ",
        "Eve\r\nSet-Cookie: hack",
        "Zeljko", /* ASCII fallback for unicode example */
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" /* 100 As */ "EXTRA"
    };

    for (int i = 0; i < 5; i++) {
        FILE* req = tmpfile();
        FILE* res = tmpfile();
        if (!req || !res) {
            fprintf(stderr, "Failed to create tmpfile\n");
            if (req) fclose(req);
            if (res) fclose(res);
            continue;
        }

        fputs(tests[i], req);
        fflush(req);
        rewind(req);

        char* msg = http_hello_page(req, res);

        // Read what was written to response
        fflush(res);
        rewind(res);
        char outBuf[256];
        size_t read = fread(outBuf, 1, sizeof(outBuf) - 1, res);
        outBuf[read] = '\0';

        printf("Returned: %s\n", msg ? msg : "(null)");
        printf("Written : %s\n", outBuf);
        printf("---\n");

        free(msg);
        fclose(req);
        fclose(res);
    }

    return 0;
}