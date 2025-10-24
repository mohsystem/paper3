import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.Set;

public class Task124 {

    // Magic header to identify our blob format and version
    private static final byte[] MAGIC = new byte[] { 'T', 'S', 'K', '1' };
    private static final int SALT_LEN = 16;
    private static final int IV_LEN = 12; // Recommended for GCM
    private static final int TAG_LEN_BITS = 128;
    private static final int KEY_LEN_BITS = 256;
    private static final int PBKDF2_ITERS = 200_000;

    private static final SecureRandom RNG = new SecureRandom();

    // Utility: constant-time array comparison (not strictly needed with GCM, but good hygiene)
    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int r = 0;
        for (int i = 0; i < a.length; i++) r |= (a[i] ^ b[i]);
        return r == 0;
    }

    private static byte[] deriveKey(char[] password, byte[] salt) throws GeneralSecurityException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, PBKDF2_ITERS, KEY_LEN_BITS);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] key = skf.generateSecret(spec).getEncoded();
            return key;
        } finally {
            spec.clearPassword();
        }
    }

    public static byte[] encrypt(byte[] plaintext, char[] password) throws GeneralSecurityException {
        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        byte[] keyBytes = null;
        byte[] iv = new byte[IV_LEN];
        RNG.nextBytes(iv);
        byte[] ciphertext = null;

        try {
            keyBytes = deriveKey(password, salt);
            SecretKeySpec key = new SecretKeySpec(keyBytes, "AES");
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_LEN_BITS, iv);
            cipher.init(Cipher.ENCRYPT_MODE, key, gcmSpec);
            cipher.updateAAD(MAGIC);
            ciphertext = cipher.doFinal(plaintext);

            ByteBuffer out = ByteBuffer.allocate(MAGIC.length + SALT_LEN + IV_LEN + ciphertext.length);
            out.put(MAGIC);
            out.put(salt);
            out.put(iv);
            out.put(ciphertext);
            return out.array();
        } finally {
            if (keyBytes != null) Arrays.fill(keyBytes, (byte) 0);
            if (plaintext != null) Arrays.fill(plaintext, (byte) 0);
            if (salt != null) Arrays.fill(salt, (byte) 0);
            if (iv != null) Arrays.fill(iv, (byte) 0);
        }
    }

    public static byte[] decrypt(byte[] blob, char[] password) throws GeneralSecurityException {
        if (blob == null || blob.length < MAGIC.length + SALT_LEN + IV_LEN + 16) {
            throw new GeneralSecurityException("Invalid blob");
        }
        ByteBuffer in = ByteBuffer.wrap(blob);
        byte[] magic = new byte[MAGIC.length];
        in.get(magic);
        if (!constantTimeEquals(magic, MAGIC)) {
            throw new GeneralSecurityException("Invalid header");
        }
        byte[] salt = new byte[SALT_LEN];
        in.get(salt);
        byte[] iv = new byte[IV_LEN];
        in.get(iv);
        byte[] ct = new byte[in.remaining()];
        in.get(ct);

        byte[] keyBytes = null;
        try {
            keyBytes = deriveKey(password, salt);
            SecretKeySpec key = new SecretKeySpec(keyBytes, "AES");
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_LEN_BITS, iv);
            cipher.init(Cipher.DECRYPT_MODE, key, gcmSpec);
            cipher.updateAAD(MAGIC);
            return cipher.doFinal(ct);
        } finally {
            if (keyBytes != null) Arrays.fill(keyBytes, (byte) 0);
            if (salt != null) Arrays.fill(salt, (byte) 0);
            if (iv != null) Arrays.fill(iv, (byte) 0);
            if (ct != null) Arrays.fill(ct, (byte) 0);
        }
    }

    public static void storeToFile(String sensitiveData, char[] password, Path path) throws IOException, GeneralSecurityException {
        byte[] plaintext = sensitiveData.getBytes(StandardCharsets.UTF_8);
        byte[] blob = null;
        try {
            blob = encrypt(plaintext, password);

            Path dir = path.getParent() != null ? path.getParent() : Paths.get(".");
            String tmpName = path.getFileName().toString() + ".tmp-" + Long.toUnsignedString(RNG.nextLong(), 36);
            Path tmp = dir.resolve(tmpName);

            // Ensure restrictive permissions where possible
            try {
                Set<PosixFilePermission> perms = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE
                );
                Files.createFile(tmp, PosixFilePermissions.asFileAttribute(perms));
            } catch (UnsupportedOperationException e) {
                Files.createFile(tmp);
            }

            Files.write(tmp, blob, StandardOpenOption.WRITE);
            try {
                Files.move(tmp, path, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tmp, path, StandardCopyOption.REPLACE_EXISTING);
            }
        } finally {
            if (blob != null) Arrays.fill(blob, (byte) 0);
            Arrays.fill(password, '\0');
        }
    }

    public static String loadFromFile(char[] password, Path path) throws IOException, GeneralSecurityException {
        byte[] blob = Files.readAllBytes(path);
        byte[] plaintext = null;
        try {
            plaintext = decrypt(blob, password);
            return new String(plaintext, StandardCharsets.UTF_8);
        } finally {
            if (plaintext != null) Arrays.fill(plaintext, (byte) 0);
            Arrays.fill(password, '\0');
            if (blob != null) Arrays.fill(blob, (byte) 0);
        }
    }

    // Test helper: flip one byte in file to simulate tampering
    private static void flipOneByte(Path path) throws IOException {
        byte[] data = Files.readAllBytes(path);
        if (data.length > MAGIC.length + SALT_LEN + IV_LEN + 16) {
            int idx = data.length - 1 - RNG.nextInt(Math.max(1, Math.min(32, data.length - (MAGIC.length + SALT_LEN + IV_LEN))));
            data[idx] ^= 0x01;
            Files.write(path, data, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
        }
        Arrays.fill(data, (byte) 0);
    }

    public static void main(String[] args) {
        try {
            Path tmpDir = Files.createTempDirectory("task124-securestore");
            Path f1 = tmpDir.resolve("cc1.bin");
            Path f2 = tmpDir.resolve("pii2.bin");
            Path f3 = tmpDir.resolve("wrongpass.bin");
            Path f4 = tmpDir.resolve("tamper.bin");
            Path f5 = tmpDir.resolve("unicode.bin");

            // Test 1: Store and retrieve credit card-like data
            String t1 = "cc=4111111111111111;exp=12/28;cvc=123;name=Jane Doe";
            char[] p1 = "StrongPassw0rd!".toCharArray();
            storeToFile(t1, Arrays.copyOf(p1, p1.length), f1);
            String r1 = loadFromFile(Arrays.copyOf(p1, p1.length), f1);
            System.out.println("Test1 OK: " + (r1.length() == t1.length()));

            // Test 2: Store and retrieve PII
            String t2 = "name=John Doe;ssn=123-45-6789;dob=1970-01-01;addr=123 Main St";
            char[] p2 = "An0ther$trongPass".toCharArray();
            storeToFile(t2, Arrays.copyOf(p2, p2.length), f2);
            String r2 = loadFromFile(Arrays.copyOf(p2, p2.length), f2);
            System.out.println("Test2 OK: " + (r2.length() == t2.length()));

            // Test 3: Wrong password should fail
            String t3 = "email=jane@example.com;phone=+1-555-555-5555";
            char[] p3 = "CorrectHorseBatteryStaple!".toCharArray();
            storeToFile(t3, Arrays.copyOf(p3, p3.length), f3);
            boolean wrongPassFailed = false;
            try {
                loadFromFile("TotallyWrongPassword".toCharArray(), f3);
            } catch (GeneralSecurityException ex) {
                wrongPassFailed = true;
            }
            System.out.println("Test3 Wrong Password Rejected: " + wrongPassFailed);

            // Test 4: Tampering detection
            String t4 = "passport=Z12345678;country=US;expiry=2030-12-31";
            char[] p4 = "TamperProofKey#2024".toCharArray();
            storeToFile(t4, Arrays.copyOf(p4, p4.length), f4);
            flipOneByte(f4);
            boolean tamperDetected = false;
            try {
                loadFromFile(Arrays.copyOf(p4, p4.length), f4);
            } catch (GeneralSecurityException ex) {
                tamperDetected = true;
            }
            System.out.println("Test4 Tampering Detected: " + tamperDetected);

            // Test 5: Unicode and empty fields handling
            String t5 = "name=Мария;note=机密情報🔒;extra=";
            char[] p5 = "Un1c0de-安全-كلمهسر".toCharArray();
            storeToFile(t5, Arrays.copyOf(p5, p5.length), f5);
            String r5 = loadFromFile(Arrays.copyOf(p5, p5.length), f5);
            System.out.println("Test5 OK: " + (r5.length() == t5.length()));

            // Wipe test passwords
            Arrays.fill(p1, '\0');
            Arrays.fill(p2, '\0');
            Arrays.fill(p3, '\0');
            Arrays.fill(p4, '\0');
            Arrays.fill(p5, '\0');

        } catch (Exception e) {
            // Do not print sensitive data, only the error class
            System.out.println("Error: " + e.getClass().getSimpleName());
        }
    }
}