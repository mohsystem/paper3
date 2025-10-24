import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Base64;

public class Task79 {
    // Encrypts the message using a password-derived AES-256-GCM key and returns "salt:iv:ciphertextTag" (all Base64)
    public static String encrypt(String message, String password) {
        if (message == null) message = "";
        if (password == null) password = "";
        final int saltLen = 16;
        final int ivLen = 12;
        final int keyLenBits = 256;
        final int tagLenBits = 128;
        final int iterations = 200_000;

        byte[] salt = new byte[saltLen];
        byte[] iv = new byte[ivLen];
        byte[] keyBytes = null;
        char[] pwdChars = password.toCharArray();

        try {
            SecureRandom sr = SecureRandom.getInstanceStrong();
            sr.nextBytes(salt);
            sr.nextBytes(iv);

            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            KeySpec spec = new PBEKeySpec(pwdChars, salt, iterations, keyLenBits);
            keyBytes = skf.generateSecret(spec).getEncoded();
            SecretKeySpec key = new SecretKeySpec(keyBytes, "AES");

            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec gcmSpec = new GCMParameterSpec(tagLenBits, iv);
            cipher.init(Cipher.ENCRYPT_MODE, key, gcmSpec);

            byte[] ciphertextWithTag = cipher.doFinal(message.getBytes(StandardCharsets.UTF_8));

            String sSalt = Base64.getEncoder().encodeToString(salt);
            String sIv = Base64.getEncoder().encodeToString(iv);
            String sCt = Base64.getEncoder().encodeToString(ciphertextWithTag);
            return sSalt + ":" + sIv + ":" + sCt;
        } catch (Exception e) {
            return null;
        } finally {
            if (keyBytes != null) {
                java.util.Arrays.fill(keyBytes, (byte) 0);
            }
            java.util.Arrays.fill(pwdChars, '\0');
        }
    }

    public static void main(String[] args) {
        System.out.println(encrypt("Hello, World!", "S3cr3t!Key"));
        System.out.println(encrypt("Attack at dawn", "another$trongKey123"));
        System.out.println(encrypt("1234567890", "password"));
        System.out.println(encrypt("Unicode ✓ © ☕", "𝒦𝑒𝓎🔒"));
        System.out.println(encrypt("", "empty-message-key"));
    }
}