#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

static inline uint64_t mul_mod_u64(uint64_t a, uint64_t b, uint64_t mod) {
    __int128 t = ( (__int128)a * (__int128)b );
    return (uint64_t)(t % mod);
}

uint64_t modexp_u64(uint64_t base, uint64_t exp, uint64_t mod) {
    base %= mod;
    uint64_t result = 1;
    while (exp > 0) {
        if (exp & 1) result = mul_mod_u64(result, base, mod);
        base = mul_mod_u64(base, base, mod);
        exp >>= 1;
    }
    return result;
}

uint64_t encrypt_rsa_u64(uint64_t message, uint64_t e, uint64_t n) {
    return modexp_u64(message, e, n);
}

uint64_t decrypt_rsa_u64(uint64_t ciphertext, uint64_t d, uint64_t n) {
    return modexp_u64(ciphertext, d, n);
}

int main(void) {
    uint64_t n = 3233ULL;
    uint64_t e = 17ULL;
    uint64_t d = 2753ULL;

    uint64_t messages[5] = {65ULL, 42ULL, 123ULL, 999ULL, 2021ULL};

    for (int i = 0; i < 5; ++i) {
        uint64_t m = messages[i];
        uint64_t c = encrypt_rsa_u64(m, e, n);
        uint64_t p = decrypt_rsa_u64(c, d, n);
        printf("Message: %" PRIu64 " -> Cipher: %" PRIu64 " -> Decrypted: %" PRIu64 "\n", m, c, p);
    }
    return 0;
}