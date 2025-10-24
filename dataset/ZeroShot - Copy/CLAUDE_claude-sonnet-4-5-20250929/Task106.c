
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

typedef struct {
    long long n;
    long long e;
    long long d;
} RSA;

long long mod_pow(long long base, long long exp, long long mod) {
    long long result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return result;
}

long long gcd(long long a, long long b) {
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

long long mod_inverse(long long a, long long m) {
    long long m0 = m, t, q;
    long long x0 = 0, x1 = 1;
    if (m == 1) return 0;
    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m;
        a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    if (x1 < 0) x1 += m0;
    return x1;
}

bool is_prime(long long n, int k) {
    if (n <= 1 || n == 4) return false;
    if (n <= 3) return true;
    
    long long d = n - 1;
    while (d % 2 == 0)
        d /= 2;
    
    for (int i = 0; i < k; i++) {
        long long a = 2 + rand() % (n - 4);
        long long x = mod_pow(a, d, n);
        
        if (x == 1 || x == n - 1)
            continue;
        
        bool composite = true;
        long long temp = d;
        while (temp != n - 1) {
            x = (x * x) % n;
            temp *= 2;
            if (x == 1) return false;
            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

long long generate_prime(int min_val, int max_val) {
    long long p;
    do {
        p = min_val + rand() % (max_val - min_val);
        if (p % 2 == 0) p++;
    } while (!is_prime(p, 5));
    return p;
}

void init_rsa(RSA* rsa) {
    srand(time(NULL));
    long long p = generate_prime(100, 500);
    long long q = generate_prime(100, 500);
    while (p == q) {
        q = generate_prime(100, 500);
    }
    rsa->n = p * q;
    long long phi = (p - 1) * (q - 1);
    rsa->e = 65537;
    while (rsa->e < phi) {
        if (gcd(rsa->e, phi) == 1)
            break;
        rsa->e++;
    }
    rsa->d = mod_inverse(rsa->e, phi);
}

long long encrypt(RSA* rsa, long long message) {
    return mod_pow(message, rsa->e, rsa->n);
}

long long decrypt(RSA* rsa, long long encrypted) {
    return mod_pow(encrypted, rsa->d, rsa->n);
}

void encrypt_string(RSA* rsa, const char* message, long long* encrypted, int* len) {
    *len = strlen(message);
    for (int i = 0; i < *len; i++) {
        encrypted[i] = encrypt(rsa, (long long)message[i]);
    }
}

void decrypt_string(RSA* rsa, const long long* encrypted, int len, char* decrypted) {
    for (int i = 0; i < len; i++) {
        decrypted[i] = (char)decrypt(rsa, encrypted[i]);
    }
    decrypted[len] = '\\0';
}

int main() {
    printf("RSA Encryption/Decryption Test Cases:\\n\\n");
    
    // Test Case 1
    printf("Test Case 1:\\n");
    RSA rsa1;
    init_rsa(&rsa1);
    char msg1[] = "Hello";
    long long encrypted1[100];
    int len1;
    encrypt_string(&rsa1, msg1, encrypted1, &len1);
    char decrypted1[100];
    decrypt_string(&rsa1, encrypted1, len1, decrypted1);
    printf("Original: %s\\n", msg1);
    printf("Decrypted: %s\\n", decrypted1);
    printf("Match: %s\\n\\n", strcmp(msg1, decrypted1) == 0 ? "true" : "false");
    
    // Test Case 2
    printf("Test Case 2:\\n");
    RSA rsa2;
    init_rsa(&rsa2);
    char msg2[] = "RSA";
    long long encrypted2[100];
    int len2;
    encrypt_string(&rsa2, msg2, encrypted2, &len2);
    char decrypted2[100];
    decrypt_string(&rsa2, encrypted2, len2, decrypted2);
    printf("Original: %s\\n", msg2);
    printf("Decrypted: %s\\n", decrypted2);
    printf("Match: %s\\n\\n", strcmp(msg2, decrypted2) == 0 ? "true" : "false");
    
    // Test Case 3
    printf("Test Case 3:\\n");
    RSA rsa3;
    init_rsa(&rsa3);
    long long num3 = 42;
    long long encrypted3 = encrypt(&rsa3, num3);
    long long decrypted3 = decrypt(&rsa3, encrypted3);
    printf("Original: %lld\\n", num3);
    printf("Decrypted: %lld\\n", decrypted3);
    printf("Match: %s\\n\\n", num3 == decrypted3 ? "true" : "false");
    
    // Test Case 4
    printf("Test Case 4:\\n");
    RSA rsa4;
    init_rsa(&rsa4);
    char msg4[] = "Test";
    long long encrypted4[100];
    int len4;
    encrypt_string(&rsa4, msg4, encrypted4, &len4);
    char decrypted4[100];
    decrypt_string(&rsa4, encrypted4, len4, decrypted4);
    printf("Original: %s\\n", msg4);
    printf("Decrypted: %s\\n", decrypted4);
    printf("Match: %s\\n\\n", strcmp(msg4, decrypted4) == 0 ? "true" : "false");
    
    // Test Case 5
    printf("Test Case 5:\\n");
    RSA rsa5;
    init_rsa(&rsa5);
    char msg5[] = "123";
    long long encrypted5[100];
    int len5;
    encrypt_string(&rsa5, msg5, encrypted5, &len5);
    char decrypted5[100];
    decrypt_string(&rsa5, encrypted5, len5, decrypted5);
    printf("Original: %s\\n", msg5);
    printf("Decrypted: %s\\n", decrypted5);
    printf("Match: %s\\n\\n", strcmp(msg5, decrypted5) == 0 ? "true" : "false");
    
    return 0;
}
