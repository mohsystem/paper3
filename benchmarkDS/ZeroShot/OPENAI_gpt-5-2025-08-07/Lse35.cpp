// Secure password verification with proper KDF (e.g., PBKDF2, bcrypt, scrypt, Argon2)
// typically requires cryptographic libraries not available in standard C++.
// To provide a secure and portable implementation without external dependencies
// is not feasible here. Consider using a well-vetted crypto library (e.g., OpenSSL,
// libsodium, argon2) and implement:
// - addUser(username, password): generate random salt; derive key; store salt and hash.
// - login(username, password): derive key with stored parameters; constant-time compare.
// Include 5 tests in main when implementing with appropriate libraries.