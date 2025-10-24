import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task124 {
    private static final byte[] MAGIC = new byte[]{'E', 'N', 'C', '1'};
    private static final byte VERSION = 1;
    private static final int SALT_LEN = 16;
    private static final int IV_LEN = 12;
    private static final int TAG_LEN = 16;
    private static final int PBKDF2_ITERATIONS = 210_000;
    private static final int KEY_LEN_BITS = 256;

    public static byte[] encryptToBytes(String passphrase, byte[] plaintext) throws GeneralSecurityException {
        if (passphrase == null || plaintext == null) {
            throw new IllegalArgumentException("Invalid inputs");
        }
        final SecureRandom rng = new SecureRandom();
        byte[] salt = new byte[SALT_LEN];
        byte[] iv = new byte[IV_LEN];
        rng.nextBytes(salt);
        rng.nextBytes(iv);

        SecretKeySpec key = deriveKey(passphrase, salt);

        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_LEN * 8, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, gcmSpec);
        byte[] ctAndTag = cipher.doFinal(plaintext);

        if (ctAndTag.length < TAG_LEN) {
            throw new GeneralSecurityException("Encryption failed");
        }
        int ctLen = ctAndTag.length - TAG_LEN;
        byte[] ciphertext = Arrays.copyOfRange(ctAndTag, 0, ctLen);
        byte[] tag = Arrays.copyOfRange(ctAndTag, ctLen, ctAndTag.length);

        ByteBuffer buf = ByteBuffer.allocate(MAGIC.length + 1 + SALT_LEN + IV_LEN + ciphertext.length + TAG_LEN);
        buf.put(MAGIC);
        buf.put(VERSION);
        buf.put(salt);
        buf.put(iv);
        buf.put(ciphertext);
        buf.put(tag);
        return buf.array();
    }

    public static byte[] decryptFromBytes(String passphrase, byte[] blob) throws GeneralSecurityException {
        if (passphrase == null || blob == null) {
            throw new IllegalArgumentException("Invalid inputs");
        }
        int minLen = MAGIC.length + 1 + SALT_LEN + IV_LEN + TAG_LEN;
        if (blob.length < minLen) {
            throw new GeneralSecurityException("Invalid blob length");
        }
        ByteBuffer buf = ByteBuffer.wrap(blob);
        byte[] magic = new byte[4];
        buf.get(magic);
        if (!Arrays.equals(magic, MAGIC)) {
            throw new GeneralSecurityException("Bad magic");
        }
        byte version = buf.get();
        if (version != VERSION) {
            throw new GeneralSecurityException("Unsupported version");
        }
        byte[] salt = new byte[SALT_LEN];
        buf.get(salt);
        byte[] iv = new byte[IV_LEN];
        buf.get(iv);
        int remaining = buf.remaining();
        if (remaining < TAG_LEN) {
            throw new GeneralSecurityException("Invalid blob");
        }
        byte[] ciphertext = new byte[remaining - TAG_LEN];
        byte[] tag = new byte[TAG_LEN];
        buf.get(ciphertext);
        buf.get(tag);

        SecretKeySpec key = deriveKey(passphrase, salt);

        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_LEN * 8, iv);
        cipher.init(Cipher.DECRYPT_MODE, key, gcmSpec);
        byte[] ctAndTag = new byte[ciphertext.length + TAG_LEN];
        System.arraycopy(ciphertext, 0, ctAndTag, 0, ciphertext.length);
        System.arraycopy(tag, 0, ctAndTag, ciphertext.length, TAG_LEN);
        return cipher.doFinal(ctAndTag);
    }

    public static boolean saveEncrypted(String baseDir, String userRelativePath, String passphrase, byte[] data) {
        try {
            Path base = Paths.get(baseDir).toAbsolutePath().normalize();
            Path target = resolveSafePath(base, userRelativePath);
            byte[] enc = encryptToBytes(passphrase, data);
            Files.createDirectories(target.getParent());
            Path tmp = target.resolveSibling(target.getFileName().toString() + ".tmp");
            Files.write(tmp, enc, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            try {
                Files.move(tmp, target, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tmp, target, StandardCopyOption.REPLACE_EXISTING);
            }
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    public static byte[] loadDecrypted(String baseDir, String userRelativePath, String passphrase) throws GeneralSecurityException, IOException {
        Path base = Paths.get(baseDir).toAbsolutePath().normalize();
        Path target = resolveSafePath(base, userRelativePath);
        byte[] blob = Files.readAllBytes(target);
        return decryptFromBytes(passphrase, blob);
    }

    private static SecretKeySpec deriveKey(String passphrase, byte[] salt) throws GeneralSecurityException {
        PBEKeySpec spec = new PBEKeySpec(passphrase.toCharArray(), salt, PBKDF2_ITERATIONS, KEY_LEN_BITS);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] keyBytes = skf.generateSecret(spec).getEncoded();
        return new SecretKeySpec(keyBytes, "AES");
    }

    private static Path resolveSafePath(Path baseDir, String userRelativePath) throws IOException {
        if (userRelativePath == null || userRelativePath.isEmpty()) {
            throw new IOException("Invalid path");
        }
        if (userRelativePath.startsWith("/") || userRelativePath.startsWith("\\") || userRelativePath.contains("..")) {
            throw new IOException("Path traversal detected");
        }
        Path candidate = baseDir.resolve(userRelativePath).normalize().toAbsolutePath();
        if (!candidate.startsWith(baseDir)) {
            throw new IOException("Resolved path escapes base directory");
        }
        return candidate;
    }

    private static String randomPassphrase(int bytesLen) {
        byte[] buf = new byte[bytesLen];
        new SecureRandom().nextBytes(buf);
        StringBuilder sb = new StringBuilder(buf.length * 2);
        for (byte b : buf) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        String base = "secure_store_java";
        String[] files = {"rec1.bin", "rec2.bin", "rec3.bin", "rec4.bin", "rec5.bin"};
        String[] passphrases = {
                randomPassphrase(16),
                randomPassphrase(16),
                randomPassphrase(16),
                randomPassphrase(16),
                randomPassphrase(16)
        };
        String[] records = {
                "User: Alice; CC: 4111-1111-1111-1111; Exp: 12/29",
                "User: Bob; SSN: 123-45-6789; DOB: 1990-01-01",
                "User: Carol; Passport: X12345678; Country: US",
                "User: Dave; Bank: 0123456789; Routing: 021000021",
                "User: Eve; Email: eve@example.com; Phone: +1-555-0100"
        };
        for (int i = 0; i < 5; i++) {
            boolean saved = saveEncrypted(base, files[i], passphrases[i], records[i].getBytes(StandardCharsets.UTF_8));
            if (!saved) {
                System.out.println("Test " + (i + 1) + " save failed");
                continue;
            }
            byte[] dec = loadDecrypted(base, files[i], passphrases[i]);
            String rec = new String(dec, StandardCharsets.UTF_8);
            if (rec.equals(records[i])) {
                System.out.println("Test " + (i + 1) + " OK");
            } else {
                System.out.println("Test " + (i + 1) + " mismatch");
            }
        }
    }
}