#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// --- Start of Single-File SHA256 Implementation ---
// Source: https://github.com/B-Con/crypto-algorithms (Public Domain)
#ifndef SHA256_H
#define SHA256_H

/**************************** DATA TYPES ****************************/
typedef unsigned char BYTE;             // 8-bit byte
typedef unsigned int  WORD;             // 32-bit word, change to "long" for 16-bit machines

typedef struct {
	BYTE data[64];
	WORD datalen;
	unsigned long long bitlen;
	WORD state[8];
} SHA256_CTX;

/*********************** FUNCTION DECLARATIONS **********************/
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len);
void sha256_final(SHA256_CTX *ctx, BYTE hash[]);

#endif // SHA256_H

/****************************** MACROS ******************************/
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

/**************************** VARIABLES *****************************/
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

/*********************** FUNCTION DEFINITIONS ***********************/
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
		h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
	}

	ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
	ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx) {
	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85; ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c; ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len) {
	size_t i;
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
// --- End of Single-File SHA256 Implementation ---


/**
 * !!!!!!!!!!!!!!!!!!!!!!!!!!! SECURITY WARNING !!!!!!!!!!!!!!!!!!!!!!!!!!!
 * This function is for DEMONSTRATION PURPOSES ONLY.
 * Using a fast hashing algorithm like SHA-256 directly for passwords is INSECURE.
 * It is vulnerable to brute-force and dictionary attacks.
 * In a real application, you MUST use a key derivation function (KDF) designed
 * for password hashing, such as Argon2, scrypt, or PBKDF2.
 * These functions are slow and memory-hard, which makes attacks much more difficult.
 * Use a well-vetted cryptography library like OpenSSL or libsodium to implement this.
 *
 * The salt generation below uses /dev/urandom and is suitable for Linux/macOS.
 * It is NOT portable to Windows. A production system needs a cross-platform
 * cryptographic random number generator.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
char* hashPassword(const char* password) {
    const int SALT_SIZE = 16;
    const int HASH_SIZE = 32;
    BYTE salt[SALT_SIZE];

    // 1. Generate a random salt
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) {
        perror("Failed to open /dev/urandom");
        // Fallback for non-POSIX systems for demonstration. NOT SECURE!
        // In a real app, use a proper crypto library for random numbers.
        for(int i = 0; i < SALT_SIZE; ++i) salt[i] = rand();
    } else {
        fread(salt, 1, SALT_SIZE, f);
        fclose(f);
    }

    // 2. Combine password and salt
    size_t pass_len = strlen(password);
    size_t to_hash_len = pass_len + SALT_SIZE;
    BYTE* to_hash = malloc(to_hash_len);
    if (!to_hash) return NULL;
    memcpy(to_hash, password, pass_len);
    memcpy(to_hash + pass_len, salt, SALT_SIZE);
    
    // 3. Hash the combination
    BYTE hash[HASH_SIZE];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, to_hash, to_hash_len);
    sha256_final(&ctx, hash);
    free(to_hash);

    // 4. Format for storage (hex(salt):hex(hash))
    // 2 chars per byte for hex, 1 for ':', 1 for null terminator
    char* result = malloc(SALT_SIZE * 2 + 1 + HASH_SIZE * 2 + 1);
    if (!result) return NULL;

    char* ptr = result;
    for (int i = 0; i < SALT_SIZE; i++) {
        ptr += sprintf(ptr, "%02x", salt[i]);
    }
    *ptr++ = ':';
    for (int i = 0; i < HASH_SIZE; i++) {
        ptr += sprintf(ptr, "%02x", hash[i]);
    }
    *ptr = '\0';

    return result;
}

void registerUser(const char* username, const char* password) {
    printf("Registering user: %s\n", username);
    char* hashedPassword = hashPassword(password);
    if (hashedPassword) {
        printf("Stored information for %s:\n", username);
        printf("  Username: %s\n", username);
        printf("  Hashed Password (salt:hash): %s\n", hashedPassword);
        free(hashedPassword); // IMPORTANT: Free the memory allocated by hashPassword
    } else {
        printf("Failed to hash password for user: %s\n", username);
    }
    printf("------------------------------------\n");
}

int main() {
    // 5 Test Cases
    registerUser("alice", "Password123!");
    registerUser("bob", "My$trongP@ssw0rd");
    registerUser("charlie", "Secr3t#Key");
    registerUser("david", "123456");
    registerUser("eve", "password");
    return 0;
}