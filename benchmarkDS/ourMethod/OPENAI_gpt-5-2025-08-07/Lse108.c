#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

static int validate_username(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len == 0 || len > 32) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)u[i];
        if (isalnum(c)) continue;
        if (c == '.' || c == '_' || c == '-') continue;
        return 0;
    }
    return 1;
}

// Executes "getent passwd <username>" via system() and directs stdout to 'output'.
// Returns the child's exit code (0 if found) or negative on validation/error.
int getent_to_fp_c(const char* username, FILE* output) {
    if (!output) return -1;
    if (!validate_username(username)) return -1;

    int fd = fileno(output);
    if (fd < 0) return -1;

    // Ensure the file descriptor is inherited by the child shell.
    int old_flags = fcntl(fd, F_GETFD);
    if (old_flags == -1) return -1;
    int need_restore = 0;
    if (old_flags & FD_CLOEXEC) {
        if (fcntl(fd, F_SETFD, old_flags & ~FD_CLOEXEC) == -1) {
            return -1;
        }
        need_restore = 1;
    }

    char cmd[512];
    int n = snprintf(cmd, sizeof(cmd), "getent passwd %s 1>&%d 2>/dev/null", username, fd);
    if (n < 0 || (size_t)n >= sizeof(cmd)) {
        if (need_restore) (void)fcntl(fd, F_SETFD, old_flags);
        return -1;
    }

    int status = system(cmd);

    if (need_restore) {
        (void)fcntl(fd, F_SETFD, old_flags);
    }

    if (status == -1) return -1;
#ifdef WIFEXITED
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
#endif
    return -1;
}

int main(void) {
    const char* tests[5] = {
        "root",
        "nobody",
        "daemon",
        "thisuserdoesnotexist",
        "bad;injection" /* invalid by validation */
    };

    for (int i = 0; i < 5; ++i) {
        // Secure temporary file
        char tmpl[] = "/tmp/getent_c_XXXXXX";
        int tfd = mkstemp(tmpl);
        if (tfd == -1) {
            perror("mkstemp");
            return 1;
        }
        // Unlink immediately; fd remains valid
        unlink(tmpl);
        FILE* f = fdopen(tfd, "w+");
        if (!f) {
            perror("fdopen");
            close(tfd);
            return 1;
        }

        int rc = getent_to_fp_c(tests[i], f);
        fflush(f);
        fseek(f, 0, SEEK_SET);

        printf("User: %s -> rc=%d\n", tests[i], rc);
        char buf[4096];
        size_t r;
        int any = 0;
        while ((r = fread(buf, 1, sizeof(buf), f)) > 0) {
            any = 1;
            if (fwrite(buf, 1, r, stdout) != r) {
                // Ignore write error to stdout for demonstration purposes
                break;
            }
        }
        if (!any) {
            printf("(no output)\n");
        }
        printf("----\n");
        fclose(f);
    }
    return 0;
}