import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse140 {

    // Validate username: 1..64 chars, allowed A-Z a-z 0-9 _ . -
    private static boolean isValidUsername(String s) {
        if (s == null) return false;
        int len = s.length();
        if (len < 1 || len > 64) return false;
        for (int i = 0; i < len; i++) {
            char c = s.charAt(i);
            if (Character.isLetterOrDigit(c)) continue;
            if (c == '_' || c == '.' || c == '-') continue;
            return false;
        }
        return true;
    }

    // Basic path validation: not null, length limit, and no ".." traversal
    private static boolean isSafePath(String p) {
        if (p == null) return false;
        if (p.length() < 1 || p.length() > 4096) return false;
        if (p.contains("..")) return false;
        return true;
    }

    // Connect to the "database" (a secure, line-delimited file) and check existence
    public static boolean usernameExists(String dbPath, String username) {
        if (!isValidUsername(username)) return false;
        if (!isSafePath(dbPath)) return false;

        Path path = Paths.get(dbPath);
        try {
            if (!Files.exists(path)) return false;
            if (Files.isDirectory(path)) return false;
            if (Files.isSymbolicLink(path)) return false;

            try (BufferedReader br = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
                String line;
                while ((line = br.readLine()) != null) {
                    if (line.length() > 256) {
                        // Skip unusually long records
                        continue;
                    }
                    // Trim trailing CR if present
                    if (!line.isEmpty() && line.charAt(line.length() - 1) == '\r') {
                        line = line.substring(0, line.length() - 1);
                    }
                    if (isValidUsername(line) && line.equals(username)) {
                        return true;
                    }
                }
            }
        } catch (IOException e) {
            // Fail closed
            return false;
        }
        return false;
    }

    // Helper: create a secure temporary "database" file with sample users
    private static String createSampleDb() throws IOException {
        Path tmp = Files.createTempFile("users_", ".db");
        // Rely on platform defaults; ensure owner-only readable/writable
        try (BufferedWriter bw = Files.newBufferedWriter(tmp, StandardCharsets.UTF_8)) {
            bw.write("alice\n");
            bw.write("Bob_123\n");
            bw.write("charlie\n");
            bw.write("david.s\n");
            bw.write("eve-9\n");
        }
        tmp.toFile().deleteOnExit();
        return tmp.toString();
    }

    public static void main(String[] args) {
        try {
            String dbPath = createSampleDb();

            String[] tests = new String[] {
                "alice",     // true
                "bob_123",   // false (case-sensitive)
                "charlie",   // true
                "mallory",   // false
                "eve-9"      // true
            };

            for (String t : tests) {
                boolean result = usernameExists(dbPath, t);
                System.out.println(t + " -> " + result);
            }
        } catch (IOException e) {
            System.err.println("Setup error.");
        }
    }
}