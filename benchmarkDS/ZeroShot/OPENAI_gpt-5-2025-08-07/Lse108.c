#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int is_valid_username(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len == 0 || len > 32) return 0;
    unsigned char c0 = (unsigned char)u[0];
    if (!(isalnum(c0) || c0 == '_')) return 0;
    for (size_t i = 1; i < len; ++i) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.')) return 0;
    }
    return 1;
}

/*
 Executes 'getent passwd <username>' via system() and redirects stdout/stderr
 to the provided FILE* output. Returns the exit status from system() or 400 on error.
*/
int get_user_info_system(const char* username, FILE* output) {
    if (!output) return 400;
    if (!is_valid_username(username)) return 400;

    int fd = fileno(output);
    if (fd < 0) return 400;

    fflush(output);

    // Constrain PATH and use absolute path to getent where possible.
    setenv("PATH", "/usr/bin:/bin", 1);

    char cmd[256];
    int n = snprintf(cmd, sizeof(cmd),
                     "/usr/bin/getent passwd %s 1>&%d 2>&1",
                     username, fd);
    if (n < 0 || (size_t)n >= sizeof(cmd)) {
        return 400;
    }

    int rc = system(cmd);
    return rc;
}

int main(void) {
    const char* tests[5] = {"root", "nobody", "daemon", "thisuserdoesnotexist12345", "bin"};

    for (int i = 0; i < 5; ++i) {
        const char* user = tests[i];
        FILE* f = tmpfile();
        if (!f) {
            perror("tmpfile");
            continue;
        }
        int rc = get_user_info_system(user, f);
        printf("=== C Test for '%s' (rc=%d) ===\n", user, rc);
        rewind(f);
        char buf[512];
        int any = 0;
        while (fgets(buf, sizeof(buf), f)) {
            any = 1;
            fputs(buf, stdout);
        }
        if (!any) {
            printf("(no output)\n");
        }
        fclose(f);
    }
    return 0;
}