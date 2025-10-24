import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.PosixFilePermission;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.EnumSet;
import java.util.Set;

public class Task89 {

    // Secure constants
    private static final String MAGIC = "PWD1";
    private static final byte VERSION = 1;
    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32;
    private static final int MIN_ITERATIONS = 210000;

    public static void main(String[] args) {
        String baseDir = "safe_store_java";
        try {
            Files.createDirectories(Paths.get(baseDir));
        } catch (IOException e) {
            System.out.println("Failed to create base dir");
            return;
        }

        // Test 1: Store and verify correct password
        boolean t1Store = storePassword(baseDir, "user1.pwd", "Password#123".toCharArray(), 210000);
        boolean t1Verify = verifyPassword(baseDir, "user1.pwd", "Password#123".toCharArray());
        System.out.println("Test1 (store+verify correct): " + (t1Store && t1Verify));

        // Test 2: Verify incorrect password
        boolean t2Verify = verifyPassword(baseDir, "user1.pwd", "WrongPassword".toCharArray());
        System.out.println("Test2 (verify incorrect): " + (!t2Verify));

        // Test 3: Path traversal attempt should fail
        boolean t3Verify = verifyPassword(baseDir, "../evil.pwd", "anything".toCharArray());
        System.out.println("Test3 (path traversal blocked): " + (!t3Verify));

        // Test 4: Non-existent file
        boolean t4Verify = verifyPassword(baseDir, "does_not_exist.pwd", "Password#123".toCharArray());
        System.out.println("Test4 (non-existent file): " + (!t4Verify));

        // Test 5: Second user with different password
        boolean t5Store = storePassword(baseDir, "user2.pwd", "Abc!2345".toCharArray(), 300000);
        boolean t5Verify = verifyPassword(baseDir, "user2.pwd", "Abc!2345".toCharArray());
        System.out.println("Test5 (second user correct): " + (t5Store && t5Verify));
    }

    public static boolean storePassword(String baseDir, String relPath, char[] password, int iterations) {
        if (password == null || relPath == null || baseDir == null) return false;
        iterations = Math.max(iterations, MIN_ITERATIONS);
        byte[] salt = new byte[SALT_LEN];
        byte[] hash = new byte[HASH_LEN];
        SecureRandom rng = new SecureRandom();
        rng.nextBytes(salt);
        try {
            hash = deriveKey(password, salt, iterations, HASH_LEN);
        } catch (Exception e) {
            zeroBytes(salt);
            zeroChars(password);
            return false;
        }
        Path base;
        try {
            base = Paths.get(baseDir).toAbsolutePath().normalize();
            if (!Files.exists(base)) Files.createDirectories(base);
        } catch (IOException e) {
            zeroBytes(salt);
            zeroBytes(hash);
            zeroChars(password);
            return false;
        }
        Path target;
        try {
            target = resolveSafePath(baseDir, relPath);
        } catch (IOException e) {
            zeroBytes(salt);
            zeroBytes(hash);
            zeroChars(password);
            return false;
        }

        byte[] content;
        try {
            content = serializeRecord(iterations, salt, hash);
        } catch (IOException e) {
            zeroBytes(salt);
            zeroBytes(hash);
            zeroChars(password);
            return false;
        }

        Path tmp = null;
        try {
            tmp = Files.createTempFile(base, "tmp-", ".pwd");
            setOwnerOnlyPermissions(tmp);
            try (FileChannel fc = FileChannel.open(tmp, StandardOpenOption.WRITE);
                 FileLock lock = fc.lock()) {
                fc.write(ByteBuffer.wrap(content));
                fc.force(true);
            }
            // Atomic move into place
            Files.move(tmp, target, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
            setOwnerOnlyPermissions(target);
            return true;
        } catch (IOException e) {
            try {
                if (tmp != null) Files.deleteIfExists(tmp);
            } catch (IOException ignored) { }
            return false;
        } finally {
            zeroBytes(salt);
            zeroBytes(hash);
            zeroBytes(content);
            zeroChars(password);
        }
    }

    public static boolean verifyPassword(String baseDir, String relPath, char[] inputPassword) {
        if (inputPassword == null || relPath == null || baseDir == null) return false;
        Path path;
        try {
            path = resolveSafePath(baseDir, relPath);
        } catch (IOException e) {
            zeroChars(inputPassword);
            return false;
        }

        // Refuse symlinks and non-regular files
        try {
            BasicFileAttributes attrs = Files.readAttributes(path, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            if (!attrs.isRegularFile()) {
                zeroChars(inputPassword);
                return false;
            }
        } catch (IOException e) {
            zeroChars(inputPassword);
            return false;
        }

        byte[] data;
        try (FileChannel fc = FileChannel.open(path, StandardOpenOption.READ);
             FileLock lock = fc.lock(0L, Long.MAX_VALUE, true)) {
            long size = fc.size();
            if (size <= 0 || size > (1 << 20)) { // limit to 1MB
                zeroChars(inputPassword);
                return false;
            }
            data = new byte[(int) size];
            ByteBuffer bb = ByteBuffer.wrap(data);
            while (bb.hasRemaining()) {
                if (fc.read(bb) < 0) break;
            }
        } catch (IOException e) {
            zeroChars(inputPassword);
            return false;
        }

        try {
            ParsedRecord rec = parseRecord(data);
            if (rec.iterations < MIN_ITERATIONS || rec.salt.length != SALT_LEN || rec.hash.length != HASH_LEN) {
                zeroChars(inputPassword);
                zeroBytes(data);
                zeroBytes(rec.salt);
                zeroBytes(rec.hash);
                return false;
            }
            byte[] computed = deriveKey(inputPassword, rec.salt, rec.iterations, HASH_LEN);
            boolean ok = constantTimeEquals(computed, rec.hash);
            zeroBytes(computed);
            zeroChars(inputPassword);
            zeroBytes(data);
            zeroBytes(rec.salt);
            zeroBytes(rec.hash);
            return ok;
        } catch (Exception e) {
            zeroChars(inputPassword);
            zeroBytes(data);
            return false;
        }
    }

    private static byte[] deriveKey(char[] password, byte[] salt, int iterations, int len) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, len * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] out = skf.generateSecret(spec).getEncoded();
        spec.clearPassword();
        return out;
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        return MessageDigest.isEqual(a, b);
    }

    private static byte[] serializeRecord(int iterations, byte[] salt, byte[] hash) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream(4 + 1 + 4 + 1 + 1 + salt.length + hash.length);
        bos.write(MAGIC.getBytes(StandardCharsets.US_ASCII));
        bos.write(VERSION);
        bos.write(ByteBuffer.allocate(4).putInt(iterations).array());
        bos.write((byte) salt.length);
        bos.write((byte) hash.length);
        bos.write(salt);
        bos.write(hash);
        return bos.toByteArray();
    }

    private static ParsedRecord parseRecord(byte[] data) throws IOException {
        if (data.length < 4 + 1 + 4 + 1 + 1) throw new IOException("Too short");
        int idx = 0;
        String magic = new String(data, idx, 4, StandardCharsets.US_ASCII);
        if (!MAGIC.equals(magic)) throw new IOException("Bad magic");
        idx += 4;
        byte ver = data[idx++];
        if (ver != VERSION) throw new IOException("Bad version");
        int iterations = ByteBuffer.wrap(data, idx, 4).getInt();
        idx += 4;
        int sLen = data[idx++] & 0xFF;
        int hLen = data[idx++] & 0xFF;
        if (sLen != SALT_LEN || hLen != HASH_LEN) throw new IOException("Bad lengths");
        if (data.length != 4 + 1 + 4 + 1 + 1 + sLen + hLen) throw new IOException("Bad size");
        byte[] salt = new byte[sLen];
        byte[] hash = new byte[hLen];
        System.arraycopy(data, idx, salt, 0, sLen);
        idx += sLen;
        System.arraycopy(data, idx, hash, 0, hLen);
        return new ParsedRecord(iterations, salt, hash);
    }

    private static Path resolveSafePath(String baseDir, String userRelPath) throws IOException {
        Path base = Paths.get(baseDir).toAbsolutePath().normalize();
        Path target = base.resolve(userRelPath).normalize();
        if (!target.startsWith(base)) throw new IOException("Path escapes base");
        if (Files.exists(target, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(target)) {
            throw new IOException("Symlink refused");
        }
        return target;
    }

    private static void setOwnerOnlyPermissions(Path path) {
        try {
            Set<PosixFilePermission> perms = EnumSet.of(
                    PosixFilePermission.OWNER_READ,
                    PosixFilePermission.OWNER_WRITE
            );
            Files.setPosixFilePermissions(path, perms);
        } catch (UnsupportedOperationException | IOException ignored) { }
    }

    private static void zeroBytes(byte[] b) {
        if (b != null) {
            for (int i = 0; i < b.length; i++) b[i] = 0;
        }
    }

    private static void zeroChars(char[] c) {
        if (c != null) {
            for (int i = 0; i < c.length; i++) c[i] = 0;
        }
    }

    private static final class ParsedRecord {
        final int iterations;
        final byte[] salt;
        final byte[] hash;
        ParsedRecord(int iterations, byte[] salt, byte[] hash) {
            this.iterations = iterations;
            this.salt = salt;
            this.hash = hash;
        }
    }
}