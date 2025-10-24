#include <iostream>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

using boost::multiprecision::cpp_int;

cpp_int modexp(cpp_int base, cpp_int exp, const cpp_int& mod) {
    base %= mod;
    cpp_int result = 1;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

cpp_int encrypt_rsa(const cpp_int& message, const cpp_int& e, const cpp_int& n) {
    return modexp(message, e, n);
}

cpp_int decrypt_rsa(const cpp_int& ciphertext, const cpp_int& d, const cpp_int& n) {
    return modexp(ciphertext, d, n);
}

int main() {
    cpp_int n = 3233;
    cpp_int e = 17;
    cpp_int d = 2753;

    std::vector<int> messages = {65, 42, 123, 999, 2021};

    for (int m : messages) {
        cpp_int msg = m;
        cpp_int c = encrypt_rsa(msg, e, n);
        cpp_int p = decrypt_rsa(c, d, n);
        std::cout << "Message: " << msg << " -> Cipher: " << c << " -> Decrypted: " << p << "\n";
    }
    return 0;
}