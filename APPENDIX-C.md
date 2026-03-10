# Cross-Prompt and Cross-Language Code Snippets for One Task Example

## Java Snippets Using Different Prompt Methods

**CoT** An insecure cipher mode appears at line 6.

```java
  // Generate random IV
  byte[] iv = new byte[IV_LENGTH];
  secureRandom.nextBytes(iv);
  IvParameterSpec ivSpec = new IvParameterSpec(iv);
  // Create cipher and encrypt
  Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding" ); 
cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);
byte[] encrypted = cipher.doFinal(message.getBytes("UTF-8"));
```

**MA-CoT** AEAD mode is used. The insecure cipher mode does not appear in the excerpt shown.

```java
private static final String ALGORITHM = "AES/GCM/NoPadding";
......
byte[] salt = new byte[SALT_LENGTH];
secureRandom.nextBytes(salt);
byte[] iv = new byte[IV_LENGTH];
secureRandom.nextBytes(iv);
SecretKey key = deriveKey(passphrase, salt);
Cipher cipher = Cipher.getInstance(ALGORITHM);
GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_LENGTH, iv);
cipher.init(Cipher.ENCRYPT_MODE, key, gcmSpec);
byte[] plaintext = message.getBytes(StandardCharsets.UTF_8);
byte[] ciphertext = cipher.doFinal(plaintext);
```

**Vanilla and ZeroShot** Both prompt methods generated the same code. An insecure cipher mode appears at line 5.

```java
byte[] key = new byte[16];
byte[] keyBytes = secretKey.getBytes("UTF-8");
System.arraycopy(keyBytes, 0, key, 0, Math.min(keyBytes.length, key.length));
SecretKeySpec secretKeySpec = new SecretKeySpec(key, "AES");
Cipher cipher = Cipher.getInstance("AES/ECB/PKCS5Padding"); 
cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec);
byte[] encryptedBytes = cipher.doFinal(message.getBytes("UTF-8"));
```

## Python Snippets

**CoT** An insecure cipher mode appears at line 15.

```python
KEY_LENGTH = 32  # 256 bits
ITERATION_COUNT = 65536
SALT_LENGTH = 16
IV_LENGTH = 16
Uncovered code
def encrypt_message(message, secret_key):
    try:
        # Generate random salt
        salt = get_random_bytes(SALT_LENGTH)
        # Derive key from password using PBKDF2
        key = PBKDF2(secret_key, salt, dkLen=KEY_LENGTH, count=ITERATION_COUNT)
        # Generate random IV
        iv = get_random_bytes(IV_LENGTH)
        # Create cipher and encrypt
        cipher = AES.new(key, AES.MODE_CBC, iv)        
        # Pad message to be multiple of 16 bytes
        padding_length = AES.block_size - len(message.encode('utf-8')) % AES.block_size
        padded_message = message.encode('utf-8') + bytes([padding_length] * padding_length)
        encrypted = cipher.encrypt(padded_message)
```

**MA-CoT** AEAD mode is used. The insecure cipher mode does not appear in the excerpt shown.

```python
MAGIC = b"ENC1"
VERSION = 1
SALT_LENGTH = 16
IV_LENGTH = 12
TAG_LENGTH = 16
ITERATIONS = 210000
def derive_key(passphrase: str, salt: bytes) -> bytes:
    # Derive a 256-bit AES key from passphrase using PBKDF2-HMAC-SHA256.
    if not passphrase or len(passphrase) < 8:
        raise ValueError("Passphrase must be at least 8 characters")
    kdf = PBKDF2HMAC(
        algorithm=hashes.SHA256(),
        length=32,
        salt=salt,
        iterations=ITERATIONS
    )
    return kdf.derive(passphrase.encode('utf-8'))
def encrypt(message: str, passphrase: str) -> bytes:
    # Encrypt a message using AES-256-GCM with a passphrase.
    if not message or message is None:
        if message != "":
            raise ValueError("Message cannot be None")
    if not passphrase or passphrase is None:
        raise ValueError("Passphrase cannot be None or empty")
    if len(passphrase) < 8:
        raise ValueError("Passphrase must be at least 8 characters")
    salt = secrets.token_bytes(SALT_LENGTH)
    iv = secrets.token_bytes(IV_LENGTH)
    key = derive_key(passphrase, salt)
    aesgcm = AESGCM(key)
    plaintext = message.encode('utf-8')
    ciphertext = aesgcm.encrypt(iv, plaintext, None)
    encrypted_data = MAGIC + bytes([VERSION]) + salt + iv + ciphertext
```

**Vanilla** An insecure cipher mode appears at line 7.

```python
def encrypt_message(message, secret_key):
    try:
        # Ensure the key is 16 bytes (128 bits) for AES
        key = secret_key.encode('utf-8')
        key = key[:16].ljust(16, b'\\0')
        # Create cipher
        cipher = AES.new(key, AES.MODE_ECB) 
        # Encrypt the message
        padded_message = pad(message.encode('utf-8'), AES.block_size)
        encrypted_bytes = cipher.encrypt(padded_message)
```

**ZeroShot** An insecure cipher mode appears at line 6.

```python
def encrypt(message, secret_key):
    try:
        # Ensure key is 16 bytes (128-bit)
        key_bytes = secret_key.encode('utf-8')[:16].ljust(16, b'\\0')
        # Create cipher for encryption
        cipher = AES.new(key_bytes, AES.MODE_ECB)
        # Pad and encrypt the message
        padded_message = pad(message.encode('utf-8'), AES.block_size)
        encrypted_bytes = cipher.encrypt(padded_message)
```

## C Snippets

**CoT**  A non-AEAD cipher mode appears at lines 31 and 38.

```c
char* encrypt_message(const char* message, const char* secret_key) {
    unsigned char salt[SALT_LENGTH];
    unsigned char key[KEY_LENGTH];
    unsigned char iv[IV_LENGTH];
    // Generate random salt
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        fprintf(stderr, "Failed to generate salt\\n");
        return NULL;
    }
    // Derive key from password using PBKDF2
    if (PKCS5_PBKDF2_HMAC(secret_key, strlen(secret_key),
                          salt, SALT_LENGTH,
                          ITERATION_COUNT,
                          EVP_sha256(),
                          KEY_LENGTH, key) != 1) {
        fprintf(stderr, "Key derivation failed\\n");
        return NULL;
    }
    // Generate random IV
    if (RAND_bytes(iv, IV_LENGTH) != 1) {
        fprintf(stderr, "Failed to generate IV\\n");
        return NULL;
    }
    // Create cipher context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Failed to create cipher context\\n");
        return NULL;
    }
    // Initialize encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) { 
        EVP_CIPHER_CTX_free(ctx);
        fprintf(stderr, "Encryption initialization failed\\n");
        return NULL;
    }
    // Encrypt message
    int message_len = strlen(message);
    unsigned char* ciphertext = (unsigned char*)malloc(message_len + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
```

**MA-CoT** AEAD mode is used. The insecure cipher mode does not appear in the excerpt shown.

```c
/* Derive key from passphrase using PBKDF2-HMAC-SHA-256 */
int derive_key(const char* passphrase, const uint8_t* salt, uint8_t* key) {
    if (passphrase == NULL || salt == NULL || key == NULL) {
        return 0;
    }
    size_t passphrase_len = strlen(passphrase);
    if (passphrase_len == 0 || passphrase_len > 1024) {
        return 0;
    }
    /* Use PBKDF2 with HMAC-SHA-256 */
    int result = PKCS5_PBKDF2_HMAC(
        passphrase, (int)passphrase_len,
        salt, SALT_SIZE,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        KEY_SIZE, key
    );
    return result;
}
/* Encrypt message using AES-256-GCM */
EncryptedData* encrypt_message(const char* message, const char* passphrase) {
    if (message == NULL || passphrase == NULL) {
        return NULL;
    }
    size_t message_len = strlen(message);
    if (message_len == 0 || message_len > 1000000) {  /* Reasonable limit */
        return NULL;
    }
    EVP_CIPHER_CTX* ctx = NULL;
    EncryptedData* result = NULL;
    uint8_t salt[SALT_SIZE];
    uint8_t iv[IV_SIZE];
    uint8_t key[KEY_SIZE];
    uint8_t tag[TAG_SIZE];
    uint8_t* ciphertext = NULL;
    int len = 0;
    int ciphertext_len = 0;
    /* Initialize all arrays */
    memset(salt, 0, SALT_SIZE);
    memset(iv, 0, IV_SIZE);
    memset(key, 0, KEY_SIZE);
    memset(tag, 0, TAG_SIZE);
    /* Generate cryptographically secure random salt and IV */
    if (RAND_bytes(salt, SALT_SIZE) != 1) {
        goto cleanup;
    }
    if (RAND_bytes(iv, IV_SIZE) != 1) {
        goto cleanup;
    }
    /* Derive key from passphrase */
    if (derive_key(passphrase, salt, key) != 1) {
        goto cleanup;
    }
    /* Create and initialize cipher context */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        goto cleanup;
    }
    /* Initialize encryption with AES-256-GCM */
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        goto cleanup;
    }
    /* Set IV length */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL) != 1) {
        goto cleanup;
    }
    /* Initialize key and IV */
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
        goto cleanup;
    }
    /* Allocate buffer for ciphertext */
    ciphertext = (uint8_t*)malloc(message_len);
    if (ciphertext == NULL) {
        goto cleanup;
    }
    /* Encrypt the message */
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, (const uint8_t*)message, (int)message_len) != 1) {
        goto cleanup;
    }
    ciphertext_len = len;
    /* Finalize encryption */
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        goto cleanup;
    }
    ciphertext_len += len;
    
    /* Get authentication tag */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        goto cleanup;
    }
    /* Build output format: [MAGIC][VERSION][SALT][IV][CIPHERTEXT][TAG] */
    size_t total_size = 4 + 1 + SALT_SIZE + IV_SIZE + ciphertext_len + TAG_SIZE;
    result = (EncryptedData*)malloc(sizeof(EncryptedData));
    if (result == NULL) {
        goto cleanup;
    }
    result->data = (uint8_t*)malloc(total_size);
    if (result->data == NULL) {
        free(result);
        result = NULL;
        goto cleanup;
    }
    result->length = total_size;
    /* Assemble the encrypted data structure */
    size_t offset = 0;
    memcpy(result->data + offset, MAGIC, 4);
    offset += 4;
    result->data[offset] = VERSION;
    offset += 1;
    memcpy(result->data + offset, salt, SALT_SIZE);
    offset += SALT_SIZE;
    memcpy(result->data + offset, iv, IV_SIZE);
    offset += IV_SIZE;
    memcpy(result->data + offset, ciphertext, ciphertext_len);
    offset += ciphertext_len;
    memcpy(result->data + offset, tag, TAG_SIZE);
cleanup:
    /* Securely clear sensitive data */
    secure_clear(key, KEY_SIZE);
    secure_clear(salt, SALT_SIZE);
    secure_clear(iv, IV_SIZE);
    secure_clear(tag, TAG_SIZE);
    if (ciphertext != NULL) {
        secure_clear(ciphertext, message_len);
        free(ciphertext);
    }
    if (ctx != NULL) {
        EVP_CIPHER_CTX_free(ctx);
    }
    return result;
}
/* Free encrypted data */
void free_encrypted_data(EncryptedData* data) {
    if (data != NULL) {
        if (data->data != NULL) {
            secure_clear(data->data, data->length);
            free(data->data);
        }
        free(data);
    }
}
```

**Vanilla** An insecure cipher mode appears at line 6.

```c
char* encryptMessage(const char* message, const char* secretKey) {
    unsigned char key[16] = {0};
    size_t keyLen = strlen(secretKey);
    memcpy(key, secretKey, keyLen < 16 ? keyLen : 16);
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb (), NULL, key, NULL);
    int messageLen = strlen(message);
    int ciphertext_len = messageLen + AES_BLOCK_SIZE;
    unsigned char* ciphertext = (unsigned char*)malloc(ciphertext_len);
    int len;
    EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)message, messageLen);
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;
    char* result = base64_encode(ciphertext, ciphertext_len);
    free(ciphertext);
    
    EVP_CIPHER_CTX_free(ctx);
```

**ZeroShot** An insecure cipher mode appears at line 8.

```c
char* encrypt(const char* message, const char* secretKey) {
    // Prepare 16-byte key
    unsigned char key[16] = {0};
    size_t keyLen = strlen(secretKey);
    memcpy(key, secretKey, keyLen < 16 ? keyLen : 16);
    // Create cipher context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb (), NULL, key, NULL); 
    // Allocate buffer for encrypted data
    int len;
    int ciphertext_len;
    unsigned char ciphertext[1024];
    // Encrypt the message
    EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)message, strlen(message));
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    // Return Base64 encoded string
    return base64_encode(ciphertext, ciphertext_len);
}
```
