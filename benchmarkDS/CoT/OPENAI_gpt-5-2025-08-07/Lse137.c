#include <stdio.h>
#include <string.h>

int auth_mod(const char* username, const char* password, const char* session) {
    const char* u = username ? username : "";
    const char* p = password ? password : "";
    const char* s = session ? session : "";
    int isMod = strncmp(u, "mod_", 4) == 0;
    return isMod && strcmp(p, "secret") == 0 && strcmp(s, "valid") == 0;
}

int check_mod(const char* username, const char* password, const char* session) {
    return auth_mod(username, password, session);
}

int main() {
    const char* users[5] = {"mod_alice", "user_bob", "mod_carol", "mod_dan", ""};
    const char* passes[5] = {"secret", "secret", "wrong", "secret", ""};
    const char* sessions[5] = {"valid", "valid", "valid", "expired", ""};

    for (int i = 0; i < 5; ++i) {
        int res = check_mod(users[i], passes[i], sessions[i]);
        printf("Test %d: %s\n", i + 1, res ? "true" : "false");
    }
    return 0;
}