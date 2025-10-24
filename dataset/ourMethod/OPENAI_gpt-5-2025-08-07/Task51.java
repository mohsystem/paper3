import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task51 {

    private static final byte[] MAGIC = new byte[]{'E', 'N', 'C', '1'};
    private static final byte VERSION = 0x01;
    private static final int SALT_LEN = 16;
    private static final int IV_LEN = 12;
    private static final int TAG_LEN = 16;
    private static final int ITERATIONS = 210_000;
    private static final int KEY_LEN_BITS = 256;

    public static byte[] encrypt(String plaintext, String passphrase) throws GeneralSecurityException {
        if (plaintext == null || passphrase == null) {
            throw new GeneralSecurityException("Invalid input");
        }
        byte[] salt = new byte[SALT_LEN];
        byte[] iv = new byte[IV_LEN];
        SecureRandom sr = new SecureRandom();
        sr.nextBytes(salt);
        sr.nextBytes(iv);

        SecretKeySpec key = deriveKey(passphrase, salt);
        byte[] pt = plaintext.getBytes(StandardCharsets.UTF_8);

        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec gcm = new GCMParameterSpec(TAG_LEN * 8, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, gcm);
        byte[] ctAndTag = cipher.doFinal(pt);

        if (ctAndTag.length < TAG_LEN) {
            throw new GeneralSecurityException("Encryption failure");
        }
        int ctLen = ctAndTag.length - TAG_LEN;
        byte[] ct = Arrays.copyOf(ctAndTag, ctLen);
        byte[] tag = Arrays.copyOfRange(ctAndTag, ctLen, ctAndTag.length);

        int totalLen = MAGIC.length + 1 + SALT_LEN + IV_LEN + ct.length + TAG_LEN;
        byte[] out = new byte[totalLen];
        int pos = 0;
        System.arraycopy(MAGIC, 0, out, pos, MAGIC.length); pos += MAGIC.length;
        out[pos++] = VERSION;
        System.arraycopy(salt, 0, out, pos, SALT_LEN); pos += SALT_LEN;
        System.arraycopy(iv, 0, out, pos, IV_LEN); pos += IV_LEN;
        System.arraycopy(ct, 0, out, pos, ct.length); pos += ct.length;
        System.arraycopy(tag, 0, out, pos, TAG_LEN);

        // Zero sensitive data
        Arrays.fill(pt, (byte) 0);
        return out;
    }

    public static String decrypt(byte[] blob, String passphrase) throws GeneralSecurityException {
        if (blob == null || passphrase == null) {
            throw new GeneralSecurityException("Invalid input");
        }
        int minLen = MAGIC.length + 1 + SALT_LEN + IV_LEN + TAG_LEN;
        if (blob.length < minLen) {
            throw new GeneralSecurityException("Decryption failed");
        }
        int pos = 0;
        if (!MessageDigest.isEqual(Arrays.copyOfRange(blob, pos, pos + MAGIC.length), MAGIC)) {
            throw new GeneralSecurityException("Decryption failed");
        }
        pos += MAGIC.length;
        byte version = blob[pos++];
        if (version != VERSION) {
            throw new GeneralSecurityException("Decryption failed");
        }
        byte[] salt = Arrays.copyOfRange(blob, pos, pos + SALT_LEN); pos += SALT_LEN;
        byte[] iv = Arrays.copyOfRange(blob, pos, pos + IV_LEN); pos += IV_LEN;
        int remaining = blob.length - pos;
        if (remaining < TAG_LEN) {
            throw new GeneralSecurityException("Decryption failed");
        }
        int ctLen = remaining - TAG_LEN;
        byte[] ct = Arrays.copyOfRange(blob, pos, pos + ctLen); pos += ctLen;
        byte[] tag = Arrays.copyOfRange(blob, pos, pos + TAG_LEN);

        SecretKeySpec key = deriveKey(passphrase, salt);

        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec gcm = new GCMParameterSpec(TAG_LEN * 8, iv);
        cipher.init(Cipher.DECRYPT_MODE, key, gcm);

        byte[] ctAndTag = new byte[ct.length + tag.length];
        System.arraycopy(ct, 0, ctAndTag, 0, ct.length);
        System.arraycopy(tag, 0, ctAndTag, ct.length, tag.length);

        byte[] pt = cipher.doFinal(ctAndTag);
        return new String(pt, StandardCharsets.UTF_8);
    }

    private static SecretKeySpec deriveKey(String passphrase, byte[] salt) throws GeneralSecurityException {
        PBEKeySpec spec = new PBEKeySpec(passphrase.toCharArray(), salt, ITERATIONS, KEY_LEN_BITS);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] keyBytes = skf.generateSecret(spec).getEncoded();
        return new SecretKeySpec(keyBytes, "AES");
    }

    public static void main(String[] args) {
        String[] tests = new String[]{
                "",
                "hello",
                "The quick brown fox jumps over the lazy dog.",
                "Numbers 1234567890!@#",
                "Repeat message repeat message repeat message."
        };
        String pass = "Strong Passphrase 2025!";
        int ok = 0;
        for (int i = 0; i < tests.length; i++) {
            try {
                byte[] enc = encrypt(tests[i], pass);
                String dec = decrypt(enc, pass);
                boolean equal = MessageDigest.isEqual(tests[i].getBytes(StandardCharsets.UTF_8), dec.getBytes(StandardCharsets.UTF_8));
                if (equal) {
                    System.out.println("Test " + (i + 1) + " OK");
                    ok++;
                } else {
                    System.out.println("Test " + (i + 1) + " FAIL");
                }
            } catch (GeneralSecurityException e) {
                System.out.println("Test " + (i + 1) + " FAIL");
            }
        }
        if (ok != tests.length) {
            System.exit(1);
        }
    }
}