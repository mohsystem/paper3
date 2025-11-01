#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// --- Start of SHA-256 implementation ---
// This is a compact, public domain SHA-256 implementation by an unknown author.
// It is included here to meet the "single source file" requirement without external libraries.
typedef unsigned char BYTE;
typedef unsigned int WORD;

#define SHA256_BLOCK_SIZE 32

typedef struct {
	BYTE data[64];
	WORD datalen;
	unsigned long long bitlen;
	WORD state[8];
} SHA256_CTX;

WORD k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

void sha256_transform(SHA256_CTX *ctx, const BYTE data[]) {
	WORD a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

	for (i = 0, j = 0; i < 16; ++i, j += 4)
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
	for ( ; i < 64; ++i)
		m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	e = ctx->state[4];
	f = ctx->state[5];
	g = ctx->state[6];
	h = ctx->state[7];

	for (i = 0; i < 64; ++i) {
		t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
		t2 = EP0(a) + MAJ(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
	ctx->state[5] += f;
	ctx->state[6] += g;
	ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx) {
	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->state[0] = 0x6a09e667;
	ctx->state[1] = 0xbb67ae85;
	ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f;
	ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab;
	ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len) {
	WORD i;

	for (i = 0; i < len; ++i) {
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64) {
			sha256_transform(ctx, ctx->data);
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}
}

void sha256_final(SHA256_CTX *ctx, BYTE hash[]) {
	WORD i;

	i = ctx->datalen;
	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56)
			ctx->data[i++] = 0x00;
	}
	else {
		ctx->data[i++] = 0x80;
		while (i < 64)
			ctx->data[i++] = 0x00;
		sha256_transform(ctx, ctx->data);
		memset(ctx->data, 0, 56);
	}

	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen;
	ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16;
	ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32;
	ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48;
	ctx->data[56] = ctx->bitlen >> 56;
	sha256_transform(ctx, ctx->data);

	for (i = 0; i < 4; ++i) {
		hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0xff;
		hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0xff;
		hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0xff;
		hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0xff;
		hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0xff;
		hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0xff;
		hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0xff;
		hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0xff;
	}
}
// --- End of SHA-256 implementation ---

#define MAX_USERS 100
#define SALT_LEN 16
#define HASH_HEX_LEN (SHA256_BLOCK_SIZE * 2 + 1)

// Struct to hold user data
typedef struct {
    char* username;
    char salt[SALT_LEN + 1];
    char hashedPassword[HASH_HEX_LEN];
} UserData;

// In-memory user database (simulated)
static UserData* userDatabase[MAX_USERS];
static int userCount = 0;

/**
 * Generates a random salt string.
 * NOTE: rand() is not cryptographically secure. For production, use a library
 * like OpenSSL's RAND_bytes. This is for demonstration purposes only.
 */
void generateSalt(char* salt_buffer) {
    const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < SALT_LEN; i++) {
        int key = rand() % (sizeof(charset) - 1);
        salt_buffer[i] = charset[key];
    }
    salt_buffer[SALT_LEN] = '\0';
}

/**
 * Hashes password with salt and converts the hash to a hex string.
 */
void hashPassword(const char* password, const char* salt, char* hash_hex_buffer) {
    char combined[256];
    snprintf(combined, sizeof(combined), "%s%s", salt, password);
    
    BYTE hash_bytes[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    
    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE*)combined, strlen(combined));
    sha256_final(&ctx, hash_bytes);
    
    for(int i = 0; i < SHA256_BLOCK_SIZE; i++) {
        sprintf(hash_hex_buffer + (i * 2), "%02x", hash_bytes[i]);
    }
    hash_hex_buffer[HASH_HEX_LEN - 1] = '\0';
}

/**
 * Registers a new user.
 */
int registerUser(const char* username, const char* password) {
    if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
        printf("Error: Username and password cannot be empty.\n");
        return 0;
    }
    if (userCount >= MAX_USERS) {
        printf("Registration failed: Database is full.\n");
        return 0;
    }
    // Check if user exists
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i]->username, username) == 0) {
            printf("Registration failed: Username '%s' already exists.\n", username);
            return 0;
        }
    }

    // Create new user data
    UserData* newUser = (UserData*)malloc(sizeof(UserData));
    if (newUser == NULL) {
        printf("Memory allocation failed.\n");
        return 0;
    }
    newUser->username = (char*)malloc(strlen(username) + 1);
    if (newUser->username == NULL) {
        free(newUser);
        printf("Memory allocation failed.\n");
        return 0;
    }
    strcpy(newUser->username, username);

    generateSalt(newUser->salt);
    hashPassword(password, newUser->salt, newUser->hashedPassword);

    userDatabase[userCount++] = newUser;
    printf("User '%s' registered successfully.\n", username);
    return 1;
}

void cleanupDatabase() {
    for (int i = 0; i < userCount; i++) {
        free(userDatabase[i]->username);
        free(userDatabase[i]);
    }
}

int main() {
    // Seed the random number generator
    srand((unsigned int)time(NULL));

    printf("--- Running 5 Test Cases for User Registration ---\n");

    // Test Case 1: Successful registration
    printf("\n[Test Case 1]\n");
    registerUser("alice", "Password123!");

    // Test Case 2: Successful registration of another user
    printf("\n[Test Case 2]\n");
    registerUser("bob", "SecurePass@2023");

    // Test Case 3: Attempting to register a user that already exists
    printf("\n[Test Case 3]\n");
    registerUser("alice", "AnotherPassword");

    // Test Case 4: Registration with an empty password
    printf("\n[Test Case 4]\n");
    registerUser("charlie", "");

    // Test Case 5: Registration with a different password for a new user
    printf("\n[Test Case 5]\n");
    registerUser("dave", "Password123!");

    // Optional: Print database state to verify
    printf("\n--- Final Database State ---\n");
    for (int i = 0; i < userCount; i++) {
        printf("Username: %s\n", userDatabase[i]->username);
        printf("Salt: %s\n", userDatabase[i]->salt);
        printf("Hashed Password: %s\n", userDatabase[i]->hashedPassword);
        printf("--------------------------\n");
    }

    // Clean up allocated memory
    cleanupDatabase();

    return 0;
}