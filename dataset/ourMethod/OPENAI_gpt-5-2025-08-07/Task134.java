import javax.crypto.AEADBadTagException;
import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.OAEPParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.PSource;
import javax.crypto.spec.MGF1ParameterSpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.util.Arrays;

public class Task134 {

    private static final byte[] MAGIC = new byte[]{'E', 'N', 'C', '1'};
    private static final byte VERSION = 1;
    private static final int SALT_LEN = 16;
    private static final int IV_LEN = 12;
    private static final int TAG_LEN = 16;
    private static final int PBKDF2_ITERS = 210_000;
    private static final int AES_KEY_LEN_BITS = 256;

    private static byte[] secureRandomBytes(int len) {
        SecureRandom rng = new SecureRandom();
        byte[] out = new byte[len];
        rng.nextBytes(out);
        return out;
    }

    private static byte[] deriveKey(char[] passphrase, byte[] salt, int iterations) {
        byte[] key = null;
        try {
            PBEKeySpec spec = new PBEKeySpec(passphrase, salt, iterations, AES_KEY_LEN_BITS);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            key = skf.generateSecret(spec).getEncoded();
            return key;
        } catch (GeneralSecurityException e) {
            return null;
        }
    }

    public static byte[] encrypt(byte[] plaintext, char[] passphrase) {
        if (plaintext == null || passphrase == null) return null;
        byte[] salt = secureRandomBytes(SALT_LEN);
        byte[] iv = secureRandomBytes(IV_LEN);
        byte[] key = deriveKey(passphrase, salt, PBKDF2_ITERS);
        if (key == null) return null;

        try {
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_LEN * 8, iv);
            cipher.init(Cipher.ENCRYPT_MODE, new javax.crypto.spec.SecretKeySpec(key, "AES"), gcmSpec);
            byte[] ctWithTag = cipher.doFinal(plaintext);
            if (ctWithTag.length < TAG_LEN) return null;
            int ctLen = ctWithTag.length - TAG_LEN;
            byte[] ct = Arrays.copyOfRange(ctWithTag, 0, ctLen);
            byte[] tag = Arrays.copyOfRange(ctWithTag, ctLen, ctWithTag.length);

            ByteBuffer buf = ByteBuffer.allocate(MAGIC.length + 1 + SALT_LEN + IV_LEN + ct.length + TAG_LEN);
            buf.put(MAGIC);
            buf.put(VERSION);
            buf.put(salt);
            buf.put(iv);
            buf.put(ct);
            buf.put(tag);
            return buf.array();
        } catch (GeneralSecurityException e) {
            return null;
        } finally {
            Arrays.fill(key, (byte) 0);
        }
    }

    public static byte[] decrypt(byte[] data, char[] passphrase) {
        if (data == null || data.length < MAGIC.length + 1 + SALT_LEN + IV_LEN + TAG_LEN || passphrase == null) {
            return null;
        }
        try {
            ByteBuffer buf = ByteBuffer.wrap(data);
            byte[] magic = new byte[4];
            buf.get(magic);
            if (!Arrays.equals(magic, MAGIC)) return null;
            byte ver = buf.get();
            if (ver != VERSION) return null;
            byte[] salt = new byte[SALT_LEN];
            byte[] iv = new byte[IV_LEN];
            buf.get(salt);
            buf.get(iv);
            int remaining = buf.remaining();
            if (remaining < TAG_LEN) return null;
            int ctLen = remaining - TAG_LEN;
            byte[] ct = new byte[ctLen];
            byte[] tag = new byte[TAG_LEN];
            buf.get(ct);
            buf.get(tag);

            byte[] key = deriveKey(passphrase, salt, PBKDF2_ITERS);
            if (key == null) return null;
            try {
                Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
                GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_LEN * 8, iv);
                cipher.init(Cipher.DECRYPT_MODE, new javax.crypto.spec.SecretKeySpec(key, "AES"), gcmSpec);
                byte[] ctWithTag = new byte[ct.length + tag.length];
                System.arraycopy(ct, 0, ctWithTag, 0, ct.length);
                System.arraycopy(tag, 0, ctWithTag, ct.length, tag.length);
                return cipher.doFinal(ctWithTag);
            } catch (AEADBadTagException ex) {
                return null;
            } catch (GeneralSecurityException ex) {
                return null;
            } finally {
                Arrays.fill(key, (byte) 0);
            }
        } catch (RuntimeException e) {
            return null;
        }
    }

    public static KeyPair generateRSA(int bits) {
        try {
            KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA");
            kpg.initialize(bits, new SecureRandom());
            return kpg.generateKeyPair();
        } catch (GeneralSecurityException e) {
            return null;
        }
    }

    public static byte[] rsaEncrypt(byte[] plaintext, PublicKey publicKey) {
        try {
            Cipher cipher = Cipher.getInstance("RSA/ECB/OAEPWithSHA-256AndMGF1Padding");
            OAEPParameterSpec oaep = new OAEPParameterSpec(
                    "SHA-256",
                    "MGF1",
                    MGF1ParameterSpec.SHA256,
                    PSource.PSpecified.DEFAULT
            );
            cipher.init(Cipher.ENCRYPT_MODE, publicKey, oaep);
            return cipher.doFinal(plaintext);
        } catch (GeneralSecurityException e) {
            return null;
        }
    }

    public static byte[] rsaDecrypt(byte[] ciphertext, PrivateKey privateKey) {
        try {
            Cipher cipher = Cipher.getInstance("RSA/ECB/OAEPWithSHA-256AndMGF1Padding");
            OAEPParameterSpec oaep = new OAEPParameterSpec(
                    "SHA-256",
                    "MGF1",
                    MGF1ParameterSpec.SHA256,
                    PSource.PSpecified.DEFAULT
            );
            cipher.init(Cipher.DECRYPT_MODE, privateKey, oaep);
            return cipher.doFinal(ciphertext);
        } catch (GeneralSecurityException e) {
            return null;
        }
    }

    public static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int res = 0;
        for (int i = 0; i < a.length; i++) {
            res |= (a[i] ^ b[i]);
        }
        return res == 0;
    }

    public static void main(String[] args) {
        boolean all = true;

        // Test 1: AES-GCM roundtrip
        byte[] msg1 = "Secret message 1".getBytes(StandardCharsets.UTF_8);
        char[] pw1 = "correct horse battery staple".toCharArray();
        byte[] enc1 = encrypt(msg1, pw1);
        byte[] dec1 = decrypt(enc1, pw1);
        boolean t1 = dec1 != null && constantTimeEquals(msg1, dec1);
        System.out.println("Test1 AES roundtrip: " + t1);
        all &= t1;

        // Test 2: AES-GCM wrong passphrase should fail
        char[] pwWrong = "wrong pass".toCharArray();
        byte[] dec2 = decrypt(enc1, pwWrong);
        boolean t2 = (dec2 == null);
        System.out.println("Test2 AES wrong passphrase: " + t2);
        all &= t2;

        // Test 3: AES-GCM tamper detection should fail
        byte[] tampered = Arrays.copyOf(enc1, enc1.length);
        int payloadStart = MAGIC.length + 1 + SALT_LEN + IV_LEN;
        if (tampered.length > payloadStart + 0) {
            tampered[payloadStart] ^= 0x01;
        }
        byte[] dec3 = decrypt(tampered, pw1);
        boolean t3 = (dec3 == null);
        System.out.println("Test3 AES tamper detection: " + t3);
        all &= t3;

        // Test 4: RSA OAEP roundtrip
        KeyPair kp = generateRSA(2048);
        byte[] msg2 = "RSA secret".getBytes(StandardCharsets.UTF_8);
        byte[] renc = rsaEncrypt(msg2, kp.getPublic());
        byte[] rdec = rsaDecrypt(renc, kp.getPrivate());
        boolean t4 = rdec != null && constantTimeEquals(msg2, rdec);
        System.out.println("Test4 RSA OAEP roundtrip: " + t4);
        all &= t4;

        // Test 5: RSA decrypt with wrong key should fail
        KeyPair kp2 = generateRSA(2048);
        byte[] rdecWrong = rsaDecrypt(renc, kp2.getPrivate());
        boolean t5 = (rdecWrong == null);
        System.out.println("Test5 RSA wrong key fails: " + t5);
        all &= t5;

        System.out.println("All tests passed: " + all);

        // Clear sensitive arrays used in tests
        Arrays.fill(pw1, '\0');
        Arrays.fill(pwWrong, '\0');
        if (dec1 != null) Arrays.fill(dec1, (byte) 0);
        if (msg1 != null) Arrays.fill(msg1, (byte) 0);
        if (msg2 != null) Arrays.fill(msg2, (byte) 0);
    }
}