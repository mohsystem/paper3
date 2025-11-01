/* Secure password verification with a proper KDF (PBKDF2/bcrypt/scrypt/Argon2)
 * requires cryptographic primitives not provided by standard C.
 * Implementing these primitives from scratch is error-prone and insecure.
 * Please use a vetted crypto library (e.g., OpenSSL, libsodium, argon2)
 * to:
 *  - add_user(username, password): generate random salt; derive key; store salt and hash.
 *  - login(username, password): derive key with stored parameters; constant-time compare.
 * Include 5 tests in main when implementing with appropriate libraries.
 */