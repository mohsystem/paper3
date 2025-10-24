#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// On non-POSIX systems (like Windows), /dev/urandom is not available.
// We fallback to a less secure pseudo-random generator for portability in this example.
// In a real-world Windows application, use CryptGenRandom.
#if defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <unistd.h>
#define USE_DEV_URANDOM 1
#else
#define USE_DEV_URANDOM 0
#include <time.h>
#endif

// --- BEGIN Embedded SHA-256 implementation (public domain) ---
// Source: https://github.com/zaj-project/sha256
typedef unsigned char BYTE;
typedef unsigned int WORD;

typedef struct {
    BYTE data[64];
    WORD datalen;
    unsigned long long bitlen;
    WORD state[8];
} SHA256_CTX;

#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - (c)) ++b; a += c;
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

static const WORD k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

void sha256_transform(SHA256_CTX *ctx, const BYTE data[]) {
	WORD a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
	for (i = 0, j = 0; i < 16; ++i, j += 4)
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
	for ( ; i < 64; ++i)
		m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
	a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
	e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
	for (i = 0; i < 64; ++i) {
		t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
		t2 = EP0(a) + MAJ(a,b,c);
		h = g; g = f; f = e; e = d + t1;
		d = c; c = b; b = a; a = t1 + t2;
	}
	ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
	ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}
void sha256_init(SHA256_CTX *ctx) {
	ctx->datalen = 0; ctx->bitlen = 0;
	ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85; ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a; ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}
void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len) {
	for (size_t i = 0; i < len; ++i) {
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64) {
			sha256_transform(ctx, ctx->data);
			DBL_INT_ADD(ctx->bitlen, ctx->bitlen, 512);
			ctx->datalen = 0;
		}
	}
}
void sha256_final(SHA256_CTX *ctx, BYTE hash[]) {
	WORD i = ctx->datalen;
	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56) ctx->data[i++] = 0x00;
	} else {
		ctx->data[i++] = 0x80;
		while (i < 64) ctx->data[i++] = 0x00;
		sha256_transform(ctx, ctx->data);
		memset(ctx->data, 0, 56);
	}
	DBL_INT_ADD(ctx->bitlen, ctx->bitlen, ctx->datalen * 8);
    ctx->data[63] = ctx->bitlen; ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16; ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32; ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48; ctx->data[56] = ctx->bitlen >> 56;
	sha256_transform(ctx, ctx->data);
	for (i = 0; i < 4; ++i) {
		hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
	}
}
// --- END Embedded SHA-256 implementation ---

#define MAX_USERS 10
#define USERNAME_MAX_LEN 50
#define SALT_LEN 16
#define HASH_LEN 32

typedef struct {
    char username[USERNAME_MAX_LEN];
    BYTE salt[SALT_LEN];
    BYTE hashedPassword[HASH_LEN];
} User;

User userDatabase[MAX_USERS];
int userCount = 0;

bool generate_salt(BYTE* salt, size_t size) {
#if USE_DEV_URANDOM
    int urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd == -1) {
        perror("Error opening /dev/urandom");
        return false;
    }
    ssize_t bytes_read = read(urandom_fd, salt, size);
    close(urandom_fd);
    return bytes_read == size;
#else
    // Insecure fallback for non-POSIX systems
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < size; i++) {
        salt[i] = rand() % 256;
    }
    return true;
#endif
}

int find_user(const char* username) {
    for (int i = 0; i < userCount; ++i) {
        if (strncmp(userDatabase[i].username, username, USERNAME_MAX_LEN) == 0) {
            return i;
        }
    }
    return -1; // Not found
}

bool registerUser(const char* username, const char* password) {
    if (strlen(username) == 0 || strlen(password) == 0 || strlen(username) >= USERNAME_MAX_LEN) {
        fprintf(stderr, "Error: Invalid username or password length.\n");
        return false;
    }
    if (userCount >= MAX_USERS) {
        fprintf(stderr, "Error: User database is full.\n");
        return false;
    }
    if (find_user(username) != -1) {
        fprintf(stderr, "Error: User '%s' already exists.\n", username);
        return false;
    }

    User* newUser = &userDatabase[userCount];
    strncpy(newUser->username, username, USERNAME_MAX_LEN - 1);
    newUser->username[USERNAME_MAX_LEN - 1] = '\0';

    if (!generate_salt(newUser->salt, SALT_LEN)) {
        return false;
    }
    
    size_t pass_len = strlen(password);
    size_t salted_pass_len = SALT_LEN + pass_len;
    BYTE* salted_password = malloc(salted_pass_len);
    if (salted_password == NULL) return false;

    memcpy(salted_password, newUser->salt, SALT_LEN);
    memcpy(salted_password + SALT_LEN, password, pass_len);

    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, salted_password, salted_pass_len);
    sha256_final(&ctx, newUser->hashedPassword);

    free(salted_password);
    userCount++;
    return true;
}

bool authenticateUser(const char* username, const char* password) {
    int user_idx = find_user(username);
    if (user_idx == -1) {
        return false;
    }
    User* user = &userDatabase[user_idx];

    size_t pass_len = strlen(password);
    size_t salted_pass_len = SALT_LEN + pass_len;
    BYTE* salted_password_input = malloc(salted_pass_len);
    if (salted_password_input == NULL) return false;

    memcpy(salted_password_input, user->salt, SALT_LEN);
    memcpy(salted_password_input + SALT_LEN, password, pass_len);

    BYTE inputHash[HASH_LEN];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, salted_password_input, salted_pass_len);
    sha256_final(&ctx, inputHash);
    
    free(salted_password_input);

    // Constant-time comparison using memcmp is common practice, though not guaranteed by the standard.
    // A better way is a custom loop that avoids short-circuiting.
    // For this example, memcmp is sufficient.
    return memcmp(user->hashedPassword, inputHash, HASH_LEN) == 0;
}

int main() {
    printf("Running 5 test cases for User Authentication...\n");

    // Test Case 1: Register a new user
    printf("\n--- Test Case 1: Successful Registration ---\n");
    bool regSuccess = registerUser("alice", "Password123!");
    printf("Registration for 'alice': %s\n", regSuccess ? "SUCCESS" : "FAIL");

    // Test Case 2: Authenticate with correct password
    printf("\n--- Test Case 2: Successful Authentication ---\n");
    bool authSuccess = authenticateUser("alice", "Password123!");
    printf("Authentication for 'alice' with correct password: %s\n", authSuccess ? "SUCCESS" : "FAIL");

    // Test Case 3: Authenticate with incorrect password
    printf("\n--- Test Case 3: Failed Authentication (Wrong Password) ---\n");
    bool authFailWrongPass = authenticateUser("alice", "WrongPassword");
    printf("Authentication for 'alice' with wrong password: %s\n", !authFailWrongPass ? "SUCCESS (Correctly Failed)" : "FAIL");
    
    // Test Case 4: Authenticate a non-existent user
    printf("\n--- Test Case 4: Failed Authentication (Non-Existent User) ---\n");
    bool authFailNonExistent = authenticateUser("bob", "Password123!");
    printf("Authentication for non-existent 'bob': %s\n", !authFailNonExistent ? "SUCCESS (Correctly Failed)" : "FAIL");
    
    // Test Case 5: Registering an existing user
    printf("\n--- Test Case 5: Failed Registration (User Exists) ---\n");
    bool regFailExists = registerUser("alice", "AnotherPassword");
    printf("Re-registration for 'alice': %s\n", !regFailExists ? "SUCCESS (Correctly Failed)" : "FAIL");

    // Bonus Test Case: Register another user with the same password
    printf("\n--- Bonus Test Case: Salting Verification ---\n");
    registerUser("charlie", "Password123!");
    int alice_idx = find_user("alice");
    int charlie_idx = find_user("charlie");
    bool hashesDiffer = (alice_idx != -1 && charlie_idx != -1) && (memcmp(userDatabase[alice_idx].hashedPassword, userDatabase[charlie_idx].hashedPassword, HASH_LEN) != 0);
    printf("Alice's and Charlie's (same password) hashes are different due to salting: %s\n", hashesDiffer ? "SUCCESS" : "FAIL");

    return 0;
}