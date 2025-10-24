#include <bits/stdc++.h>
using namespace std;

// Step 1: Problem understanding - RSA basic components for 64-bit demo

using u64 = uint64_t;
using u128 = __uint128_t;

static u64 modmul(u64 a, u64 b, u64 m) {
    return (u128)a * b % m;
}

static u64 modexp(u64 a, u64 e, u64 m) {
    u64 res = 1 % m;
    a %= m;
    while (e) {
        if (e & 1) res = modmul(res, a, m);
        a = modmul(a, a, m);
        e >>= 1;
    }
    return res;
}

static u64 gcd_u64(u64 a, u64 b) {
    while (b) { u64 t = a % b; a = b; b = t; }
    return a;
}

static long long egcd(long long a, long long b, long long &x, long long &y) {
    if (b == 0) { x = 1; y = 0; return a; }
    long long x1, y1;
    long long g = egcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

static bool modinv(u64 a, u64 m, u64 &inv) {
    long long x, y;
    long long g = egcd((long long)a, (long long)m, x, y);
    if (g != 1) return false;
    long long res = x % (long long)m;
    if (res < 0) res += (long long)m;
    inv = (u64)res;
    return true;
}

static bool isPrime(u64 n) {
    if (n < 2) return false;
    for (u64 p : {2ULL,3ULL,5ULL,7ULL,11ULL,13ULL,17ULL,19ULL,23ULL,29ULL,31ULL,37ULL}) {
        if (n % p == 0) return n == p;
    }
    u64 d = n - 1, s = 0;
    while ((d & 1) == 0) { d >>= 1; ++s; }
    static const u64 bases[] = {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL};
    for (u64 a : bases) {
        if (a % n == 0) continue;
        u64 x = modexp(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool cont = false;
        for (u64 r = 1; r < s; ++r) {
            x = modmul(x, x, n);
            if (x == n - 1) { cont = true; break; }
        }
        if (!cont) return false;
    }
    return true;
}

static u64 random_u64_secure() {
    // Step 2: Security - use non-deterministic source
    std::random_device rd;
    u64 v = 0;
    for (int i = 0; i < 5; ++i) { // accumulate entropy
        v ^= ((u64)rd()) << ((i % 2) ? 32 : 0);
        v = std::rotl(v, 13);
    }
    return v ^ ((u64)rd() << 32) ^ (u64)rd();
}

static u64 random_bits(int bits) {
    u64 x = 0;
    int produced = 0;
    while (produced < bits) {
        x <<= 32;
        x ^= (u64)random_u64_secure();
        produced += 32;
    }
    if (bits < 64) {
        x &= ((1ULL << bits) - 1ULL);
    }
    x |= (1ULL << (bits - 1)); // ensure MSB
    x |= 1ULL; // odd
    return x;
}

static u64 gen_prime_32() {
    while (true) {
        u64 cand = random_bits(32);
        if (isPrime(cand)) return cand;
    }
}

struct RSAKey {
    u64 n, e, d;
};

static RSAKey generateKeys() {
    // Step 3: Secure coding - using e=65537 and ensuring gcd(e,phi)=1
    const u64 e = 65537ULL;
    while (true) {
        u64 p = gen_prime_32();
        u64 q = gen_prime_32();
        if (p == q) continue;
        u64 n = p * q;
        u64 phi = (p - 1) * (q - 1);
        if (gcd_u64(e, phi) != 1) continue;
        u64 d;
        if (!modinv(e, phi, d)) continue;
        return RSAKey{n, e, d};
    }
}

static u64 encrypt(u64 m, u64 e, u64 n) {
    if (m >= n) throw std::runtime_error("Message out of range");
    return modexp(m, e, n);
}

static u64 decrypt(u64 c, u64 d, u64 n) {
    if (c >= n) throw std::runtime_error("Ciphertext out of range");
    return modexp(c, d, n);
}

static u64 string_to_u64(const string& s) {
    u64 v = 0;
    for (unsigned char ch : s) {
        v = (v << 8) | (u64)ch;
    }
    return v;
}

static string u64_to_string(u64 x) {
    if (x == 0) return string();
    string out;
    while (x > 0) {
        unsigned char b = (unsigned char)(x & 0xFF);
        out.push_back((char)b);
        x >>= 8;
    }
    reverse(out.begin(), out.end());
    return out;
}

// Step 4+5: Review and final tests
int main() {
    RSAKey key = generateKeys();

    vector<string> tests = {
        "A",
        "Hi",
        "Cat",
        "Test",
        "Hello"
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        string msg = tests[i];
        u64 m = string_to_u64(msg);
        if (m >= key.n) {
            cout << "Test " << (i+1) << ": message too large for modulus\n";
            continue;
        }
        u64 c = encrypt(m, key.e, key.n);
        u64 p = decrypt(c, key.d, key.n);
        string rec = u64_to_string(p);
        cout << "Test " << (i+1) << ":\n";
        cout << "  Message: " << msg << "\n";
        std::stringstream ss; ss << std::hex << c;
        cout << "  Cipher (hex): 0x" << ss.str() << "\n";
        cout << "  Decrypted: " << rec << "\n";
        cout << "  OK: " << boolalpha << (rec == msg) << "\n";
    }
    return 0;
}