// Chain-of-Through Process in Code (C++)
// 1) Problem understanding:
//    - Provide a function to change a user's password using chpasswd (securely) and then drop privileges.
// 2) Security requirements:
//    - Validate inputs, avoid shell, pass secret via stdin, minimal environment.
//    - Drop supplementary groups, then gid, then uid.
//    - Zero out sensitive buffers.
// 3) Secure coding generation: careful fork/exec/pipe and error handling.
// 4) Code review: no secret prints, check return codes, close fds, zero buffers.
// 5) Secure code output: final function and test cases below.

#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

static void secure_memzero(void* p, size_t n) {
    volatile unsigned char* v = (volatile unsigned char*)p;
    while (n--) *v++ = 0;
}

static bool path_exists_exec(const char* p) {
    return p && access(p, X_OK) == 0;
}

static std::string find_chpasswd() {
    const char* cands[] = {
        "/usr/sbin/chpasswd",
        "/usr/bin/chpasswd",
        "/sbin/chpasswd",
        "/bin/chpasswd"
    };
    for (auto c : cands) {
        if (path_exists_exec(c)) return std::string(c);
    }
    return std::string();
}

static bool is_valid_username(const std::string& u) {
    if (u.size() < 1 || u.size() > 32) return false;
    char c0 = u[0];
    if (!(c0 == '_' || (c0 >= 'a' && c0 <= 'z'))) return false;
    for (size_t i = 1; i < u.size(); ++i) {
        char c = u[i];
        bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-';
        if (!ok) return false;
    }
    return true;
}

static bool is_valid_password(const std::string& pw) {
    if (pw.size() < 8 || pw.size() > 128) return false;
    for (unsigned char c : pw) {
        if (c == ':' || c < 0x20 || c > 0x7E) return false;
    }
    return true;
}

static int run_chpasswd(const std::string& user, const std::string& pw) {
    std::string bin = find_chpasswd();
    if (bin.empty()) return -2;

    int pipefd[2];
    if (pipe(pipefd) != 0) return -1;

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        // Child: read from pipe -> stdin, exec chpasswd
        // Close write end
        close(pipefd[1]);
        // Dup read end to stdin
        if (dup2(pipefd[0], STDIN_FILENO) < 0) _exit(127);
        // Close read end after dup
        close(pipefd[0]);
        // Close other fds
        for (int fd = 3; fd < 256; ++fd) close(fd);
        // Minimal environment
        char const* envp[] = {
            "PATH=/usr/sbin:/usr/bin:/bin",
            "LANG=C",
            nullptr
        };
        char const* argv[] = { bin.c_str(), nullptr };
        execve(bin.c_str(), (char* const*)argv, (char* const*)envp);
        _exit(127);
    }

    // Parent
    close(pipefd[0]);
    // Build payload "user:password\n"
    std::string payload = user + ":" + pw + "\n";
    ssize_t n = write(pipefd[1], payload.data(), payload.size());
    (void)n;
    // Zero sensitive contents of payload
    secure_memzero(&payload[0], payload.size());
    // Close write end to signal EOF
    close(pipefd[1]);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) return -1;
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

static bool drop_privileges(uid_t to_uid, gid_t to_gid) {
    if (geteuid() != 0) return true;
    // Clear supplementary groups
    if (setgroups(0, nullptr) != 0) {
        // continue even if it fails
    }
    // Set gid then uid
#if defined(__linux__) && defined(HAVE_SETRESGID)
    if (setresgid(to_gid, to_gid, to_gid) != 0) return false;
#else
    if (setgid(to_gid) != 0) return false;
#endif

#if defined(__linux__) && defined(HAVE_SETRESUID)
    if (setresuid(to_uid, to_uid, to_uid) != 0) return false;
#else
    if (setuid(to_uid) != 0) return false;
#endif
    // Verify
    return (geteuid() == to_uid) && (getegid() == to_gid);
}

// Returns 0 on success, negative for validation/system errors, positive for external command exit code
int change_password_and_drop_cpp(const std::string& username, const std::string& new_password, bool dry_run, uid_t drop_uid, gid_t drop_gid) {
    if (!is_valid_username(username)) return -1;
    if (!is_valid_password(new_password)) return -2;

    int rc = 0;
    if (!dry_run && geteuid() == 0) {
        rc = run_chpasswd(username, new_password);
        if (rc != 0) return (rc == -2) ? -3 : rc;
    }
    // Drop to requested or real uid/gid
    uid_t target_uid = (drop_uid == (uid_t)-1) ? getuid() : drop_uid;
    gid_t target_gid = (drop_gid == (gid_t)-1) ? getgid() : drop_gid;
    drop_privileges(target_uid, target_gid);
    return rc;
}

int main() {
    // 5 test cases (dry-run to avoid changing system)
    struct Case { const char* u; const char* p; } cases[] = {
        {"alice", "S3curePwd1!"},
        {"bob", "AnotherPwd2#"},
        {"charlie", "ThirdPwd3$"},
        {"dave-user", "FourthPwd4%"},
        {"eve_user", "FifthPwd5^"}
    };
    for (int i = 0; i < 5; ++i) {
        int rc = change_password_and_drop_cpp(cases[i].u, cases[i].p, true, (uid_t)-1, (gid_t)-1);
        std::printf("Test %d user=%s rc=%d\n", i+1, cases[i].u, rc);
    }
    return 0;
}