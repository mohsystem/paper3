import javax.crypto.Cipher;
import javax.crypto.Mac;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task83 {
    private static final byte[] MAGIC = "ECBC".getBytes(StandardCharsets.US_ASCII);
    private static final byte VERSION = 1;
    private static final int SALT_LEN = 16;
    private static final int IV_LEN = 16;
    private static final int ENC_KEY_LEN = 32; // AES-256
    private static final int MAC_KEY_LEN = 32; // HMAC-SHA256
    private static final int HMAC_LEN = 32;
    private static final int MAX_PLAINTEXT = 1 << 20; // 1 MiB

    public static byte[] encryptKeyAesCbcEtM(byte[] keyToEncrypt, byte[] masterKey) throws GeneralSecurityException {
        if (keyToEncrypt == null || masterKey == null) {
            throw new IllegalArgumentException("Inputs must not be null.");
        }
        if (keyToEncrypt.length == 0 || keyToEncrypt.length > MAX_PLAINTEXT) {
            throw new IllegalArgumentException("Plaintext length out of allowed range.");
        }
        if (masterKey.length < 16) {
            throw new IllegalArgumentException("Master key must be at least 16 bytes.");
        }

        SecureRandom rng = new SecureRandom();
        byte[] salt = new byte[SALT_LEN];
        byte[] iv = new byte[IV_LEN];
        rng.nextBytes(salt);
        rng.nextBytes(iv);

        byte[] okm = hkdfSha256(masterKey, salt, "AES-CBC-ETM".getBytes(StandardCharsets.US_ASCII), ENC_KEY_LEN + MAC_KEY_LEN);
        byte[] encKey = Arrays.copyOfRange(okm, 0, ENC_KEY_LEN);
        byte[] macKey = Arrays.copyOfRange(okm, ENC_KEY_LEN, ENC_KEY_LEN + MAC_KEY_LEN);
        Arrays.fill(okm, (byte) 0);

        byte[] ciphertext;
        try {
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            SecretKeySpec encKeySpec = new SecretKeySpec(encKey, "AES");
            cipher.init(Cipher.ENCRYPT_MODE, encKeySpec, new IvParameterSpec(iv));
            ciphertext = cipher.doFinal(keyToEncrypt);
        } finally {
            Arrays.fill(encKey, (byte) 0);
        }

        Mac hmac = Mac.getInstance("HmacSHA256");
        SecretKeySpec macKeySpec = new SecretKeySpec(macKey, "HmacSHA256");
        hmac.init(macKeySpec);
        hmac.update(MAGIC);
        hmac.update(new byte[]{VERSION});
        hmac.update(salt);
        hmac.update(iv);
        hmac.update(ciphertext);
        byte[] tag = hmac.doFinal();
        Arrays.fill(macKey, (byte) 0);

        ByteBuffer out = ByteBuffer.allocate(MAGIC.length + 1 + SALT_LEN + IV_LEN + ciphertext.length + HMAC_LEN);
        out.put(MAGIC);
        out.put(VERSION);
        out.put(salt);
        out.put(iv);
        out.put(ciphertext);
        out.put(tag);
        return out.array();
    }

    private static byte[] hkdfSha256(byte[] ikm, byte[] salt, byte[] info, int length) throws GeneralSecurityException {
        Mac mac = Mac.getInstance("HmacSHA256");
        byte[] realSalt = salt != null && salt.length > 0 ? salt : new byte[mac.getMacLength()];
        SecretKeySpec saltKey = new SecretKeySpec(realSalt, "HmacSHA256");
        mac.init(saltKey);
        byte[] prk = mac.doFinal(ikm);

        byte[] okm = new byte[length];
        byte[] t = new byte[0];
        int pos = 0;
        int counter = 1;
        while (pos < length) {
            mac.reset();
            mac.init(new SecretKeySpec(prk, "HmacSHA256"));
            mac.update(t);
            if (info != null) mac.update(info);
            mac.update((byte) counter);
            t = mac.doFinal();
            int toCopy = Math.min(t.length, length - pos);
            System.arraycopy(t, 0, okm, pos, toCopy);
            pos += toCopy;
            counter++;
        }
        Arrays.fill(prk, (byte) 0);
        Arrays.fill(t, (byte) 0);
        return okm;
    }

    private static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) sb.append(String.format("%02x", b));
        return sb.toString();
    }

    public static void main(String[] args) {
        try {
            SecureRandom rng = new SecureRandom();
            for (int i = 1; i <= 5; i++) {
                byte[] masterKey = new byte[32];
                rng.nextBytes(masterKey);
                int ptLen = 8 * i; // varying lengths
                byte[] keyToEncrypt = new byte[ptLen];
                rng.nextBytes(keyToEncrypt);

                byte[] enc = encryptKeyAesCbcEtM(keyToEncrypt, masterKey);
                MessageDigest sha256 = MessageDigest.getInstance("SHA-256");
                byte[] digest = sha256.digest(enc);
                System.out.println("Test " + i + ": outLen=" + enc.length + ", sha256=" + toHex(digest));

                Arrays.fill(masterKey, (byte) 0);
                Arrays.fill(keyToEncrypt, (byte) 0);
            }
        } catch (Exception e) {
            System.out.println("Error during tests: " + e.getMessage());
        }
    }
}