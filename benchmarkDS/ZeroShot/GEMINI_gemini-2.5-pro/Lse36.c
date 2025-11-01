#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// --- Start of SHA-256 Implementation (for demonstration) ---
// NOTE: In a production environment, you should use a well-vetted, dedicated
// cryptography library like OpenSSL or libsodium. This implementation
// is for self-contained demonstration purposes only.
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define SHA256_BLOCK_SIZE 32

typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    unsigned long long bitlen;
    uint32_t state[8];
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

void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
	uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];

	for (i=0,j=0; i < 16; ++i, j += 4)
		m[i] = (data[j] << 24) | (data[j+1] << 16) | (data[j+2] << 8) | (data[j+3]);
	for ( ; i < 64; ++i)
		m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

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
	ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85;
	ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
	uint32_t i;
	for (i=0; i < len; ++i) {
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
	uint32_t i;
	i = ctx->datalen;
	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56) ctx->data[i++] = 0x00;
	} else {
		ctx->data[i++] = 0x80;
		while (i < 64) ctx->data[i++] = 0x00;
		sha256_transform(ctx,ctx->data);
		memset(ctx->data,0,56);
	}
	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen; ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16; ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32; ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48; ctx->data[56] = ctx->bitlen >> 56;
	sha256_transform(ctx,ctx->data);

	for (i=0; i < 4; ++i) {
		hash[i]    = (ctx->state[0] >> (24-i*8)) & 0x000000ff;
		hash[i+4]  = (ctx->state[1] >> (24-i*8)) & 0x000000ff;
		hash[i+8]  = (ctx->state[2] >> (24-i*8)) & 0x000000ff;
		hash[i+12] = (ctx->state[3] >> (24-i*8)) & 0x000000ff;
		hash[i+16] = (ctx->state[4] >> (24-i*8)) & 0x000000ff;
		hash[i+20] = (ctx->state[5] >> (24-i*8)) & 0x000000ff;
		hash[i+24] = (ctx->state[6] >> (24-i*8)) & 0x000000ff;
		hash[i+28] = (ctx->state[7] >> (24-i*8)) & 0x000000ff;
	}
}
// --- End of SHA-256 Implementation ---

#define SALT_SIZE 16
#define MAX_USERNAME_LEN 50
#define DB_CAPACITY 10

typedef struct {
    char username[MAX_USERNAME_LEN];
    uint8_t salt[SALT_SIZE];
    uint8_t hash[SHA256_BLOCK_SIZE];
} UserData;

// In-memory array to simulate a user database.
static UserData userDatabase[DB_CAPACITY];
static int userCount = 0;

void generateSalt(uint8_t* salt) {
    for (int i = 0; i < SALT_SIZE; i++) {
        salt[i] = rand() % 256;
    }
}

void hashPassword(const char* password, const uint8_t* salt, uint8_t* hash_out) {
    SHA256_CTX ctx;
    size_t pass_len = strlen(password);
    size_t combined_len = pass_len + SALT_SIZE;
    uint8_t* combined = malloc(combined_len);
    if (!combined) return;

    memcpy(combined, password, pass_len);
    memcpy(combined + pass_len, salt, SALT_SIZE);

    sha256_init(&ctx);
    sha256_update(&ctx, combined, combined_len);
    sha256_final(&ctx, hash_out);
    
    free(combined);
}

bool secureCompare(const uint8_t* a, const uint8_t* b, size_t size) {
    uint8_t result = 0;
    for (size_t i = 0; i < size; ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

void registerUser(const char* username, const char* password) {
    if (userCount >= DB_CAPACITY || strlen(username) >= MAX_USERNAME_LEN) return;
    
    UserData* newUser = &userDatabase[userCount];
    strncpy(newUser->username, username, MAX_USERNAME_LEN -1);
    newUser->username[MAX_USERNAME_LEN - 1] = '\0';

    generateSalt(newUser->salt);
    hashPassword(password, newUser->salt, newUser->hash);
    
    userCount++;
}

bool login(const char* username, const char* password) {
    UserData* user = NULL;
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            user = &userDatabase[i];
            break;
        }
    }

    if (user == NULL) {
        uint8_t dummy_salt[SALT_SIZE];
        uint8_t dummy_hash[SHA256_BLOCK_SIZE];
        generateSalt(dummy_salt);
        hashPassword(password, dummy_salt, dummy_hash);
        printf("Login failed: Username does not exist.\n");
        return false;
    }

    uint8_t providedPasswordHash[SHA256_BLOCK_SIZE];
    hashPassword(password, user->salt, providedPasswordHash);

    if (secureCompare(user->hash, providedPasswordHash, SHA256_BLOCK_SIZE)) {
        printf("Login successful for %s. Redirecting to home page...\n", username);
        return true;
    } else {
        printf("Login failed: Incorrect password.\n");
        return false;
    }
}

int main() {
    // WARNING: Seeding with time is not cryptographically secure.
    // For production, use an OS-specific source of entropy like /dev/urandom.
    srand(time(NULL));

    registerUser("admin", "P@ssword123!");
    registerUser("user", "mySecurePass");
    
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Successful login
    printf("\n[Test 1] Correct username and password:\n");
    login("admin", "P@ssword123!");

    // Test Case 2: Incorrect password
    printf("\n[Test 2] Correct username, incorrect password:\n");
    login("admin", "WrongPassword");

    // Test Case 3: Non-existent username
    printf("\n[Test 3] Non-existent username:\n");
    login("guest", "SomePassword");
    
    // Test Case 4: Empty password
    printf("\n[Test 4] Correct username, empty password:\n");
    login("user", "");

    // Test Case 5: Case-sensitive username
    printf("\n[Test 5] Case-sensitive username check (should fail):\n");
    login("Admin", "P@ssword123!");

    printf("\n--- Test Cases Finished ---\n");
    
    return 0;
}