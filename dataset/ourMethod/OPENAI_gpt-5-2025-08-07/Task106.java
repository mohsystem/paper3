import javax.crypto.Cipher;
import javax.crypto.spec.OAEPParameterSpec;
import javax.crypto.spec.MGF1ParameterSpec;
import javax.crypto.spec.PSource;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.util.Base64;

public class Task106 {

    public static KeyPair generateRsaKeyPair(final int bits) {
        if (bits < 2048) {
            throw new IllegalArgumentException("RSA key size must be at least 2048 bits");
        }
        try {
            KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA");
            SecureRandom sr = SecureRandom.getInstanceStrong();
            kpg.initialize(bits, sr);
            return kpg.generateKeyPair();
        } catch (GeneralSecurityException e) {
            throw new RuntimeException("Key generation failed", e);
        }
    }

    public static byte[] rsaEncrypt(final byte[] plaintext, final PublicKey publicKey) {
        if (plaintext == null || publicKey == null) {
            throw new IllegalArgumentException("Invalid input");
        }
        try {
            Cipher cipher = Cipher.getInstance("RSA/ECB/OAEPWithSHA-256AndMGF1Padding");
            OAEPParameterSpec spec = new OAEPParameterSpec(
                    "SHA-256",
                    "MGF1",
                    MGF1ParameterSpec.SHA256,
                    PSource.PSpecified.DEFAULT
            );
            cipher.init(Cipher.ENCRYPT_MODE, publicKey, spec);
            return cipher.doFinal(plaintext);
        } catch (GeneralSecurityException e) {
            throw new RuntimeException("Encryption failed", e);
        }
    }

    public static byte[] rsaDecrypt(final byte[] ciphertext, final PrivateKey privateKey) {
        if (ciphertext == null || privateKey == null) {
            throw new IllegalArgumentException("Invalid input");
        }
        try {
            Cipher cipher = Cipher.getInstance("RSA/ECB/OAEPWithSHA-256AndMGF1Padding");
            OAEPParameterSpec spec = new OAEPParameterSpec(
                    "SHA-256",
                    "MGF1",
                    MGF1ParameterSpec.SHA256,
                    PSource.PSpecified.DEFAULT
            );
            cipher.init(Cipher.DECRYPT_MODE, privateKey, spec);
            return cipher.doFinal(ciphertext);
        } catch (GeneralSecurityException e) {
            throw new RuntimeException("Decryption failed", e);
        }
    }

    public static String toBase64(byte[] data) {
        return Base64.getEncoder().encodeToString(data);
    }

    public static void main(String[] args) {
        String[] messages = new String[] {
                "Hello, RSA OAEP!",
                "The quick brown fox jumps over the lazy dog.",
                "RSA with OAEP SHA-256 and MGF1.",
                "Data 1234567890!@#$%^&*()",
                "Unicode: こんにちは世界🌐"
        };

        KeyPair kp = generateRsaKeyPair(2048);
        int passed = 0;
        for (int i = 0; i < messages.length; i++) {
            byte[] pt = messages[i].getBytes(StandardCharsets.UTF_8);
            byte[] ct = rsaEncrypt(pt, kp.getPublic());
            byte[] dec = rsaDecrypt(ct, kp.getPrivate());
            boolean ok = MessageDigest.isEqual(pt, dec);
            if (ok) {
                passed++;
            }
            // Print minimal test result without exposing plaintext or keys.
            System.out.println("Test " + (i + 1) + ": " + (ok ? "OK" : "FAIL") + " | Ciphertext(Base64) length=" + toBase64(ct).length());
        }
        System.out.println("Passed " + passed + " of " + messages.length + " tests.");
    }
}