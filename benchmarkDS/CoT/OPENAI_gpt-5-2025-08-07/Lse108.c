#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Secure helper: validate username to prevent command injection
static int is_valid_username(const char* u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len == 0 || len > 32) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)u[i];
        if (!(isalnum(ch) || ch == '.' || ch == '_' || ch == '-')) return 0;
    }
    return 1;
}

// Core function: executes "getent passwd <username>" via system(), directs output to FILE* output,
// and returns a newly allocated string containing the written content if the stream is seekable.
// Caller must free the returned string. Returns NULL if nothing captured or on error.
char* get_user_info_to_output(const char* username, FILE* output) {
    if (!output || !is_valid_username(username)) {
        return NULL;
    }

    int fd = fileno(output);
    if (fd < 0) return NULL;

    long pos_before = ftell(output); // may fail (-1) if non-seekable
    fflush(output);

    // Build command safely; username validated to avoid injection
    char cmd[256];
    if (strlen(username) + 64 >= sizeof(cmd)) return NULL;
    snprintf(cmd, sizeof(cmd), "getent passwd %s 1>&%d 2>&%d", username, fd, fd);

    int rc = system(cmd);
    (void)rc; // even on failure, try to read any output if present

    fflush(output);

    if (pos_before >= 0) {
        if (fseek(output, 0, SEEK_END) == 0) {
            long end_pos = ftell(output);
            if (end_pos >= 0 && end_pos >= pos_before) {
                long sz = end_pos - pos_before;
                if (sz < 0) return NULL;
                if (fseek(output, pos_before, SEEK_SET) != 0) return NULL;

                char* buf = (char*)malloc((size_t)sz + 1);
                if (!buf) return NULL;
                size_t readn = fread(buf, 1, (size_t)sz, output);
                buf[readn] = '\0';

                // restore to end
                fseek(output, end_pos, SEEK_SET);
                return buf;
            }
        }
    }
    return NULL;
}

// Test harness with 5 test cases
int main(void) {
    const char* tests[5] = {
        "root",
        "nobody",
        "daemon",
        "doesnotexistuser",
        "root;echo HACKED"
    };

    for (int i = 0; i < 5; ++i) {
        FILE* tmp = tmpfile();
        if (!tmp) {
            perror("tmpfile");
            return 1;
        }
        char* out = get_user_info_to_output(tests[i], tmp);
        printf("Test username='%s' -> Output: %s\n", tests[i], (out ? out : ""));
        if (out) free(out);
        fclose(tmp);
    }
    return 0;
}