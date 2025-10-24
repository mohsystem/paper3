#include <iostream>

// Use long long for a wider integer range to prevent overflow with small primes
using int_type = long long;

/**
 * @brief Calculates (base^exp) % mod efficiently.
 * @param base The base of the power.
 * @param exp The exponent.
 * @param mod The modulus.
 * @return The result of (base^exp) % mod.
 */
int_type power(int_type base, int_type exp, int_type mod) {
    int_type res = 1;
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
int_type gcd(int_type a, int_type b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

/**
 * @brief Finds the modular inverse of 'a' under modulo 'm'.
 * Uses the extended Euclidean algorithm.
 */
int_type modInverse(int_type a, int_type m) {
    int_type m0 = m;
    int_type y = 0, x = 1;

    if (m == 1) return 0;

    while (a > 1) {
        int_type q = a / m;
        int_type t = m;
        m = a % m, a = t;
        t = y;
        y = x - q * y;
        x = t;
    }

    if (x < 0) x += m0;

    return x;
}

// Struct to hold the key pair for convenience
struct KeyPair {
    int_type e, d, n;
};

/**
 * @brief Generates public and private keys.
 */
KeyPair generate_keys(int_type p, int_type q) {
    KeyPair keys;
    keys.n = p * q;
    int_type phi = (p - 1) * (q - 1);
    
    // Choose e
    keys.e = 17; // Common choice for e
    while (gcd(keys.e, phi) != 1) {
        keys.e++;
    }

    // Calculate d
    keys.d = modInverse(keys.e, phi);
    return keys;
}

/**
 * @brief Encrypts a message.
 */
int_type encrypt(int_type msg, int_type e, int_type n) {
    return power(msg, e, n);
}

/**
 * @brief Decrypts a message.
 */
int_type decrypt(int_type encrypted_msg, int_type d, int_type n) {
    return power(encrypted_msg, d, n);
}

int main() {
    int_type p = 61;
    int_type q = 53;

    KeyPair keys = generate_keys(p, q);

    std::cout << "Public Key (e, n): (" << keys.e << ", " << keys.n << ")" << std::endl;
    std::cout << "Private Key (d, n): (" << keys.d << ", " << keys.n << ")" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    int_type test_messages[] = {123, 89, 999, 2500, 3000};
    int num_tests = sizeof(test_messages) / sizeof(test_messages[0]);

    for (int i = 0; i < num_tests; ++i) {
        int_type msg = test_messages[i];
        std::cout << "Test Case " << i + 1 << std::endl;
        std::cout << "Original Message: " << msg << std::endl;
        
        int_type encrypted_msg = encrypt(msg, keys.e, keys.n);
        std::cout << "Encrypted Message: " << encrypted_msg << std::endl;

        int_type decrypted_msg = decrypt(encrypted_msg, keys.d, keys.n);
        std::cout << "Decrypted Message: " << decrypted_msg << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }

    return 0;
}