// Chain-of-Through process in code generation:
// 1) Problem understanding: Provide a function that computes a salted password hash and returns it.
// 2) Security requirements: Use a strong KDF (PBKDF2 with HMAC-SHA-256) and adequate iterations.
// 3) Secure coding generation: Use standard cryptographic APIs, UTF-8 encoding, immutable handling.
// 4) Code review: Avoid insecure algorithms, constant iteration value, hex-safe output.
// 5) Secure code output: Final code uses PBKDF2WithHmacSHA256 with 200000 iterations and 256-bit output.

import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task82 {
    public static String computeHash(String password, String salt) {
        // PBKDF2-HMAC-SHA256 with fixed strong iteration count and 256-bit key
        final int iterations = 200000;
        final int keyLengthBits = 256; // 32 bytes
        try {
            byte[] saltBytes = salt.getBytes(StandardCharsets.UTF_8);
            KeySpec spec = new PBEKeySpec(password.toCharArray(), saltBytes, iterations, keyLengthBits);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = skf.generateSecret(spec).getEncoded();
            return bytesToHex(hash);
        } catch (Exception e) {
            // In production, handle appropriately; here we return an empty string to satisfy "return output"
            return "";
        }
    }

    private static String bytesToHex(byte[] bytes) {
        char[] HEX = "0123456789abcdef".toCharArray();
        char[] out = new char[bytes.length * 2];
        int i = 0;
        for (byte b : bytes) {
            int v = b & 0xFF;
            out[i++] = HEX[v >>> 4];
            out[i++] = HEX[v & 0x0F];
        }
        return new String(out);
    }

    // 5 test cases
    public static void main(String[] args) {
        System.out.println(computeHash("password", "salt"));
        System.out.println(computeHash("correct horse battery staple", "somesalt"));
        System.out.println(computeHash("P@ssw0rd!", "NaCl"));
        System.out.println(computeHash("", "salt"));
        System.out.println(computeHash("password", ""));
    }
}