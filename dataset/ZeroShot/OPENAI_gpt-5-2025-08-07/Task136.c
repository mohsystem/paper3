/* Secure password hashing (e.g., PBKDF2/bcrypt/scrypt/Argon2) is not available in the C standard library.
   To avoid insecure, home-grown cryptography, this implementation is intentionally omitted.
   Please use vetted libraries such as libsodium, Argon2, or OpenSSL's libcrypto for a secure C implementation. */