#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>

#include <iostream>
#include <string>
#include <vector>
#include <cctype>

static bool validateUsername(const std::string &username) {
    if (username.empty() || username.size() > 32) return false;
    for (char c : username) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

static bool validatePassword(const std::string &password) {
    if (password.size() < 12 || password.size() > 128) return false;
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    const std::string specials = "!@#$%^&*()-_=+[]{}|;:',.<>/?`~\"\\";
    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) hasUpper = true;
        else if (std::islower(static_cast<unsigned char>(c))) hasLower = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
        else if (specials.find(c) != std::string::npos) hasSpecial = true;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

static bool isRootFromEuid(uid_t euid) {
    return euid == 0;
}

static uid_t getEffectiveUid(int /*dummy*/) {
    return geteuid();
}

// Simulated privileged change (does not modify OS password DB)
static bool performPrivilegedPasswordChange(uid_t euid, const std::string &username, const std::string &newPassword) {
    if (!isRootFromEuid(euid)) return false;
    if (!validateUsername(username)) return false;
    if (!validatePassword(newPassword)) return false;
    // Simulate success
    return true;
}

static bool dropPrivileges(uid_t euid, const std::string &targetUser) {
    if (!isRootFromEuid(euid)) return false;
    if (targetUser.empty() || targetUser.size() > 32) return false;

    struct passwd *pw = getpwnam(targetUser.c_str());
    if (!pw) return false;

    // Set supplementary groups, then primary gid, then uid
    if (initgroups(pw->pw_name, pw->pw_gid) != 0) return false;
    if (setgid(pw->pw_gid) != 0) return false;
    if (setuid(pw->pw_uid) != 0) return false;

    // Verify we cannot regain root
    if (setuid(0) == 0) {
        return false; // should not be able to regain root
    }
    return geteuid() != 0 && getuid() != 0;
}

static std::pair<bool, bool> changePasswordThenDrop(const std::string &username, const std::string &newPassword, const std::string &targetUser) {
    uid_t euid = getEffectiveUid(0);
    bool changed = performPrivilegedPasswordChange(euid, username, newPassword);
    bool dropped = false;
    if (changed) {
        dropped = dropPrivileges(euid, targetUser);
    }
    return {changed, dropped};
}

static bool canStillPerformPrivilegedOps(int /*dummy*/) {
    return isRootFromEuid(geteuid());
}

int main() {
    std::vector<std::tuple<std::string, std::string, std::string>> tests = {
        {"alice", "StrongPassw0rd!", "nobody"},
        {"bob-01", "Another$trongP4ss", "daemon"},
        {"bad!name", "ValidP@ssw0rd2025", "nobody"},
        {"charlie", "WeakPass1!", "nobody"},
        {"d", "Too$hort1", "nobody"}
    };

    int i = 1;
    for (const auto &tc : tests) {
        const std::string &user = std::get<0>(tc);
        const std::string &pass = std::get<1>(tc);
        const std::string &target = std::get<2>(tc);

        auto res = changePasswordThenDrop(user, pass, target);
        bool canPriv = canStillPerformPrivilegedOps(0);
        std::cout << "Test" << i++ << ": changedAndDropped=" << (res.first && res.second ? "true" : "false")
                  << ", canPrivNow=" << (canPriv ? "true" : "false")
                  << ", userTried=" << user << "\n";
    }
    return 0;
}