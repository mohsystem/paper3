#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// For safety, this function simulates a privileged operation (e.g., changing a password)
// and demonstrates dropping privileges. It does not actually modify system passwords.
char* perform_privileged_and_drop(const char* targetUser, const char* newPassword, const char* dropToUser) {
    // Allocate a result buffer
    char* out = (char*)malloc(1024);
    if (!out) return NULL;
    out[0] = '\0';

    int isRoot = (geteuid() == 0);
    snprintf(out + strlen(out), 1024 - strlen(out), "isRoot=%s; ", isRoot ? "true" : "false");

    pid_t pid = getpid();
    char path[256];
    snprintf(path, sizeof(path), "/tmp/task116_c_priv_%d", (int)pid);

    int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd < 0) {
        snprintf(out + strlen(out), 1024 - strlen(out), "createFileError=%d; ", errno);
        return out;
    }
    char buf[256];
    snprintf(buf, sizeof(buf), "Simulated privileged content for %s\n", targetUser ? targetUser : "");
    write(fd, buf, strlen(buf));
    close(fd);
    snprintf(out + strlen(out), 1024 - strlen(out), "createdFile=%s; ", path);

    if (isRoot) {
        struct passwd* pw = getpwnam(dropToUser);
        if (!pw) {
            snprintf(out + strlen(out), 1024 - strlen(out), "dropToUserNotFound=%s; ", dropToUser);
            return out;
        }
        if (initgroups(pw->pw_name, pw->pw_gid) != 0) {
            snprintf(out + strlen(out), 1024 - strlen(out), "initgroupsError=%d; ", errno);
            return out;
        }
        if (setgid(pw->pw_gid) != 0) {
            snprintf(out + strlen(out), 1024 - strlen(out), "setgidError=%d; ", errno);
            return out;
        }
        if (setuid(pw->pw_uid) != 0) {
            snprintf(out + strlen(out), 1024 - strlen(out), "setuidError=%d; ", errno);
            return out;
        }
        int afd = open(path, O_WRONLY | O_APPEND);
        if (afd >= 0) {
            snprintf(out + strlen(out), 1024 - strlen(out), "writeAfterDrop=unexpected-success; ");
            close(afd);
        } else {
            if (errno == EACCES || errno == EPERM) {
                snprintf(out + strlen(out), 1024 - strlen(out), "writeAfterDrop=permission-denied; ");
            } else {
                snprintf(out + strlen(out), 1024 - strlen(out), "writeAfterDrop=error-%d; ", errno);
            }
        }
        snprintf(out + strlen(out), 1024 - strlen(out), "euidAfterDrop=%d; ", (int)geteuid());
    } else {
        snprintf(out + strlen(out), 1024 - strlen(out), "notRootNoRealDrop; ");
    }

    return out;
}

int main(void) {
    // 5 test cases (simulation only)
    char* r1 = perform_privileged_and_drop("alice", "Secret123!", "nobody");
    char* r2 = perform_privileged_and_drop("bob", "P@ssw0rd!", "daemon");
    char* r3 = perform_privileged_and_drop("carol", "NewPass#1", "nonexistentuser");
    char* r4 = perform_privileged_and_drop("dave", "Another#2", "nobody");
    char* r5 = perform_privileged_and_drop("root", "RootPass#3", "daemon");

    if (r1) { printf("%s\n", r1); free(r1); }
    if (r2) { printf("%s\n", r2); free(r2); }
    if (r3) { printf("%s\n", r3); free(r3); }
    if (r4) { printf("%s\n", r4); free(r4); }
    if (r5) { printf("%s\n", r5); free(r5); }

    return 0;
}