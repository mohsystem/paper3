import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse91 {

    // Registers a user by storing username, salt, and hash in a CSV "database" file.
    // Returns a status message.
    public static String registerUser(String dbPath, String username, String password) {
        try {
            if (username == null || username.trim().isEmpty()) {
                return "ERROR: Username cannot be empty";
            }
            if (password == null || password.isEmpty()) {
                return "ERROR: Password cannot be empty";
            }
            username = username.trim();

            Path path = Paths.get(dbPath);
            ensureDbExists(path);

            // Check uniqueness
            try (BufferedReader reader = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
                String line;
                while ((line = reader.readLine()) != null) {
                    String[] parts = line.split(",", -1);
                    if (parts.length >= 1 && parts[0].equals(username)) {
                        return "ERROR: Username already exists";
                    }
                }
            }

            // Generate salt and hash
            byte[] salt = new byte[16];
            SecureRandom sr = new SecureRandom();
            sr.nextBytes(salt);
            String saltB64 = Base64.getEncoder().encodeToString(salt);
            String hashHex = sha256Hex(concat(salt, password.getBytes(StandardCharsets.UTF_8)));

            // Append to DB
            String record = username + "," + saltB64 + "," + hashHex + System.lineSeparator();
            try (BufferedWriter writer = Files.newBufferedWriter(path, StandardCharsets.UTF_8, StandardOpenOption.APPEND)) {
                writer.write(record);
            }

            return "OK: User registered";
        } catch (Exception e) {
            return "ERROR: " + e.getMessage();
        }
    }

    private static void ensureDbExists(Path path) throws IOException {
        File f = path.toFile();
        if (!f.exists()) {
            f.getParentFile(); // ensure parent not needed
            Files.createDirectories(path.toAbsolutePath().getParent() == null ? Paths.get(".") : path.toAbsolutePath().getParent());
            Files.createFile(path);
        }
    }

    private static byte[] concat(byte[] a, byte[] b) {
        byte[] out = new byte[a.length + b.length];
        System.arraycopy(a, 0, out, 0, a.length);
        System.arraycopy(b, 0, out, a.length, b.length);
        return out;
    }

    private static String sha256Hex(byte[] data) throws Exception {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        byte[] digest = md.digest(data);
        StringBuilder sb = new StringBuilder(digest.length * 2);
        for (byte d : digest) {
            sb.append(String.format("%02x", d));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String db = "users_java.csv";
        System.out.println(registerUser(db, "alice", "password123"));
        System.out.println(registerUser(db, "bob", "Secr3t!"));
        System.out.println(registerUser(db, "charlie", "hunter2"));
        System.out.println(registerUser(db, "alice", "anotherpass")); // duplicate
        System.out.println(registerUser(db, "dave", "")); // empty password
    }
}