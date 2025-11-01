import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.SecureRandom;
import java.util.*;
import java.util.regex.Pattern;

public class Lse98 {

    private static final int ITERATIONS = 210_000;
    private static final int SALT_LEN = 16;
    private static final int DK_LEN = 32;
    private static final int MAX_USERS = 100_000;
    private static final SecureRandom RNG = new SecureRandom();
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{3,32}$");

    public static String registerUser(String dbPath, String username, String password) {
        if (dbPath == null || dbPath.isEmpty()) {
            return "ERROR: Invalid database path";
        }
        if (username == null || password == null) {
            return "ERROR: Null input";
        }
        if (!isValidUsername(username)) {
            return "ERROR: Invalid username (3-32 chars; allowed A-Z, a-z, 0-9, _ . -)";
        }
        String pwdPolicy = checkPasswordPolicy(password);
        if (pwdPolicy != null) {
            return "ERROR: " + pwdPolicy;
        }

        Path db = Paths.get(dbPath);
        Path dir = db.getParent();
        if (dir == null) dir = Paths.get(".");
        Map<String, String> existing = new LinkedHashMap<>();
        int linesRead = 0;

        try {
            if (Files.exists(db)) {
                try (BufferedReader br = Files.newBufferedReader(db, StandardCharsets.UTF_8)) {
                    String line;
                    while ((line = br.readLine()) != null) {
                        if (line.isEmpty() || line.startsWith("#")) continue;
                        String[] parts = line.split("\\|");
                        if (parts.length != 4) continue; // skip malformed lines
                        String u = parts[0];
                        existing.put(u, line);
                        linesRead++;
                        if (linesRead > MAX_USERS) {
                            return "ERROR: Database too large";
                        }
                    }
                }
                if (existing.containsKey(username)) {
                    return "ERROR: Username already exists";
                }
            }
        } catch (IOException e) {
            return "ERROR: Unable to read database";
        }

        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        char[] pwdChars = password.toCharArray();
        byte[] dk = null;
        try {
            dk = pbkdf2(pwdChars, salt, ITERATIONS, DK_LEN);
        } catch (Exception e) {
            Arrays.fill(salt, (byte) 0);
            Arrays.fill(pwdChars, '\0');
            return "ERROR: Hashing failed";
        } finally {
            Arrays.fill(pwdChars, '\0');
        }

        String record = username + "|" + ITERATIONS + "|" + toHex(salt) + "|" + toHex(dk);
        Arrays.fill(salt, (byte) 0);
        Arrays.fill(dk, (byte) 0);

        // Prepare new content
        List<String> outLines = new ArrayList<>(existing.values());
        outLines.add(record);

        // Atomic write
        Path tmp = null;
        try {
            FileAttribute<?>[] attrs = new FileAttribute<?>[0];
            try {
                if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                    Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                    attrs = new FileAttribute<?>[]{PosixFilePermissions.asFileAttribute(perms)};
                }
            } catch (Exception ignored) {}
            tmp = Files.createTempFile(dir, "users_tmp_", ".db", attrs);
            try (FileChannel ch = FileChannel.open(tmp, StandardOpenOption.WRITE);
                 BufferedWriter bw = Files.newBufferedWriter(tmp, StandardCharsets.UTF_8)) {
                for (String l : outLines) {
                    bw.write(l);
                    bw.newLine();
                }
                bw.flush();
                ch.force(true);
            }
            try {
                Files.move(tmp, db, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tmp, db, StandardCopyOption.REPLACE_EXISTING);
            }
        } catch (IOException e) {
            if (tmp != null) {
                try { Files.deleteIfExists(tmp); } catch (IOException ignored) {}
            }
            return "ERROR: Unable to write database";
        }

        return "OK";
    }

    private static boolean isValidUsername(String u) {
        return USERNAME_PATTERN.matcher(u).matches();
    }

    private static String checkPasswordPolicy(String p) {
        if (p.length() < 12) return "Password must be at least 12 characters long";
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (int i = 0; i < p.length(); i++) {
            char c = p.charAt(i);
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        if (!(hasUpper && hasLower && hasDigit && hasSpecial)) {
            return "Password must include upper, lower, digit, and special character";
        }
        return null;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    // 5 test cases
    public static void main(String[] args) {
        // Create a unique DB path for tests
        byte[] rnd = new byte[8];
        RNG.nextBytes(rnd);
        String dbPath = "users_java_" + toHex(rnd) + ".db";

        String[] usernames = {
                "alice",
                "bob",
                "invalid name",
                "alice", // duplicate
                "charlie"
        };

        String[] passwords = {
                "Str0ngP@ssw0rd!",
                "weakpass",
                "Val1dP@ssw0rd!!",
                "An0ther$tr0ng!",
                "An0th3r$tr0ng!"
        };

        for (int i = 0; i < usernames.length; i++) {
            String res = registerUser(dbPath, usernames[i], passwords[i]);
            System.out.println("Test " + (i + 1) + ": " + res);
        }
    }
}