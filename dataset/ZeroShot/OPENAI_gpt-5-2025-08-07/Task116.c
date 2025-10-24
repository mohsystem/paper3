#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <errno.h>
#include <stdlib.h>

static int is_valid_username(const char* u) {
    if (!u) return 0;
    regex_t re;
    if (regcomp(&re, "^[a-z_][a-z0-9_-]{0,31}$", REG_EXTENDED | REG_NOSUB) != 0) {
        return 0;
    }
    int ok = regexec(&re, u, 0, NULL, 0) == 0;
    regfree(&re);
    if (!ok) return 0;
    errno = 0;
    struct passwd* pw = getpwnam(u);
    if (!pw) return 0;
    return 1;
}

static int is_strong_password(const char* p) {
    if (!p) return 0;
    size_t n = strlen(p);
    if (n < 8 || n > 128) return 0;
    return 1;
}

static void secure_memzero(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) *p++ = 0;
}

static const char* find_chpasswd_path() {
    static const char* candidates[] = {
        "/usr/sbin/chpasswd",
        "/sbin/chpasswd",
        "/usr/bin/chpasswd",
        "/bin/chpasswd",
        NULL
    };
    struct stat st;
    for (int i = 0; candidates[i]; i++) {
        if (stat(candidates[i], &st) == 0 && (st.st_mode & S_IXUSR)) {
            return candidates[i];
        }
    }
    return NULL;
}

static int drop_privileges_to(const char* user) {
    if (!user || !*user) user = "nobody";
    struct passwd* pw = getpwnam(user);
    if (!pw) return 0;

#if defined(__linux__)
    if (initgroups(pw->pw_name, pw->pw_gid) != 0) return 0;
#endif
    if (setgid(pw->pw_gid) != 0) return 0;
#if defined(__linux__)
    if (setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid) != 0) return 0;
#else
    if (setuid(pw->pw_uid) != 0) return 0;
#endif
    if (geteuid() == 0 || getuid() == 0) return 0;
    return 1;
}

int change_password_and_drop(const char* username, const char* new_password, const char* drop_user, int dry_run) {
    if (!is_valid_username(username)) return 0;
    if (!is_strong_password(new_password)) return 0;

    if (!dry_run) {
        if (geteuid() != 0) return 0;
        const char* chpasswd = find_chpasswd_path();
        if (!chpasswd) return 0;

        int pipefd[2];
#ifdef O_CLOEXEC
        if (pipe2(pipefd, O_CLOEXEC) != 0) return 0;
#else
        if (pipe(pipefd) != 0) return 0;
        fcntl(pipefd[0], F_SETFD, FD_CLOEXEC);
        fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);
#endif

        pid_t pid = fork();
        if (pid < 0) {
            close(pipefd[0]); close(pipefd[1]);
            return 0;
        }
        if (pid == 0) {
            // Child
            if (dup2(pipefd[0], STDIN_FILENO) < 0) _exit(127);
            close(pipefd[0]);
            close(pipefd[1]);

            char path[] = "PATH=/usr/sbin:/usr/bin:/sbin:/bin";
            char lcall[] = "LC_ALL=C";
            char* envp[] = { path, lcall, NULL };
            char* argv[] = { (char*)chpasswd, NULL };
            execve(chpasswd, argv, envp);
            _exit(127);
        } else {
            // Parent
            close(pipefd[0]);
            size_t ulen = strlen(username);
            size_t plen = strlen(new_password);
            // Build "username:password\n" safely
            size_t total = ulen + 1 + plen + 1;
            char* line = (char*)malloc(total + 1);
            if (!line) {
                close(pipefd[1]);
                int st=0; waitpid(pid, &st, 0);
                return 0;
            }
            memcpy(line, username, ulen);
            line[ulen] = ':';
            memcpy(line + ulen + 1, new_password, plen);
            line[ulen + 1 + plen] = '\n';
            line[total] = '\0';

            size_t written = 0;
            while (written < total) {
                ssize_t w = write(pipefd[1], line + written, total - written);
                if (w < 0) {
                    secure_memzero(line, total + 1);
                    free(line);
                    close(pipefd[1]);
                    int st=0; waitpid(pid, &st, 0);
                    return 0;
                }
                written += (size_t)w;
            }
            secure_memzero(line, total + 1);
            free(line);
            close(pipefd[1]);

            int status = 0;
            if (waitpid(pid, &status, 0) < 0) return 0;
            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) return 0;
        }
    }

    if (geteuid() == 0) {
        const char* du = (drop_user && *drop_user) ? drop_user : "nobody";
        if (!drop_privileges_to(du)) return 0;
    }
    return 1;
}

int main(void) {
    printf("%s\n", change_password_and_drop("testuser", "CorrectHorse1!", "nobody", 1) ? "true" : "false");
    printf("%s\n", change_password_and_drop("root", "XyZ12345!", "nobody", 1) ? "true" : "false");
    printf("%s\n", change_password_and_drop("bad:user", "StrongPassw0rd!", "nobody", 1) ? "true" : "false");
    printf("%s\n", change_password_and_drop("good_user", "short", "nobody", 1) ? "true" : "false");
    printf("%s\n", change_password_and_drop("alice", "Secur3P@ssword!", "nobody", 1) ? "true" : "false");
    return 0;
}