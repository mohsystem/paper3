#include <iostream>
#include <cstring>
#include <cstddef>

struct User {
    char email[255];
    char password[129];
    bool loggedIn;
};

static size_t my_strnlen(const char* s, size_t maxlen) {
    if (!s) return 0;
    size_t i = 0;
    for (; i < maxlen; ++i) {
        if (s[i] == '\0') break;
    }
    return i;
}

static bool safe_copy(char* dst, size_t dstsz, const char* src) {
    if (!dst || dstsz == 0) return false;
    if (!src) { dst[0] = '\0'; return false; }
    size_t n = my_strnlen(src, dstsz - 1);
    std::memcpy(dst, src, n);
    dst[n] = '\0';
    return my_strnlen(src, SIZE_MAX) == n; // true if not truncated
}

static bool ct_equal_strings(const char* a, const char* b, size_t maxCheck) {
    size_t la = my_strnlen(a, maxCheck);
    size_t lb = my_strnlen(b, maxCheck);
    size_t maxl = la > lb ? la : lb;
    unsigned char diff = (unsigned char)(la ^ lb);
    for (size_t i = 0; i < maxl; ++i) {
        unsigned char ca = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char cb = (i < lb) ? (unsigned char)b[i] : 0;
        diff |= (unsigned char)(ca ^ cb);
    }
    return diff == 0;
}

User init_user(const char* email, const char* password) {
    User u{};
    u.loggedIn = false;
    safe_copy(u.email, sizeof(u.email), email ? email : "");
    // Store password in-memory; use constant-time comparison on checks.
    safe_copy(u.password, sizeof(u.password), password ? password : "");
    return u;
}

bool login(User& u, const char* email, const char* password) {
    if (!email || !password) return false;
    // Email check (regular equality)
    if (std::strncmp(u.email, email, sizeof(u.email)) != 0) return false;
    // Constant-time password verify
    bool ok = ct_equal_strings(u.password, password, sizeof(u.password));
    if (ok) u.loggedIn = true;
    return ok;
}

void logout(User& u) {
    u.loggedIn = false;
}

bool change_email(User& u, const char* oldEmail, const char* password, const char* newEmail) {
    if (!oldEmail || !password || !newEmail) return false;
    if (!u.loggedIn) return false;
    if (std::strncmp(u.email, oldEmail, sizeof(u.email)) != 0) return false;
    if (!ct_equal_strings(u.password, password, sizeof(u.password))) return false;
    // Validate new email length
    if (my_strnlen(newEmail, sizeof(u.email) - 1) == sizeof(u.email) - 1 &&
        newEmail[sizeof(u.email) - 1] != '\0') {
        return false; // too long, would truncate
    }
    safe_copy(u.email, sizeof(u.email), newEmail);
    return true;
}

const char* get_email(const User& u) {
    return u.email;
}

int main() {
    // Test Case 1: Successful login and email change
    User u1 = init_user("user@example.com", "StrongPass!123");
    bool t1Login = login(u1, "user@example.com", "StrongPass!123");
    bool t1Change = change_email(u1, "user@example.com", "StrongPass!123", "new1@example.com");
    std::cout << "TC1 login: " << (t1Login ? "true" : "false")
              << ", change: " << (t1Change ? "true" : "false")
              << ", email: " << get_email(u1) << "\n";

    // Test Case 2: Attempt change without login
    User u2 = init_user("alpha@ex.com", "Ap@55word");
    bool t2Change = change_email(u2, "alpha@ex.com", "Ap@55word", "alpha2@ex.com");
    std::cout << "TC2 change without login: " << (t2Change ? "true" : "false")
              << ", email: " << get_email(u2) << "\n";

    // Test Case 3: Wrong old email after login
    User u3 = init_user("beta@ex.com", "BetaSecure1!");
    bool t3Login = login(u3, "beta@ex.com", "BetaSecure1!");
    bool t3Change = change_email(u3, "wrong@ex.com", "BetaSecure1!", "beta2@ex.com");
    std::cout << "TC3 login: " << (t3Login ? "true" : "false")
              << ", change with wrong old email: " << (t3Change ? "true" : "false")
              << ", email: " << get_email(u3) << "\n";

    // Test Case 4: Wrong password after login
    User u4 = init_user("gamma@ex.com", "GammaStrong1#");
    bool t4Login = login(u4, "gamma@ex.com", "GammaStrong1#");
    bool t4Change = change_email(u4, "gamma@ex.com", "WrongPass", "gamma2@ex.com");
    std::cout << "TC4 login: " << (t4Login ? "true" : "false")
              << ", change with wrong password: " << (t4Change ? "true" : "false")
              << ", email: " << get_email(u4) << "\n";

    // Test Case 5: Change email again after re-login with updated email
    logout(u1);
    bool t5Login = login(u1, "new1@example.com", "StrongPass!123");
    bool t5Change = change_email(u1, "new1@example.com", "StrongPass!123", "final@example.com");
    std::cout << "TC5 login with updated email: " << (t5Login ? "true" : "false")
              << ", change: " << (t5Change ? "true" : "false")
              << ", email: " << get_email(u1) << "\n";

    return 0;
}