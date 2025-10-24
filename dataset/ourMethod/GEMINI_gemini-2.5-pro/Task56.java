import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Optional;

public class Task56 {

    private static final String ENCRYPTION_ALGORITHM = "AES/GCM/NoPadding";
    private static final int AES_KEY_SIZE = 256; // in bits
    private static final int GCM_IV_LENGTH = 12; // in bytes
    private static final int GCM_TAG_LENGTH = 16; // in bytes (128 bits)

    /**
     * Generates a secure authentication token.
     * The token contains the userId and an expiration timestamp, encrypted and authenticated.
     *
     * @param userId          The user identifier to be included in the token.
     * @param key             The secret key for encryption (must be 256 bits/32 bytes).
     * @param validitySeconds The token's validity duration in seconds.
     * @return An Optional containing the Base64 encoded token, or empty if an error occurs.
     */
    public static Optional<String> generateToken(String userId, SecretKey key, int validitySeconds) {
        if (userId == null || userId.isEmpty()) {
            return Optional.empty();
        }

        try {
            long expirationTime = System.currentTimeMillis() / 1000 + validitySeconds;
            String payload = userId + ":" + expirationTime;

            SecureRandom secureRandom = new SecureRandom();
            byte[] iv = new byte[GCM_IV_LENGTH];
            secureRandom.nextBytes(iv);

            Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
            GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
            cipher.init(Cipher.ENCRYPT_MODE, key, gcmParameterSpec);

            byte[] cipherTextWithTag = cipher.doFinal(payload.getBytes(StandardCharsets.UTF_8));

            // Concatenate IV and ciphertext (which includes the auth tag)
            ByteBuffer byteBuffer = ByteBuffer.allocate(iv.length + cipherTextWithTag.length);
            byteBuffer.put(iv);
            byteBuffer.put(cipherTextWithTag);
            byte[] tokenBytes = byteBuffer.array();

            return Optional.of(Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes));
        } catch (Exception e) {
            // In a real application, log this error securely
            return Optional.empty();
        }
    }

    /**
     * Validates a token and extracts the user ID if the token is valid and not expired.
     *
     * @param token The Base64 encoded token string.
     * @param key   The secret key for decryption.
     * @return An Optional containing the user ID if validation is successful, otherwise empty.
     */
    public static Optional<String> validateToken(String token, SecretKey key) {
        if (token == null || token.isEmpty()) {
            return Optional.empty();
        }

        try {
            byte[] tokenBytes = Base64.getUrlDecoder().decode(token);

            if (tokenBytes.length < GCM_IV_LENGTH + GCM_TAG_LENGTH) {
                return Optional.empty(); // Not long enough to be a valid token
            }

            ByteBuffer byteBuffer = ByteBuffer.wrap(tokenBytes);
            byte[] iv = new byte[GCM_IV_LENGTH];
            byteBuffer.get(iv);
            byte[] cipherTextWithTag = new byte[byteBuffer.remaining()];
            byteBuffer.get(cipherTextWithTag);

            Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
            GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
            cipher.init(Cipher.DECRYPT_MODE, key, gcmParameterSpec);

            byte[] decryptedPayloadBytes = cipher.doFinal(cipherTextWithTag);
            String decryptedPayload = new String(decryptedPayloadBytes, StandardCharsets.UTF_8);

            String[] parts = decryptedPayload.split(":", 2);
            if (parts.length != 2) {
                return Optional.empty(); // Malformed payload
            }

            String userId = parts[0];
            long expirationTime = Long.parseLong(parts[1]);
            long currentTime = System.currentTimeMillis() / 1000;

            if (expirationTime < currentTime) {
                System.out.println("Token expired.");
                return Optional.empty();
            }

            return Optional.of(userId);
        } catch (Exception e) {
            // Catches AEADBadTagException for tampered tokens, IllegalArgumentException for bad Base64, etc.
            return Optional.empty();
        }
    }

    public static void main(String[] args) throws Exception {
        // In a real application, this key MUST be loaded from a secure keystore or environment variable.
        // It must NOT be hardcoded.
        KeyGenerator keyGen = KeyGenerator.getInstance("AES");
        keyGen.init(AES_KEY_SIZE);
        SecretKey secretKey = keyGen.generateKey();
        
        SecretKey wrongKey = KeyGenerator.getInstance("AES");
        wrongKey.init(AES_KEY_SIZE);
        wrongKey.generateKey();

        String userId = "user-12345";
        int validitySeconds = 3600;

        System.out.println("--- Test Case 1: Generate and validate a valid token ---");
        Optional<String> tokenOpt = generateToken(userId, secretKey, validitySeconds);
        if (tokenOpt.isPresent()) {
            String token = tokenOpt.get();
            System.out.println("Generated Token: " + token);
            Optional<String> validatedUserId = validateToken(token, secretKey);
            System.out.println("Validation result: " + (validatedUserId.isPresent() && validatedUserId.get().equals(userId) ? "SUCCESS" : "FAILURE"));
            validatedUserId.ifPresent(id -> System.out.println("Validated User ID: " + id));
        } else {
            System.out.println("Token generation failed.");
        }

        System.out.println("\n--- Test Case 2: Validate an expired token ---");
        Optional<String> expiredTokenOpt = generateToken(userId, secretKey, -1);
        if (expiredTokenOpt.isPresent()) {
            String expiredToken = expiredTokenOpt.get();
            Optional<String> validatedUserId = validateToken(expiredToken, secretKey);
            System.out.println("Validation result: " + (!validatedUserId.isPresent() ? "SUCCESS (expired as expected)" : "FAILURE"));
        } else {
            System.out.println("Expired token generation failed.");
        }

        System.out.println("\n--- Test Case 3: Validate a tampered token ---");
        tokenOpt.ifPresent(token -> {
            String tamperedToken = token.substring(0, token.length() - 1) + "Z";
            System.out.println("Tampered Token: " + tamperedToken);
            Optional<String> validatedUserId = validateToken(tamperedToken, secretKey);
            System.out.println("Validation result: " + (!validatedUserId.isPresent() ? "SUCCESS (tampering detected)" : "FAILURE"));
        });
        
        System.out.println("\n--- Test Case 4: Validate with a wrong key ---");
        tokenOpt.ifPresent(token -> {
            Optional<String> validatedUserId = validateToken(token, wrongKey);
            System.out.println("Validation result: " + (!validatedUserId.isPresent() ? "SUCCESS (wrong key detected)" : "FAILURE"));
        });
        
        System.out.println("\n--- Test Case 5: Validate a malformed token ---");
        String malformedToken = "this-is-not-a-valid-token";
        Optional<String> validatedUserId = validateToken(malformedToken, secretKey);
        System.out.println("Validation result: " + (!validatedUserId.isPresent() ? "SUCCESS (malformed token detected)" : "FAILURE"));
    }
}