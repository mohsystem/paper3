// Implementation omitted: Standard C++ library does not provide secure RSA generation or OpenSSH key serialization.
// Use a well-vetted library such as OpenSSL or libsodium (note: libsodium does not support RSA) with careful handling.
// Example approach (not implemented here):
// - Use OpenSSL EVP_PKEY with RSA key generation.
// - Serialize to OpenSSH private key format "openssh-key-v1" with proper structure, or use OpenSSH's sshkey APIs.