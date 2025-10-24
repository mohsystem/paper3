/* Chain-of-Through Process in Code (C)
   1) Problem understanding:
      - Implement a function to change a user's password using chpasswd securely, then drop privileges.
   2) Security requirements:
      - Strict input validation, avoid shell, pass secret via stdin, minimal env, zero out secrets.
   3) Secure coding generation: careful fork/exec/pipe usage, error checks.
   4) Code review: no secret printing, handle errors, close fds, verify privilege drop.
   5) Secure code output: final function and 5 dry-run test cases.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

static void secure_memzero(void* p, size_t n) {
    volatile unsigned char* v = (volatile unsigned char*)p;
    while (n--) *v++ = 0;
}

static int path_exists_exec(const char* p) {
    return p && access(p, X_OK) == 0;
}

static const char* find_chpasswd() {
    static const char* cands[] = {
        "/usr/sbin/chpasswd",
        "/usr/bin/chpasswd",
        "/sbin/chpasswd",
        "/bin/chpasswd",
        NULL
    };
    for (int i = 0; cands[i]; ++i) {
        if (path_exists_exec(cands[i])) return cands[i];
    }
    return NULL;
}

static int is_valid_username(const char* u) {
    if (!u) return 0;
    size_t n = strlen(u);
    if (n < 1 || n > 32) return 0;
    unsigned char c0 = (unsigned char)u[0];
    if (!(c0 == '_' || (c0 >= 'a' && c0 <= 'z'))) return 0;
    for (size_t i = 1; i < n; ++i) {
        unsigned char c = (unsigned char)u[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int is_valid_password(const char* pw) {
    if (!pw) return 0;
    size_t n = strlen(pw);
    if (n < 8 || n > 128) return 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)pw[i];
        if (c == ':' || c < 0x20 || c > 0x7E) return 0;
    }
    return 1;
}

static int run_chpasswd(const char* user, const char* pw) {
    const char* bin = find_chpasswd();
    if (!bin) return -2;

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        return -1;
    }
    if (pid == 0) {
        // Child
        close(pipefd[1]);
        if (dup2(pipefd[0], STDIN_FILENO) < 0) _exit(127);
        close(pipefd[0]);
        for (int fd = 3; fd < 256; ++fd) close(fd);
        char const* envp[] = {
            "PATH=/usr/sbin:/usr/bin:/bin",
            "LANG=C",
            NULL
        };
        char const* argv[] = { bin, NULL };
        execve(bin, (char* const*)argv, (char* const*)envp);
        _exit(127);
    }

    // Parent
    close(pipefd[0]);
    // Build payload
    size_t ulen = strlen(user);
    size_t plen = strlen(pw);
    size_t total = ulen + 1 + plen + 1; // user:pw

    char* buf = (char*)calloc(total + 1, 1);
    if (!buf) {
        close(pipefd[1]);
        return -1;
    }
    memcpy(buf, user, ulen);
    buf[ulen] = ':';
    memcpy(buf + ulen + 1, pw, plen);
    buf[total - 1] = '\n';

    ssize_t wn = write(pipefd[1], buf, total);
    (void)wn;
    secure_memzero(buf, total + 1);
    free(buf);
    close(pipefd[1]);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) return -1;
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return -1;
}

static int drop_privileges(uid_t to_uid, gid_t to_gid) {
    if (geteuid() != 0) return 1;
    // clear supplementary groups
    if (setgroups(0, NULL) != 0) {
        // continue anyway
    }
#if defined(__linux__) && defined(HAVE_SETRESGID)
    if (setresgid(to_gid, to_gid, to_gid) != 0) return 0;
#else
    if (setgid(to_gid) != 0) return 0;
#endif

#if defined(__linux__) && defined(HAVE_SETRESUID)
    if (setresuid(to_uid, to_uid, to_uid) != 0) return 0;
#else
    if (setuid(to_uid) != 0) return 0;
#endif

    return (geteuid() == to_uid && getegid() == to_gid) ? 1 : 0;
}

/* Returns 0 on success, negative on validation errors, positive for external command exit status */
int change_password_and_drop_c(const char* username, const char* new_password, int dry_run, uid_t drop_uid, gid_t drop_gid) {
    if (!is_valid_username(username)) return -1;
    if (!is_valid_password(new_password)) return -2;

    int rc = 0;
    if (!dry_run && geteuid() == 0) {
        rc = run_chpasswd(username, new_password);
        if (rc != 0) return (rc == -2) ? -3 : rc;
    }
    uid_t target_uid = (drop_uid == (uid_t)-1) ? getuid() : drop_uid;
    gid_t target_gid = (drop_gid == (gid_t)-1) ? getgid() : drop_gid;
    (void)drop_privileges(target_uid, target_gid);
    return rc;
}

int main(void) {
    // 5 test cases (dry-run)
    struct { const char* u; const char* p; } cases[5] = {
        {"alice", "S3curePwd1!"},
        {"bob", "AnotherPwd2#"},
        {"charlie", "ThirdPwd3$"},
        {"dave-user", "FourthPwd4%"},
        {"eve_user", "FifthPwd5^"}
    };
    for (int i = 0; i < 5; ++i) {
        int rc = change_password_and_drop_c(cases[i].u, cases[i].p, 1, (uid_t)-1, (gid_t)-1);
        printf("Test %d user=%s rc=%d\n", i+1, cases[i].u, rc);
    }
    return 0;
}