import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.Base64;

public class Task79 {

    // Encrypts plaintext using passphrase-derived AES-256-GCM key and returns the blob:
    // [magic="ENC1"][version=1][salt(16)][iv(12)][ciphertext||tag(16)]
    public static byte[] encrypt(String passphrase, byte[] plaintext) throws GeneralSecurityException {
        if (passphrase == null || passphrase.isEmpty()) {
            throw new IllegalArgumentException("Invalid passphrase");
        }
        if (plaintext == null) {
            throw new IllegalArgumentException("Invalid plaintext");
        }
        if (plaintext.length > 50_000_000) {
            throw new IllegalArgumentException("Plaintext too large");
        }

        final byte[] salt = new byte[16];
        final byte[] iv = new byte[12];
        SecureRandom sr = new SecureRandom();
        sr.nextBytes(salt);
        sr.nextBytes(iv);

        final int iterations = 210_000;
        PBEKeySpec spec = new PBEKeySpec(passphrase.toCharArray(), salt, iterations, 256);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] keyBytes = skf.generateSecret(spec).getEncoded();
        SecretKeySpec key = new SecretKeySpec(keyBytes, "AES");

        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec gcmSpec = new GCMParameterSpec(128, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, gcmSpec);
        byte[] ctAndTag = cipher.doFinal(plaintext);

        byte[] magic = "ENC1".getBytes(StandardCharsets.US_ASCII);
        byte version = 1;
        int totalLen = magic.length + 1 + salt.length + iv.length + ctAndTag.length;
        byte[] out = new byte[totalLen];

        int pos = 0;
        System.arraycopy(magic, 0, out, pos, magic.length); pos += magic.length;
        out[pos++] = version;
        System.arraycopy(salt, 0, out, pos, salt.length); pos += salt.length;
        System.arraycopy(iv, 0, out, pos, iv.length); pos += iv.length;
        System.arraycopy(ctAndTag, 0, out, pos, ctAndTag.length);

        // Zero sensitive arrays when feasible
        java.util.Arrays.fill(keyBytes, (byte) 0);
        spec.clearPassword();

        return out;
    }

    private static void printCiphertextB64(byte[] blob) {
        String b64 = Base64.getEncoder().encodeToString(blob);
        System.out.println(b64);
    }

    public static void main(String[] args) {
        try {
            byte[] c1 = encrypt("correct horse battery staple", "Hello, world!".getBytes(StandardCharsets.UTF_8));
            byte[] c2 = encrypt("Tr0ub4dor&3", "Security-focused encryption using AES-256-GCM.".getBytes(StandardCharsets.UTF_8));
            byte[] c3 = encrypt("another strong passphrase", "短いメッセージ".getBytes(StandardCharsets.UTF_8));
            byte[] c4 = encrypt("Yet-Another-Secret", "1234567890".getBytes(StandardCharsets.UTF_8));
            byte[] c5 = encrypt("SufficientlyLongAndComplexPassphrase!", "The quick brown fox jumps over the lazy dog.".getBytes(StandardCharsets.UTF_8));

            printCiphertextB64(c1);
            printCiphertextB64(c2);
            printCiphertextB64(c3);
            printCiphertextB64(c4);
            printCiphertextB64(c5);
        } catch (GeneralSecurityException | IllegalArgumentException e) {
            System.out.println("Encryption failed");
        }
    }
}