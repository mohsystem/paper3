import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.File;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Base64;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse93 {

    private static final int SALT_LEN = 16;
    private static final int DK_LEN = 32;
    private static final int PBKDF2_ITERS = 210_000;
    private static final SecureRandom RNG = new SecureRandom();
    private static final Pattern USERNAME_RX = Pattern.compile("^[A-Za-z0-9_]{3,32}$");

    static final class HashResult {
        final String saltB64;
        final String hashB64;
        final int iterations;
        HashResult(String saltB64, String hashB64, int iterations) {
            this.saltB64 = saltB64;
            this.hashB64 = hashB64;
            this.iterations = iterations;
        }
    }

    public static boolean insertUser(String dbPath, String username, char[] password) {
        if (!isValidUsername(username)) {
            zeroize(password);
            return false;
        }
        if (!isStrongPassword(password)) {
            zeroize(password);
            return false;
        }

        Path path = Paths.get(dbPath);
        try {
            ensureDbFile(path);
        } catch (IOException e) {
            zeroize(password);
            return false;
        }

        // Check duplicate username
        try (BufferedReader br = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
            String line;
            while ((line = br.readLine()) != null) {
                String[] parts = line.split("\\|", -1);
                if (parts.length >= 1 && username.equals(parts[0])) {
                    zeroize(password);
                    return false;
                }
            }
        } catch (IOException e) {
            zeroize(password);
            return false;
        }

        HashResult hr;
        try {
            hr = hashPassword(password);
        } catch (Exception e) {
            zeroize(password);
            return false;
        } finally {
            zeroize(password);
        }

        String createdAt = Instant.now().toString();
        String record = username + "|" + hr.iterations + "|" + hr.saltB64 + "|" + hr.hashB64 + "|" + createdAt + "\n";

        try (BufferedWriter bw = Files.newBufferedWriter(path, StandardCharsets.UTF_8, StandardOpenOption.APPEND)) {
            bw.write(record);
            bw.flush();
        } catch (IOException e) {
            return false;
        }
        return true;
    }

    private static void ensureDbFile(Path path) throws IOException {
        if (Files.exists(path)) {
            return;
        }
        // Create with restrictive permissions if POSIX is supported
        try {
            Set<PosixFilePermission> perms = EnumSet.of(
                    PosixFilePermission.OWNER_READ,
                    PosixFilePermission.OWNER_WRITE
            );
            Files.createFile(path, PosixFilePermissions.asFileAttribute(perms));
        } catch (UnsupportedOperationException e) {
            // Fallback for non-POSIX (e.g., Windows)
            Files.createFile(path);
            File f = path.toFile();
            // Best-effort: restrict writable/readonly (not as strict as POSIX)
            f.setReadable(true, true);
            f.setWritable(true, true);
            f.setExecutable(false, false);
        }
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_RX.matcher(username).matches();
    }

    private static boolean isStrongPassword(char[] password) {
        if (password == null || password.length < 12) return false;
        boolean hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
        for (char c : password) {
            if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        return hasLower && hasUpper && hasDigit && hasSpecial;
    }

    private static byte[] randomBytes(int len) {
        byte[] out = new byte[len];
        RNG.nextBytes(out);
        return out;
    }

    private static HashResult hashPassword(char[] password) throws Exception {
        byte[] salt = randomBytes(SALT_LEN);
        PBEKeySpec spec = new PBEKeySpec(password, salt, PBKDF2_ITERS, DK_LEN * 8);
        byte[] dk = null;
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            dk = skf.generateSecret(spec).getEncoded();
            String saltB64 = Base64.getEncoder().encodeToString(salt);
            String hashB64 = Base64.getEncoder().encodeToString(dk);
            return new HashResult(saltB64, hashB64, PBKDF2_ITERS);
        } finally {
            spec.clearPassword();
            if (dk != null) zeroize(dk);
            zeroize(salt);
        }
    }

    private static void zeroize(char[] a) {
        if (a != null) java.util.Arrays.fill(a, '\0');
    }

    private static void zeroize(byte[] a) {
        if (a != null) java.util.Arrays.fill(a, (byte) 0);
    }

    public static void main(String[] args) throws IOException {
        String db = "users_java.db";
        // Fresh start for demo
        try {
            Files.deleteIfExists(Paths.get(db));
        } catch (IOException ignored) {}

        // 5 test cases
        String[][] tests = new String[][]{
            {"alice_01", "StrongPass!234"},
            {"bob_02", "Another$trongP4ss"},
            {"alice_01", "DupUserGoodP@ss1"}, // duplicate username
            {"ab", "ValidButUserTooShort1!"},
            {"charlie_03", "weakpass"} // weak password
        };

        for (int i = 0; i < tests.length; i++) {
            String u = tests[i][0];
            char[] p = tests[i][1].toCharArray();
            boolean ok = insertUser(db, u, p);
            System.out.println("Test " + (i+1) + " insert(" + u + "): " + (ok ? "OK" : "FAIL"));
        }

        // Show number of records stored
        int count = 0;
        try (BufferedReader br = Files.newBufferedReader(Paths.get(db), StandardCharsets.UTF_8)) {
            while (br.readLine() != null) count++;
        }
        System.out.println("Total records in DB file: " + count);
    }
}