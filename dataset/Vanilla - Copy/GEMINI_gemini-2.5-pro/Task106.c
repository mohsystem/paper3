#include <stdio.h>
#include <stdlib.h>

// Use unsigned long long for a wider integer range
typedef unsigned long long ull;

/**
 * @brief Calculates (base^exp) % mod efficiently.
 */
ull power(ull base, ull exp, ull mod) {
    ull res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * base) % mod;
        base = (base * base) % mod;
        exp /= 2;
    }
    return res;
}

/**
 * @brief Finds the greatest common divisor of two numbers.
 */
ull gcd(ull a, ull b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

/**
 * @brief Extended Euclidean Algorithm to find `g = ax + by`.
 * @param x Pointer to store coefficient of a.
 * @param y Pointer to store coefficient of b.
 * @return The greatest common divisor `g`.
 */
long long extended_gcd(long long a, long long b, long long *x, long long *y) {
    if (a == 0) {
        *x = 0;
        *y = 1;
        return b;
    }
    long long x1, y1;
    long long d = extended_gcd(b % a, a, &x1, &y1);
    *x = y1 - (b / a) * x1;
    *y = x1;
    return d;
}

/**
 * @brief Finds the modular inverse of 'a' under modulo 'm'.
 */
ull modInverse(ull a, ull m) {
    long long x, y;
    long long g = extended_gcd(a, m, &x, &y);
    if (g != 1) {
        // Modular inverse does not exist
        return -1;
    }
    // m is added to handle negative x
    return (x % (long long)m + (long long)m) % (long long)m;
}

/**
 * @brief Generates public and private keys.
 */
void generate_keys(ull p, ull q, ull* e, ull* d, ull* n) {
    *n = p * q;
    ull phi = (p - 1) * (q - 1);

    // Choose e
    *e = 17; // Common choice for e
    while (gcd(*e, phi) != 1) {
        (*e)++;
    }

    // Calculate d
    *d = modInverse(*e, phi);
}

/**
 * @brief Encrypts a message.
 */
ull encrypt(ull msg, ull e, ull n) {
    return power(msg, e, n);
}

/**
 * @brief Decrypts a message.
 */
ull decrypt(ull encrypted_msg, ull d, ull n) {
    return power(encrypted_msg, d, n);
}

int main() {
    ull p = 61;
    ull q = 53;

    ull n, e, d;
    generate_keys(p, q, &e, &d, &n);

    printf("Public Key (e, n): (%llu, %llu)\n", e, n);
    printf("Private Key (d, n): (%llu, %llu)\n", d, n);
    printf("----------------------------------------\n");

    ull test_messages[] = {123, 89, 999, 2500, 3000};
    int num_tests = sizeof(test_messages) / sizeof(test_messages[0]);

    for (int i = 0; i < num_tests; ++i) {
        ull msg = test_messages[i];
        printf("Test Case %d\n", i + 1);
        printf("Original Message: %llu\n", msg);
        
        ull encrypted_msg = encrypt(msg, e, n);
        printf("Encrypted Message: %llu\n", encrypted_msg);

        ull decrypted_msg = decrypt(encrypted_msg, d, n);
        printf("Decrypted Message: %llu\n", decrypted_msg);
        printf("----------------------------------------\n");
    }

    return 0;
}