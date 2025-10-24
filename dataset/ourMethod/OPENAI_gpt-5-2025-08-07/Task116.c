#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static bool validate_username(const char *username) {
    if (username == NULL) return false;
    size_t len = strlen(username);
    if (len < 1 || len > 32) return false;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)username[i];
        if (!(isalnum(c) || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

static bool validate_password(const char *password) {
    if (password == NULL) return false;
    size_t len = strlen(password);
    if (len < 12 || len > 128) return false;
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    const char *specials = "!@#$%^&*()-_=+[]{}|;:',.<>/?`~\"\\";
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (isupper(c)) hasUpper = true;
        else if (islower(c)) hasLower = true;
        else if (isdigit(c)) hasDigit = true;
        else {
            // check special
            for (const char *p = specials; *p; ++p) {
                if (c == (unsigned char)*p) {
                    hasSpecial = true;
                    break;
                }
            }
        }
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

static bool is_root_from_euid(uid_t euid) {
    return euid == 0;
}

static uid_t get_effective_uid(int /*dummy*/) {
    return geteuid();
}

// Simulated privileged password change (does not alter OS state)
static bool perform_privileged_password_change(uid_t euid, const char *username, const char *new_password) {
    if (!is_root_from_euid(euid)) return false;
    if (!validate_username(username)) return false;
    if (!validate_password(new_password)) return false;
    // Simulate success
    return true;
}

static bool drop_privileges(uid_t euid, const char *target_user) {
    if (!is_root_from_euid(euid)) return false;
    if (target_user == NULL) return false;
    size_t len = strlen(target_user);
    if (len < 1 || len > 32) return false;

    struct passwd *pw = getpwnam(target_user);
    if (pw == NULL) return false;

    if (initgroups(pw->pw_name, pw->pw_gid) != 0) return false;
    if (setgid(pw->pw_gid) != 0) return false;
    if (setuid(pw->pw_uid) != 0) return false;

    // Verify we cannot regain root
    if (setuid(0) == 0) {
        return false; // should not be able to regain root
    }
    return geteuid() != 0 && getuid() != 0;
}

struct ChangeDropResult {
    bool changed;
    bool dropped;
};

static struct ChangeDropResult change_password_then_drop(const char *username, const char *new_password, const char *target_user) {
    uid_t euid = get_effective_uid(0);
    bool changed = perform_privileged_password_change(euid, username, new_password);
    bool dropped = false;
    if (changed) {
        dropped = drop_privileges(euid, target_user);
    }
    struct ChangeDropResult res = { changed, dropped };
    return res;
}

static bool can_still_perform_privileged_ops(int /*dummy*/) {
    return is_root_from_euid(geteuid());
}

int main(void) {
    const char *users[5] = {"alice", "bob-01", "bad!name", "charlie", "d"};
    const char *passwords[5] = {"StrongPassw0rd!", "Another$trongP4ss", "ValidP@ssw0rd2025", "WeakPass1!", "Too$hort1"};
    const char *targets[5] = {"nobody", "daemon", "nobody", "nobody", "nobody"};

    for (int i = 0; i < 5; i++) {
        struct ChangeDropResult res = change_password_then_drop(users[i], passwords[i], targets[i]);
        bool canPriv = can_still_perform_privileged_ops(0);
        printf("Test%d: changedAndDropped=%s, canPrivNow=%s, userTried=%s\n",
               i + 1,
               (res.changed && res.dropped) ? "true" : "false",
               canPriv ? "true" : "false",
               users[i]);
    }
    return 0;
}