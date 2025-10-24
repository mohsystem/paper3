import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.Set;

public class Task52 {

    private static final byte[] MAGIC = new byte[]{'E', 'N', 'C', '1'};
    private static final byte VERSION = 1;
    private static final int SALT_LEN = 16;
    private static final int IV_LEN = 12;
    private static final int GCM_TAG_BITS = 128;
    private static final int KEY_BITS = 256;
    private static final int PBKDF2_ITERS = 210_000;

    public static boolean encryptFile(Path baseDir, String inputRel, String outputRel, char[] passphrase) {
        SecureRandom rng = new SecureRandom();
        byte[] salt = new byte[SALT_LEN];
        byte[] iv = new byte[IV_LEN];
        rng.nextBytes(salt);
        rng.nextBytes(iv);

        Path absBase;
        Path inputPath = null;
        Path outputPath = null;
        Path tmpPath = null;
        SecretKeySpec key = null;

        try {
            absBase = baseDir.toAbsolutePath().normalize();
            inputPath = resolveSafePath(absBase, inputRel, false);
            outputPath = resolveSafePath(absBase, outputRel, true);

            if (!Files.isRegularFile(inputPath, LinkOption.NOFOLLOW_LINKS)) {
                return false;
            }
            Path outParent = outputPath.getParent();
            if (outParent == null || !Files.exists(outParent)) {
                return false;
            }
            if (Files.isSymbolicLink(outputPath)) {
                return false;
            }
            rejectSymlinksInPath(absBase, inputPath);
            rejectSymlinksInPath(absBase, outParent);

            key = deriveKey(passphrase, salt);

            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_BITS, iv);
            cipher.init(Cipher.ENCRYPT_MODE, key, spec);

            FileAttribute<?>[] attrs = secureFileAttributes();
            tmpPath = Files.createTempFile(outParent, ".enc", ".tmp", attrs);
            setOwnerOnlyPermissions(tmpPath);

            try (InputStream in = Files.newInputStream(inputPath, StandardOpenOption.READ);
                 FileChannel outCh = FileChannel.open(tmpPath, StandardOpenOption.WRITE);
                 OutputStream outRaw = Channels.newOutputStream(outCh)) {

                // Write header: MAGIC + VERSION + SALT + IV
                ByteBuffer header = ByteBuffer.allocate(MAGIC.length + 1 + SALT_LEN + IV_LEN);
                header.put(MAGIC);
                header.put(VERSION);
                header.put(salt);
                header.put(iv);
                header.flip();
                outCh.write(header);

                try (CipherOutputStream cos = new CipherOutputStream(outRaw, cipher)) {
                    byte[] buffer = new byte[8192];
                    int read;
                    while ((read = in.read(buffer)) != -1) {
                        cos.write(buffer, 0, read);
                    }
                    cos.flush();
                }

                outCh.force(true);
            }

            try {
                Files.move(tmpPath, outputPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tmpPath, outputPath, StandardCopyOption.REPLACE_EXISTING);
            }
            setOwnerOnlyPermissions(outputPath);
            return true;
        } catch (Exception e) {
            try {
                if (tmpPath != null) {
                    Files.deleteIfExists(tmpPath);
                }
            } catch (IOException ignore) {
            }
            return false;
        } finally {
            if (key != null) {
                Arrays.fill(key.getEncoded(), (byte) 0);
            }
            Arrays.fill(salt, (byte) 0);
            Arrays.fill(iv, (byte) 0);
        }
    }

    public static boolean decryptFile(Path baseDir, String inputRel, String outputRel, char[] passphrase) {
        Path absBase;
        Path inputPath = null;
        Path outputPath = null;
        Path tmpPath = null;
        SecretKeySpec key = null;
        byte[] salt = new byte[SALT_LEN];
        byte[] iv = new byte[IV_LEN];

        try {
            absBase = baseDir.toAbsolutePath().normalize();
            inputPath = resolveSafePath(absBase, inputRel, false);
            outputPath = resolveSafePath(absBase, outputRel, true);

            if (!Files.isRegularFile(inputPath, LinkOption.NOFOLLOW_LINKS)) {
                return false;
            }
            Path outParent = outputPath.getParent();
            if (outParent == null || !Files.exists(outParent)) {
                return false;
            }
            if (Files.isSymbolicLink(outputPath)) {
                return false;
            }
            rejectSymlinksInPath(absBase, inputPath);
            rejectSymlinksInPath(absBase, outParent);

            // Read and validate header
            try (FileChannel inCh = FileChannel.open(inputPath, StandardOpenOption.READ)) {
                ByteBuffer header = ByteBuffer.allocate(MAGIC.length + 1 + SALT_LEN + IV_LEN);
                int read = inCh.read(header);
                if (read != header.capacity()) {
                    return false;
                }
                header.flip();
                byte[] magicRead = new byte[MAGIC.length];
                header.get(magicRead);
                if (!Arrays.equals(magicRead, MAGIC)) {
                    return false;
                }
                byte ver = header.get();
                if (ver != VERSION) {
                    return false;
                }
                header.get(salt);
                header.get(iv);

                key = deriveKey(passphrase, salt);
                Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
                cipher.init(Cipher.DECRYPT_MODE, key, new GCMParameterSpec(GCM_TAG_BITS, iv));

                FileAttribute<?>[] attrs = secureFileAttributes();
                tmpPath = Files.createTempFile(outParent, ".dec", ".tmp", attrs);
                setOwnerOnlyPermissions(tmpPath);

                try (InputStream rawIn = Channels.newInputStream(inCh.position(header.capacity()));
                     CipherInputStream cis = new CipherInputStream(rawIn, cipher);
                     FileChannel outCh = FileChannel.open(tmpPath, StandardOpenOption.WRITE);
                     OutputStream out = Channels.newOutputStream(outCh)) {

                    byte[] buffer = new byte[8192];
                    int n;
                    while ((n = cis.read(buffer)) != -1) {
                        out.write(buffer, 0, n);
                    }
                    out.flush();
                    outCh.force(true);
                } catch (IOException ex) {
                    // If AEAD tag fails, Java often throws IOException wrapping AEADBadTagException at read/close
                    try {
                        Files.deleteIfExists(tmpPath);
                    } catch (IOException ignore) {}
                    return false;
                }
            }

            try {
                Files.move(tmpPath, outputPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tmpPath, outputPath, StandardCopyOption.REPLACE_EXISTING);
            }
            setOwnerOnlyPermissions(outputPath);
            return true;
        } catch (Exception e) {
            try {
                if (tmpPath != null) {
                    Files.deleteIfExists(tmpPath);
                }
            } catch (IOException ignore) {
            }
            return false;
        } finally {
            Arrays.fill(salt, (byte) 0);
            Arrays.fill(iv, (byte) 0);
            if (key != null) {
                Arrays.fill(key.getEncoded(), (byte) 0);
            }
        }
    }

    private static SecretKeySpec deriveKey(char[] passphrase, byte[] salt) throws GeneralSecurityException {
        PBEKeySpec spec = new PBEKeySpec(passphrase, salt, PBKDF2_ITERS, KEY_BITS);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] keyBytes = skf.generateSecret(spec).getEncoded();
        return new SecretKeySpec(keyBytes, "AES");
    }

    private static Path resolveSafePath(Path base, String rel, boolean forWrite) throws IOException {
        if (rel == null) throw new IOException("Null path");
        Path relPath = Paths.get(rel);
        if (relPath.isAbsolute()) throw new IOException("Absolute path not allowed");
        Path candidate = base.resolve(relPath).normalize();
        Path absBase = base.toAbsolutePath().normalize();
        if (!candidate.toAbsolutePath().normalize().startsWith(absBase)) {
            throw new IOException("Path escapes base directory");
        }
        if (!Files.exists(absBase)) {
            throw new IOException("Base does not exist");
        }
        if (!Files.isDirectory(absBase)) {
            throw new IOException("Base is not directory");
        }
        if (!forWrite) {
            if (!Files.exists(candidate)) {
                throw new IOException("Input does not exist");
            }
            if (!Files.isRegularFile(candidate, LinkOption.NOFOLLOW_LINKS)) {
                throw new IOException("Input not a regular file");
            }
        } else {
            Path parent = candidate.getParent();
            if (parent == null || !Files.exists(parent) || !Files.isDirectory(parent, LinkOption.NOFOLLOW_LINKS)) {
                throw new IOException("Output parent invalid");
            }
        }
        return candidate;
    }

    private static void rejectSymlinksInPath(Path base, Path path) throws IOException {
        Path absBase = base.toAbsolutePath().normalize();
        Path absPath = path.toAbsolutePath().normalize();
        if (!absPath.startsWith(absBase)) {
            throw new IOException("Path escapes base");
        }
        Path rel = absBase.relativize(absPath);
        Path cur = absBase;
        for (Path part : rel) {
            cur = cur.resolve(part);
            if (Files.exists(cur, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(cur)) {
                throw new IOException("Symlink encountered in path");
            }
        }
    }

    private static FileAttribute<?>[] secureFileAttributes() {
        try {
            Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
            return new FileAttribute[]{PosixFilePermissions.asFileAttribute(perms)};
        } catch (UnsupportedOperationException e) {
            return new FileAttribute<?>[0];
        }
    }

    private static void setOwnerOnlyPermissions(Path p) {
        try {
            Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
            Files.setPosixFilePermissions(p, perms);
        } catch (UnsupportedOperationException | IOException ignored) {
        }
    }

    public static void main(String[] args) throws Exception {
        Path base = Files.createTempDirectory("enc_base_java");
        char[] pass = "StrongPassphrase!123".toCharArray();

        // Create 5 test files
        byte[][] contents = new byte[5][];
        contents[0] = new byte[0];
        contents[1] = "Hello, World!".getBytes(StandardCharsets.UTF_8);
        contents[2] = "こんにちは世界🌏".getBytes(StandardCharsets.UTF_8);
        contents[3] = new byte[1024];
        new SecureRandom().nextBytes(contents[3]);
        contents[4] = new byte[128 * 1024];
        new SecureRandom().nextBytes(contents[4]);

        boolean allOk = true;
        for (int i = 0; i < 5; i++) {
            Path in = base.resolve("input_" + i + ".bin");
            Files.write(in, contents[i], StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
            Path enc = base.resolve("out_" + i + ".enc");
            Path dec = base.resolve("dec_" + i + ".bin");
            boolean encOk = encryptFile(base, "input_" + i + ".bin", "out_" + i + ".enc", pass);
            boolean decOk = decryptFile(base, "out_" + i + ".enc", "dec_" + i + ".bin", pass);
            boolean match = false;
            if (encOk && decOk) {
                byte[] decData = Files.readAllBytes(dec);
                match = Arrays.equals(decData, contents[i]);
            }
            System.out.println("Test " + i + " => encrypt: " + encOk + ", decrypt: " + decOk + ", match: " + match);
            allOk = allOk && encOk && decOk && match;
        }
        System.out.println("Overall: " + (allOk ? "SUCCESS" : "FAIL"));
        Arrays.fill(pass, '\0');
    }
}