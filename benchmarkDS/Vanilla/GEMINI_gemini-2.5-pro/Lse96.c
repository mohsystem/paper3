#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

// --- Start of embedded SHA256 implementation ---
// A public domain, compact, C implementation of SHA-256.
// Original source: https://github.com/B-Con/crypto-algorithms (by Brad Conte)
#define SHA256_BLOCK_SIZE 32

typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    unsigned long long bitlen;
    uint32_t state[8];
} SHA256_CTX;

void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len);
void sha256_final(SHA256_CTX *ctx, uint8_t hash[]);

static const uint32_t k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define ROTR(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define Ch(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22))
#define EP1(x) (ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25))
#define SIG0(x) (ROTR(x,7) ^ ROTR(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x,17) ^ ROTR(x,19) ^ ((x) >> 10))

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
	uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

	for (i = 0, j = 0; i < 16; ++i, j += 4)
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
	for ( ; i < 64; ++i)
		m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

	a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
	e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

	for (i = 0; i < 64; ++i) {
		t1 = h + EP1(e) + Ch(e,f,g) + k[i] + m[i];
		t2 = EP0(a) + Maj(a,b,c);
		h = g; g = f; f = e; e = d + t1;
		d = c; c = b; b = a; a = t1 + t2;
	}
	ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
	ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx) {
	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85; ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a; ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
	for (size_t i = 0; i < len; ++i) {
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64) {
			sha256_transform(ctx, ctx->data);
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}
}

void sha256_final(SHA256_CTX *ctx, uint8_t hash[]) {
	uint32_t i = ctx->datalen;
	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56) ctx->data[i++] = 0x00;
	} else {
		ctx->data[i++] = 0x80;
		while (i < 64) ctx->data[i++] = 0x00;
		sha256_transform(ctx, ctx->data);
		memset(ctx->data, 0, 56);
	}
	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen;
	ctx->data[62] = ctx->bitlen >> 8; ctx->data[61] = ctx->bitlen >> 16;
	ctx->data[60] = ctx->bitlen >> 24; ctx->data[59] = ctx->bitlen >> 32;
	ctx->data[58] = ctx->bitlen >> 40; ctx->data[57] = ctx->bitlen >> 48;
	ctx->data[56] = ctx->bitlen >> 56;
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
// --- End of embedded SHA256 implementation ---

#define MAX_USERS 10
#define USERNAME_MAX_LEN 50
#define SALT_LEN 16
#define STORED_VALUE_MAX_LEN (SALT_LEN * 2 + SHA256_BLOCK_SIZE * 2 + 1)

typedef struct {
    char username[USERNAME_MAX_LEN];
    char stored_value[STORED_VALUE_MAX_LEN];
} User;

static User userDatabase[MAX_USERS];
static int userCount = 0;

void bytes_to_hex(const uint8_t* bytes, size_t len, char* hex_str) {
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
}

void generate_salt(uint8_t* salt, size_t len) {
    // NOTE: This is NOT cryptographically secure. For a real application,
    // use a proper CSPRNG like /dev/urandom on Linux or CryptGenRandom on Windows.
    for (size_t i = 0; i < len; ++i) {
        salt[i] = rand() % 256;
    }
}

void hash_password(const char* password, const uint8_t* salt, uint8_t* hash_out) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    // Combine salt and password before hashing
    sha256_update(&ctx, salt, SALT_LEN);
    sha256_update(&ctx, (const uint8_t*)password, strlen(password));
    sha256_final(&ctx, hash_out);
}

const char* registerUser(const char* username, const char* password) {
    if (userCount >= MAX_USERS) {
        return "Error: Database is full.";
    }
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            return "Error: Username already exists.";
        }
    }
    if (password == NULL || strlen(password) == 0) {
        return "Error: Password cannot be empty.";
    }

    uint8_t salt_bytes[SALT_LEN];
    generate_salt(salt_bytes, SALT_LEN);
    
    uint8_t hash_output[SHA256_BLOCK_SIZE];
    hash_password(password, salt_bytes, hash_output);

    char salt_hex[SALT_LEN * 2 + 1];
    char hash_hex[SHA256_BLOCK_SIZE * 2 + 1];
    bytes_to_hex(salt_bytes, SALT_LEN, salt_hex);
    bytes_to_hex(hash_output, SHA256_BLOCK_SIZE, hash_hex);

    User* newUser = &userDatabase[userCount];
    strncpy(newUser->username, username, USERNAME_MAX_LEN - 1);
    newUser->username[USERNAME_MAX_LEN - 1] = '\0';
    
    strcpy(newUser->stored_value, salt_hex);
    strcat(newUser->stored_value, hash_hex);
    
    userCount++;
    return newUser->stored_value;
}

int main() {
    // Seed the random number generator (for salt generation)
    srand(time(NULL));

    printf("--- C Registration Simulation ---\n");

    // Test Case 1
    printf("\n1) Registering user 'alice'...\n");
    const char* aliceData = registerUser("alice", "password123");
    printf("   Stored data for alice: %s\n", aliceData);

    // Test Case 2
    printf("\n2) Registering user 'bob'...\n");
    const char* bobData = registerUser("bob", "mySecret!@#");
    printf("   Stored data for bob: %s\n", bobData);

    // Test Case 3
    printf("\n3) Registering user 'charlie'...\n");
    const char* charlieData = registerUser("charlie", "P@ssw0rd_");
    printf("   Stored data for charlie: %s\n", charlieData);
    
    // Test Case 4
    printf("\n4) Attempting to register 'alice' again...\n");
    const char* aliceDuplicateData = registerUser("alice", "anotherPassword");
    printf("   Result: %s\n", aliceDuplicateData);
    
    // Test Case 5
    printf("\n5) Registering user 'dave'...\n");
    const char* daveData = registerUser("dave", "davespassword");
    printf("   Stored data for dave: %s\n", daveData);

    printf("\n--- Final Database State ---\n");
    for (int i = 0; i < userCount; ++i) {
        printf("User: %s, Stored: %s\n", userDatabase[i].username, userDatabase[i].stored_value);
    }
    
    return 0;
}