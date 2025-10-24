
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <lmaccess.h>
    #include <lm.h>
#else
    #include <unistd.h>
    #include <pwd.h>
    #include <sys/types.h>
    #include <errno.h>
    #include <grp.h>
#endif

#define MAX_USERNAME_LEN 32
#define MIN_PASSWORD_LEN 8
#define MAX_PASSWORD_LEN 128

/* Secure memory clearing - prevents optimization removal */
void secure_clear(void* ptr, size_t size) {
    if (ptr == NULL || size == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (size--) *p++ = 0;
}

/* Validate username to prevent injection attacks */
bool validate_username(const char* username) {
    if (username == NULL) {
        return false;
    }
    
    size_t len = strnlen(username, MAX_USERNAME_LEN + 1);
    
    /* Check length bounds */
    if (len == 0 || len > MAX_USERNAME_LEN) {
        return false;
    }
    
    /* Validate each character - alphanumeric, underscore, or dash only */
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)username[i];
        if (!isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }
    
    /* Prevent directory traversal */
    if (strstr(username, "..") != NULL) {
        return false;
    }
    
    return true;
}

/* Validate password - enforce strong password policy */
bool validate_password(const char* password) {
    if (password == NULL) {
        return false;
    }
    
    size_t len = strnlen(password, MAX_PASSWORD_LEN + 1);
    
    /* Check length requirements */
    if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) {
        return false;
    }
    
    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;
    bool has_special = false;
    
    /* Check password complexity */
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (isupper(c)) {
            has_upper = true;
        } else if (islower(c)) {
            has_lower = true;
        } else if (isdigit(c)) {
            has_digit = true;
        } else if (ispunct(c) || c == ' ') {
            has_special = true;
        }
    }
    
    /* Require at least 3 of 4 character classes */
    int complexity = (has_upper ? 1 : 0) + (has_lower ? 1 : 0) + 
                     (has_digit ? 1 : 0) + (has_special ? 1 : 0);
    
    return complexity >= 3;
}

#ifdef _WIN32
/* Windows implementation of password change */
bool change_user_password_privileged(const char* username, const char* new_password) {
    /* Validate inputs before privileged operations */
    if (!validate_username(username)) {
        fprintf(stderr, "Invalid username format\\n");
        return false;
    }
    
    if (!validate_password(new_password)) {
        fprintf(stderr, "Password does not meet complexity requirements\\n");
        return false;
    }
    
    /* Convert to wide strings for Windows API */
    int username_size = MultiByteToWideChar(CP_UTF8, 0, username, -1, NULL, 0);
    int password_size = MultiByteToWideChar(CP_UTF8, 0, new_password, -1, NULL, 0);
    
    if (username_size <= 0 || password_size <= 0) {
        return false;
    }
    
    wchar_t* wusername = (wchar_t*)calloc(username_size, sizeof(wchar_t));
    wchar_t* wpassword = (wchar_t*)calloc(password_size, sizeof(wchar_t));
    
    if (wusername == NULL || wpassword == NULL) {
        free(wusername);
        free(wpassword);
        return false;
    }
    
    MultiByteToWideChar(CP_UTF8, 0, username, -1, wusername, username_size);
    MultiByteToWideChar(CP_UTF8, 0, new_password, -1, wpassword, password_size);
    
    USER_INFO_1003 ui;
    ui.usri1003_password = wpassword;
    
    /* Perform privileged operation */
    NET_API_STATUS status = NetUserSetInfo(NULL, wusername, 1003, (LPBYTE)&ui, NULL);
    
    /* Clear sensitive data immediately */
    secure_clear(wpassword, password_size * sizeof(wchar_t));
    
    free(wusername);
    free(wpassword);
    
    if (status != NERR_Success) {
        fprintf(stderr, "Failed to change password. Error: %lu\\n", status);
        return false;
    }
    
    return true;
}

/* Windows privilege dropping */
bool drop_privileges(void) {
    HANDLE hToken = NULL;
    
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }
    
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_REMOVED;
    
    /* Attempt to remove debug privilege */
    if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid)) {
        AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
    }
    
    CloseHandle(hToken);
    return true;
}

#else
/* Unix/Linux implementation */
bool change_user_password_privileged(const char* username, const char* new_password) {
    /* Must be running as root */
    if (geteuid() != 0) {
        fprintf(stderr, "Must run with root privileges to change passwords\\n");
        return false;
    }
    
    /* Validate inputs before privileged operations */
    if (!validate_username(username)) {
        fprintf(stderr, "Invalid username format\\n");
        return false;
    }
    
    if (!validate_password(new_password)) {
        fprintf(stderr, "Password does not meet complexity requirements\\n");
        return false;
    }
    
    /* Verify user exists */
    struct passwd* pw = getpwnam(username);
    if (pw == NULL) {
        fprintf(stderr, "User does not exist\\n");
        return false;
    }
    
    /* Build command with validated inputs */
    /* Maximum command size with safety margin */
    size_t cmd_size = strlen(username) + strlen(new_password) + 50;
    char* command = (char*)calloc(cmd_size, sizeof(char));
    
    if (command == NULL) {
        return false;
    }
    
    /* Use snprintf to prevent buffer overflow */
    int ret = snprintf(command, cmd_size, "echo '%s:%s' | chpasswd", username, new_password);
    
    if (ret < 0 || (size_t)ret >= cmd_size) {
        secure_clear(command, cmd_size);
        free(command);
        return false;
    }
    
    int result = system(command);
    
    /* Clear sensitive data */
    secure_clear(command, cmd_size);
    free(command);
    
    if (result != 0) {
        fprintf(stderr, "Failed to change password\\n");
        return false;
    }
    
    return true;
}

/* Unix/Linux privilege dropping */
bool drop_privileges(void) {
    /* Drop to nobody user (uid 65534) */
    uid_t target_uid = 65534;
    gid_t target_gid = 65534;
    
    /* Drop supplementary groups */
    if (setgroups(0, NULL) != 0) {
        fprintf(stderr, "Failed to drop supplementary groups\\n");
        return false;
    }
    
    /* Drop GID first (must be done before dropping UID) */
    if (setgid(target_gid) != 0) {
        fprintf(stderr, "Failed to drop group privileges\\n");
        return false;
    }
    
    /* Drop UID */
    if (setuid(target_uid) != 0) {
        fprintf(stderr, "Failed to drop user privileges\\n");
        return false;
    }
    
    /* Verify we cannot regain privileges */
    if (setuid(0) == 0 || seteuid(0) == 0) {
        fprintf(stderr, "Failed to permanently drop privileges\\n");
        return false;
    }
    
    return true;
}
#endif

/* Main secure password change function */
bool secure_password_change(const char* username, const char* new_password) {
    /* Perform privileged operation */
    bool success = change_user_password_privileged(username, new_password);
    
    /* Always attempt to drop privileges after operation */
    if (!drop_privileges()) {
        fprintf(stderr, "Warning: Failed to drop privileges\\n");
    }
    
    return success;
}

int main(void) {
    /* Test Case 1: Valid password change */
    printf("=== Test Case 1: Valid password change ===\\n");
    {
        char username[] = "testuser1";
        char password[] = "SecurePass123!";
        bool result = secure_password_change(username, password);
        printf("Result: %s\\n", result ? "SUCCESS" : "FAILED");
        secure_clear(password, sizeof(password));
    }
    
    /* Test Case 2: Invalid username (too long) */
    printf("\\n=== Test Case 2: Invalid username (too long) ===\\n");
    {
        char username[] = "thisusernameiswaytoolongandshouldfailvalidation";
        char password[] = "ValidPass123!";
        bool result = secure_password_change(username, password);
        printf("Result: %s\\n", result ? "SUCCESS" : "FAILED (expected)");
        secure_clear(password, sizeof(password));
    }
    
    /* Test Case 3: Invalid password (too short) */
    printf("\\n=== Test Case 3: Invalid password (too short) ===\\n");
    {
        char username[] = "testuser2";
        char password[] = "short";
        bool result = secure_password_change(username, password);
        printf("Result: %s\\n", result ? "SUCCESS" : "FAILED (expected)");
        secure_clear(password, sizeof(password));
    }
    
    /* Test Case 4: Invalid username (injection attempt) */
    printf("\\n=== Test Case 4: Invalid username (injection attempt) ===\\n");
    {
        char username[] = "user; rm -rf /";
        char password[] = "ValidPass123!";
        bool result = secure_password_change(username, password);
        printf("Result: %s\\n", result ? "SUCCESS" : "FAILED (expected)");
        secure_clear(password, sizeof(password));
    }
    
    /* Test Case 5: Valid complex password */
    printf("\\n=== Test Case 5: Valid complex password ===\\n");
    {
        char username[] = "testuser3";
        char password[] = "C0mpl3x!P@ssw0rd";
        bool result = secure_password_change(username, password);
        printf("Result: %s\\n", result ? "SUCCESS" : "FAILED");
        secure_clear(password, sizeof(password));
    }
    
    return 0;
}
