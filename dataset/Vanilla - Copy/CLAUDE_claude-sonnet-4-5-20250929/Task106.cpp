
#include <iostream>
#include <string>
#include <random>
#include <cmath>
#include <vector>
#include <sstream>

using namespace std;

class Task106 {
private:
    long long n, d, e;
    
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
                result = (__int128)result * base % mod;
            }
            base = (__int128)base * base % mod;
            exp /= 2;
        }
        return result;
    }
    
    bool is_prime(long long n, int k = 5) {
        if (n < 2) return false;
        if (n == 2 || n == 3) return true;
        if (n % 2 == 0) return false;
        
        long long d = n - 1;
        int r = 0;
        while (d % 2 == 0) {
            r++;
            d /= 2;
        }
        
        random_device rd;
        mt19937_64 gen(rd());
        uniform_int_distribution<long long> dis(2, n - 2);
        
        for (int i = 0; i < k; i++) {
            long long a = dis(gen);
            long long x = mod_pow(a, d, n);
            
            if (x == 1 || x == n - 1) continue;
            
            bool composite = true;
            for (int j = 0; j < r - 1; j++) {
                x = (__int128)x * x % n;
                if (x == n - 1) {
                    composite = false;
                    break;
                }
            }
            if (composite) return false;
        }
        return true;
    }
    
    long long generate_prime(int bits) {
        random_device rd;
        mt19937_64 gen(rd());
        uniform_int_distribution<long long> dis(1LL << (bits - 1), (1LL << bits) - 1);
        
        while (true) {
            long long num = dis(gen);
            if (num % 2 == 0) num++;
            if (is_prime(num)) return num;
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
    
public:
    Task106(int bit_length = 20) {
        long long p = generate_prime(bit_length / 2);
        long long q = generate_prime(bit_length / 2);
        
        n = p * q;
        long long phi = (p - 1) * (q - 1);
        
        e = 65537;
        if (e > phi) e = 3;
        while (gcd(e, phi) != 1) {
            e += 2;
        }
        
        d = mod_inverse(e, phi);
    }
    
    long long encrypt(long long message) {
        return mod_pow(message, e, n);
    }
    
    long long decrypt(long long encrypted) {
        return mod_pow(encrypted, d, n);
    }
    
    string encrypt_string(const string& message) {
        long long msg = 0;
        for (char c : message) {
            msg = msg * 256 + (unsigned char)c;
        }
        long long encrypted = encrypt(msg);
        return to_string(encrypted);
    }
    
    string decrypt_string(const string& encrypted_message) {
        long long encrypted = stoll(encrypted_message);
        long long decrypted = decrypt(encrypted);
        
        string result = "";
        while (decrypted > 0) {
            result = (char)(decrypted % 256) + result;
            decrypted /= 256;
        }
        return result;
    }
};

int main() {
    cout << "RSA Encryption/Decryption Test Cases:" << endl;
    cout << "=====================================" << endl << endl;
    
    // Test Case 1
    cout << "Test Case 1: Simple message" << endl;
    Task106 rsa1(20);
    string message1 = "Hello";
    string encrypted1 = rsa1.encrypt_string(message1);
    string decrypted1 = rsa1.decrypt_string(encrypted1);
    cout << "Original: " << message1 << endl;
    cout << "Encrypted: " << encrypted1 << endl;
    cout << "Decrypted: " << decrypted1 << endl;
    cout << "Match: " << (message1 == decrypted1 ? "true" : "false") << endl << endl;
    
    // Test Case 2
    cout << "Test Case 2: Short text" << endl;
    Task106 rsa2(20);
    string message2 = "RSA";
    string encrypted2 = rsa2.encrypt_string(message2);
    string decrypted2 = rsa2.decrypt_string(encrypted2);
    cout << "Original: " << message2 << endl;
    cout << "Encrypted: " << encrypted2 << endl;
    cout << "Decrypted: " << decrypted2 << endl;
    cout << "Match: " << (message2 == decrypted2 ? "true" : "false") << endl << endl;
    
    // Test Case 3
    cout << "Test Case 3: Numbers" << endl;
    Task106 rsa3(20);
    string message3 = "123";
    string encrypted3 = rsa3.encrypt_string(message3);
    string decrypted3 = rsa3.decrypt_string(encrypted3);
    cout << "Original: " << message3 << endl;
    cout << "Encrypted: " << encrypted3 << endl;
    cout << "Decrypted: " << decrypted3 << endl;
    cout << "Match: " << (message3 == decrypted3 ? "true" : "false") << endl << endl;
    
    // Test Case 4
    cout << "Test Case 4: Special char" << endl;
    Task106 rsa4(20);
    string message4 = "Hi!";
    string encrypted4 = rsa4.encrypt_string(message4);
    string decrypted4 = rsa4.decrypt_string(encrypted4);
    cout << "Original: " << message4 << endl;
    cout << "Encrypted: " << encrypted4 << endl;
    cout << "Decrypted: " << decrypted4 << endl;
    cout << "Match: " << (message4 == decrypted4 ? "true" : "false") << endl << endl;
    
    // Test Case 5
    cout << "Test Case 5: Single character" << endl;
    Task106 rsa5(20);
    string message5 = "A";
    string encrypted5 = rsa5.encrypt_string(message5);
    string decrypted5 = rsa5.decrypt_string(encrypted5);
    cout << "Original: " << message5 << endl;
    cout << "Encrypted: " << encrypted5 << endl;
    cout << "Decrypted: " << decrypted5 << endl;
    cout << "Match: " << (message5 == decrypted5 ? "true" : "false") << endl << endl;
    
    return 0;
}
