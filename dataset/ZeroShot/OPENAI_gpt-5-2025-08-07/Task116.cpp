#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

static bool isValidUsername(const std::string& u) {
    static const std::regex re("^[a-z_][a-z0-9_-]{0,31}$");
    if (!std::regex_match(u, re)) return false;
    errno = 0;
    struct passwd* pw = getpwnam(u.c_str());
    if (!pw) return false;
    return true;
}

static bool isStrongPassword(const std::string& p) {
    if (p.size() < 8 || p.size() > 128) return false;
    return true;
}

static std::string findChpasswdPath() {
    const char* candidates[] = {
        "/usr/sbin/chpasswd",
        "/sbin/chpasswd",
        "/usr/bin/chpasswd",
        "/bin/chpasswd"
    };
    struct stat st{};
    for (const char* c : candidates) {
        if (stat(c, &st) == 0 && (st.st_mode & S_IXUSR)) {
            return std::string(c);
        }
    }
    return std::string();
}

static void secure_memzero(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) *p++ = 0;
}

static bool dropPrivilegesTo(const std::string& user) {
    struct passwd* pw = getpwnam(user.c_str());
    if (!pw) return false;

#ifdef __linux__
    if (initgroups(pw->pw_name, pw->pw_gid) != 0) return false;
#endif
    if (setgid(pw->pw_gid) != 0) return false;
#if defined(HAVE_SETRESUID) || (defined(__linux__) && !defined(__APPLE__))
    if (setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid) != 0) return false;
#else
    if (setuid(pw->pw_uid) != 0) return false;
#endif
    if (geteuid() == 0 || getuid() == 0) return false;
    return true;
}

static bool changePasswordAndDrop(const std::string& username, const std::string& newPassword, const std::string& dropUser, bool dryRun) {
    if (!isValidUsername(username)) return false;
    if (!isStrongPassword(newPassword)) return false;

    if (!dryRun) {
        if (geteuid() != 0) return false;
        std::string chpasswd = findChpasswdPath();
        if (chpasswd.empty()) return false;

        int pipefd[2];
        if (pipe2(pipefd, O_CLOEXEC) != 0) return false;

        pid_t pid = fork();
        if (pid < 0) {
            close(pipefd[0]); close(pipefd[1]);
            return false;
        }
        if (pid == 0) {
            // Child
            // stdin from pipe
            if (dup2(pipefd[0], STDIN_FILENO) < 0) _exit(127);
            close(pipefd[0]);
            close(pipefd[1]);

            // Minimal env
            char path[] = (char*)"PATH=/usr/sbin:/usr/bin:/sbin:/bin";
            char lcall[] = (char*)"LC_ALL=C";
            char* envp[] = { path, lcall, nullptr };

            char* const argv[] = { (char*)chpasswd.c_str(), nullptr };
            execve(chpasswd.c_str(), argv, envp);
            _exit(127);
        } else {
            // Parent
            close(pipefd[0]);
            std::string line = username + ":" + newPassword + "\n";
            ssize_t toWrite = (ssize_t)line.size();
            const char* buf = line.c_str();
            ssize_t written = 0;
            while (written < toWrite) {
                ssize_t w = write(pipefd[1], buf + written, (size_t)(toWrite - written));
                if (w < 0) {
                    close(pipefd[1]);
                    // Attempt to wait child
                    int st=0; waitpid(pid, &st, 0);
                    return false;
                }
                written += w;
            }
            secure_memzero((void*)line.data(), line.size());
            close(pipefd[1]);
            int status = 0;
            if (waitpid(pid, &status, 0) < 0) return false;
            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) return false;
        }
    }

    // Drop privileges if running as root
    if (geteuid() == 0) {
        std::string du = dropUser.empty() ? "nobody" : dropUser;
        if (!dropPrivilegesTo(du)) return false;
    }
    return true;
}

int main() {
    std::cout << std::boolalpha;
    std::cout << changePasswordAndDrop("testuser", "CorrectHorse1!", "nobody", true) << "\n";
    std::cout << changePasswordAndDrop("root", "XyZ12345!", "nobody", true) << "\n";
    std::cout << changePasswordAndDrop("bad:user", "StrongPassw0rd!", "nobody", true) << "\n";
    std::cout << changePasswordAndDrop("good_user", "short", "nobody", true) << "\n";
    std::cout << changePasswordAndDrop("alice", "Secur3P@ssword!", "nobody", true) << "\n";
    return 0;
}