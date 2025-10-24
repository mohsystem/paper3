
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

typedef struct {
    long long n;
    long long e;
    long long d;
} RSA;

long long gcd(long long a, long long b) {
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

long long mod_pow(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = ((__int128)result * base) % mod;
        }
        base = ((__int128)base * base) % mod;
        exp /= 2;
    }
    return result;
}

int is_prime(long long n, int k) {
    if (n < 2) return 0;
    if (n == 2 || n == 3) return 1;
    if (n % 2 == 0) return 0;
    
    long long d = n - 1;
    int r = 0;
    while (d % 2 == 0) {
        r++;
        d /= 2;
    }
    
    for (int i = 0; i < k; i++) {
        long long a = 2 + rand() % (n - 3);
        long long x = mod_pow(a, d, n);
        
        if (x == 1 || x == n - 1) continue;
        
        int composite = 1;
        for (int j = 0; j < r - 1; j++) {
            x = ((__int128)x * x) % n;
            if (x == n - 1) {
                composite = 0;
                break;
            }
        }
        if (composite) return 0;
    }
    return 1;
}

long long generate_prime(int bits) {
    long long min = 1LL << (bits - 1);
    long long max = (1LL << bits) - 1;
    
    while (1) {
        long long num = min + rand() % (max - min);
        if (num % 2 == 0) num++;
        if (is_prime(num, 5)) return num;
    }
}

long long mod_inverse(long long a, long long m) {
    long long m0 = m, x0 = 0, x1 = 1;
    if (m == 1) return 0;
    
    while (a > 1) {
        long long q = a / m;
        long long t = m;
        m = a % m;
        a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    
    if (x1 < 0) x1 += m0;
    return x1;
}

void init_rsa(RSA* rsa, int bit_length) {
    long long p = generate_prime(bit_length / 2);
    long long q = generate_prime(bit_length / 2);
    
    rsa->n = p * q;
    long long phi = (p - 1) * (q - 1);
    
    rsa->e = 65537;
    if (rsa->e > phi) rsa->e = 3;
    while (gcd(rsa->e, phi) != 1) {
        rsa->e += 2;
    }
    
    rsa->d = mod_inverse(rsa->e, phi);
}

long long encrypt(RSA* rsa, long long message) {
    return mod_pow(message, rsa->e, rsa->n);
}

long long decrypt(RSA* rsa, long long encrypted) {
    return mod_pow(encrypted, rsa->d, rsa->n);
}

long long string_to_number(const char* str) {
    long long num = 0;
    for (int i = 0; str[i] != '\\0'; i++) {
        num = num * 256 + (unsigned char)str[i];
    }
    return num;
}

void number_to_string(long long num, char* str) {
    int len = 0;
    long long temp = num;
    while (temp > 0) {
        len++;
        temp /= 256;
    }
    
    str[len] = '\\0';
    for (int i = len - 1; i >= 0; i--) {
        str[i] = (char)(num % 256);
        num /= 256;
    }
}

int main() {
    srand(time(NULL));
    
    printf("RSA Encryption/Decryption Test Cases:\\n");
    printf("=====================================\\n\\n");
    
    // Test Case 1
    printf("Test Case 1: Simple message\\n");
    RSA rsa1;
    init_rsa(&rsa1, 20);
    char message1[] = "Hello";
    long long msg1 = string_to_number(message1);
    long long enc1 = encrypt(&rsa1, msg1);
    long long dec1 = decrypt(&rsa1, enc1);
    char result1[100];
    number_to_string(dec1, result1);
    printf("Original: %s\\n", message1);
    printf("Encrypted: %lld\\n", enc1);
    printf("Decrypted: %s\\n", result1);
    printf("Match: %s\\n\\n", strcmp(message1, result1) == 0 ? "true" : "false");
    
    // Test Case 2
    printf("Test Case 2: Short text\\n");
    RSA rsa2;
    init_rsa(&rsa2, 20);
    char message2[] = "RSA";
    long long msg2 = string_to_number(message2);
    long long enc2 = encrypt(&rsa2, msg2);
    long long dec2 = decrypt(&rsa2, enc2);
    char result2[100];
    number_to_string(dec2, result2);
    printf("Original: %s\\n", message2);
    printf("Encrypted: %lld\\n", enc2);
    printf("Decrypted: %s\\n", result2);
    printf("Match: %s\\n\\n", strcmp(message2, result2) == 0 ? "true" : "false");
    
    // Test Case 3
    printf("Test Case 3: Numbers\\n");
    RSA rsa3;
    init_rsa(&rsa3, 20);
    char message3[] = "123";
    long long msg3 = string_to_number(message3);
    long long enc3 = encrypt(&rsa3, msg3);
    long long dec3 = decrypt(&rsa3, enc3);
    char result3[100];
    number_to_string(dec3, result3);
    printf("Original: %s\\n", message3);
    printf("Encrypted: %lld\\n", enc3);
    printf("Decrypted: %s\\n", result3);
    printf("Match: %s\\n\\n", strcmp(message3, result3) == 0 ? "true" : "false");
    
    // Test Case 4
    printf("Test Case 4: Special char\\n");
    RSA rsa4;
    init_rsa(&rsa4, 20);
    char message4[] = "Hi!";
    long long msg4 = string_to_number(message4);
    long long enc4 = encrypt(&rsa4, msg4);
    long long dec4 = decrypt(&rsa4, enc4);
    char result4[100];
    number_to_string(dec4, result4);
    printf("Original: %s\\n", message4);
    printf("Encrypted: %lld\\n", enc4);
    printf("Decrypted: %s\\n", result4);
    printf("Match: %s\\n\\n", strcmp(message4, result4) == 0 ? "true" : "false");
    
    // Test Case 5
    printf("Test Case 5: Single character\\n");
    RSA rsa5;
    init_rsa(&rsa5, 20);
    char message5[] = "A";
    long long msg5 = string_to_number(message5);
    long long enc5 = encrypt(&rsa5, msg5);
    long long dec5 = decrypt(&rsa5, enc5);
    char result5[100];
    number_to_string(dec5, result5);
    printf("Original: %s\\n", message5);
    printf("Encrypted: %lld\\n", enc5);
    printf("Decrypted: %s\\n", result5);
    printf("Match: %s\\n\\n", strcmp(message5, result5) == 0 ? "true" : "false");
    
    return 0;
}
