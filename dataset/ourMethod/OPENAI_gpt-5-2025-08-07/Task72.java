import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task72 {

    private static final String MAGIC = "ENC1";
    private static final byte VERSION = 1;
    private static final int SALT_LEN = 16;
    private static final int IV_LEN = 12;
    private static final int TAG_LEN_BITS = 128; // 16 bytes
    private static final int ITERATIONS = 210_000;
    private static final int KEY_LEN_BITS = 256;

    public static byte[] encrypt(String passphrase, byte[] plaintext) throws GeneralSecurityException {
        if (plaintext == null) throw new GeneralSecurityException("Invalid plaintext");
        if (passphrase == null) throw new GeneralSecurityException("Invalid passphrase");

        SecureRandom sr = new SecureRandom();
        byte[] salt = new byte[SALT_LEN];
        byte[] iv = new byte[IV_LEN];
        sr.nextBytes(salt);
        sr.nextBytes(iv);

        char[] pw = passphrase.toCharArray();
        byte[] key = null;
        try {
            key = deriveKey(pw, salt);
            SecretKeySpec keySpec = new SecretKeySpec(key, "AES");

            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_LEN_BITS, iv);
            cipher.init(Cipher.ENCRYPT_MODE, keySpec, gcmSpec);

            byte[] ciphertextAndTag = cipher.doFinal(plaintext);

            int totalLen = 4 + 1 + SALT_LEN + IV_LEN + ciphertextAndTag.length;
            byte[] out = new byte[totalLen];
            int pos = 0;
            byte[] magicBytes = MAGIC.getBytes(StandardCharsets.US_ASCII);
            System.arraycopy(magicBytes, 0, out, pos, magicBytes.length);
            pos += magicBytes.length;
            out[pos++] = VERSION;
            System.arraycopy(salt, 0, out, pos, SALT_LEN);
            pos += SALT_LEN;
            System.arraycopy(iv, 0, out, pos, IV_LEN);
            pos += IV_LEN;
            System.arraycopy(ciphertextAndTag, 0, out, pos, ciphertextAndTag.length);
            return out;
        } finally {
            Arrays.fill(pw, '\0');
            if (key != null) Arrays.fill(key, (byte) 0);
        }
    }

    public static byte[] decrypt(String passphrase, byte[] blob) throws GeneralSecurityException {
        if (blob == null || blob.length < 4 + 1 + SALT_LEN + IV_LEN + (TAG_LEN_BITS / 8)) {
            throw new GeneralSecurityException("Invalid input");
        }
        if (passphrase == null) throw new GeneralSecurityException("Invalid passphrase");

        int pos = 0;
        byte[] magicBytes = MAGIC.getBytes(StandardCharsets.US_ASCII);
        for (int i = 0; i < magicBytes.length; i++) {
            if (blob[pos + i] != magicBytes[i]) throw new GeneralSecurityException("Bad header");
        }
        pos += magicBytes.length;
        byte ver = blob[pos++];
        if (ver != VERSION) throw new GeneralSecurityException("Unsupported version");

        byte[] salt = Arrays.copyOfRange(blob, pos, pos + SALT_LEN);
        pos += SALT_LEN;
        byte[] iv = Arrays.copyOfRange(blob, pos, pos + IV_LEN);
        pos += IV_LEN;
        byte[] ciphertextAndTag = Arrays.copyOfRange(blob, pos, blob.length);
        if (ciphertextAndTag.length < (TAG_LEN_BITS / 8)) {
            throw new GeneralSecurityException("Ciphertext too short");
        }

        char[] pw = passphrase.toCharArray();
        byte[] key = null;
        try {
            key = deriveKey(pw, salt);
            SecretKeySpec keySpec = new SecretKeySpec(key, "AES");

            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_LEN_BITS, iv);
            cipher.init(Cipher.DECRYPT_MODE, keySpec, gcmSpec);

            return cipher.doFinal(ciphertextAndTag);
        } finally {
            Arrays.fill(pw, '\0');
            if (key != null) Arrays.fill(key, (byte) 0);
        }
    }

    private static byte[] deriveKey(char[] password, byte[] salt) throws GeneralSecurityException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LEN_BITS);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null || a.length != b.length) return false;
        int r = 0;
        for (int i = 0; i < a.length; i++) r |= (a[i] ^ b[i]);
        return r == 0;
    }

    public static void main(String[] args) {
        try {
            String pass = "correct horse battery staple";
            String[] messages = new String[]{
                    "Hello, World!",
                    "",
                    "The quick brown fox jumps over the lazy dog",
                    "0".repeat(1000),
                    "Unicode π 🚀"
            };
            for (int i = 0; i < messages.length; i++) {
                byte[] plaintext = messages[i].getBytes(StandardCharsets.UTF_8);
                byte[] enc = encrypt(pass, plaintext);
                byte[] dec = decrypt(pass, enc);
                boolean ok = constantTimeEquals(plaintext, dec);
                System.out.println("Test " + (i + 1) + ": " + (ok ? "OK" : "FAIL") + " encSize=" + enc.length);
            }
        } catch (Exception e) {
            System.out.println("Error");
        }
    }
}