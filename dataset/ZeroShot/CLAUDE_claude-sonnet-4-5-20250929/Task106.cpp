
#include <iostream>
#include <string>
#include <random>
#include <ctime>
#include <cmath>
#include <vector>

using namespace std;

class Task106 {
private:
    long long n, d, e;
    
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
    
    bool is_prime(long long n, int k = 5) {
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
        } while (!is_prime(p));
        return p;
    }

public:
    Task106() {
        srand(time(0));
        long long p = generate_prime(100, 500);
        long long q = generate_prime(100, 500);
        while (p == q) {
            q = generate_prime(100, 500);
        }
        n = p * q;
        long long phi = (p - 1) * (q - 1);
        e = 65537;
        while (e < phi) {
            if (gcd(e, phi) == 1)
                break;
            e++;
        }
        d = mod_inverse(e, phi);
    }
    
    long long encrypt(long long message) {
        return mod_pow(message, e, n);
    }
    
    long long decrypt(long long encrypted) {
        return mod_pow(encrypted, d, n);
    }
    
    vector<long long> encrypt_string(const string& message) {
        vector<long long> encrypted;
        for (char c : message) {
            encrypted.push_back(encrypt((long long)c));
        }
        return encrypted;
    }
    
    string decrypt_string(const vector<long long>& encrypted) {
        string decrypted;
        for (long long enc : encrypted) {
            decrypted += (char)decrypt(enc);
        }
        return decrypted;
    }
};

int main() {
    cout << "RSA Encryption/Decryption Test Cases:\\n\\n";
    
    // Test Case 1
    cout << "Test Case 1:\\n";
    Task106 rsa1;
    string msg1 = "Hello";
    vector<long long> encrypted1 = rsa1.encrypt_string(msg1);
    string decrypted1 = rsa1.decrypt_string(encrypted1);
    cout << "Original: " << msg1 << "\\n";
    cout << "Decrypted: " << decrypted1 << "\\n";
    cout << "Match: " << (msg1 == decrypted1 ? "true" : "false") << "\\n\\n";
    
    // Test Case 2
    cout << "Test Case 2:\\n";
    Task106 rsa2;
    string msg2 = "RSA";
    vector<long long> encrypted2 = rsa2.encrypt_string(msg2);
    string decrypted2 = rsa2.decrypt_string(encrypted2);
    cout << "Original: " << msg2 << "\\n";
    cout << "Decrypted: " << decrypted2 << "\\n";
    cout << "Match: " << (msg2 == decrypted2 ? "true" : "false") << "\\n\\n";
    
    // Test Case 3
    cout << "Test Case 3:\\n";
    Task106 rsa3;
    long long num3 = 42;
    long long encrypted3 = rsa3.encrypt(num3);
    long long decrypted3 = rsa3.decrypt(encrypted3);
    cout << "Original: " << num3 << "\\n";
    cout << "Decrypted: " << decrypted3 << "\\n";
    cout << "Match: " << (num3 == decrypted3 ? "true" : "false") << "\\n\\n";
    
    // Test Case 4
    cout << "Test Case 4:\\n";
    Task106 rsa4;
    string msg4 = "Test";
    vector<long long> encrypted4 = rsa4.encrypt_string(msg4);
    string decrypted4 = rsa4.decrypt_string(encrypted4);
    cout << "Original: " << msg4 << "\\n";
    cout << "Decrypted: " << decrypted4 << "\\n";
    cout << "Match: " << (msg4 == decrypted4 ? "true" : "false") << "\\n\\n";
    
    // Test Case 5
    cout << "Test Case 5:\\n";
    Task106 rsa5;
    string msg5 = "123";
    vector<long long> encrypted5 = rsa5.encrypt_string(msg5);
    string decrypted5 = rsa5.decrypt_string(encrypted5);
    cout << "Original: " << msg5 << "\\n";
    cout << "Decrypted: " << decrypted5 << "\\n";
    cout << "Match: " << (msg5 == decrypted5 ? "true" : "false") << "\\n\\n";
    
    return 0;
}
