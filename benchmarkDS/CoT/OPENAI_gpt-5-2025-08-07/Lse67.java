// Java implementation
// Class name must be Lse67
// Chain-of-Through process (brief in-code annotations only):
// 1) Problem understanding: store an image file into a MySQL database as BLOB.
// 2) Security requirements: validate identifiers, use parameterized queries, limit file size, safe resource handling.
// 3) Secure coding generation: implement using JDBC with try-with-resources, transactions, and input validation.
// 4) Code review: keep sensitive data out of code, avoid SQL injection, proper error handling.
// 5) Secure code output: final code adheres to above with defensive checks.

import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.nio.file.StandardCopyOption;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.UUID;
import java.io.IOException;
import java.sql.*;
import java.time.Duration;

public class Lse67 {

    private static final long MAX_FILE_SIZE_BYTES = 10L * 1024L * 1024L; // 10 MB

    public static boolean storeImageToMySQL(
            String jdbcUrl,
            String dbUser,
            String dbPassword,
            String tableName,
            String recordId,
            Path imagePath
    ) {
        if (!isSafeIdentifier(tableName)) {
            return false;
        }
        if (recordId == null || recordId.isEmpty()) {
            return false;
        }
        if (imagePath == null) {
            return false;
        }
        try {
            if (!Files.isRegularFile(imagePath)) return false;
            long size = Files.size(imagePath);
            if (size <= 0 || size > MAX_FILE_SIZE_BYTES) return false;
            byte[] data = Files.readAllBytes(imagePath);
            String filename = imagePath.getFileName().toString();
            String mimeType = detectMimeType(imagePath, filename);

            try (Connection conn = DriverManager.getConnection(jdbcUrl, dbUser, dbPassword)) {
                conn.setAutoCommit(false);
                try {
                    String createSql = "CREATE TABLE IF NOT EXISTS `" + tableName + "` (" +
                            "id VARCHAR(128) PRIMARY KEY," +
                            "filename VARCHAR(255) NOT NULL," +
                            "mime_type VARCHAR(100) NOT NULL," +
                            "size_bytes BIGINT NOT NULL," +
                            "data LONGBLOB NOT NULL" +
                            ")";
                    try (Statement st = conn.createStatement()) {
                        st.execute(createSql);
                    }

                    String insertSql = "REPLACE INTO `" + tableName + "` (id, filename, mime_type, size_bytes, data) VALUES (?,?,?,?,?)";
                    try (PreparedStatement ps = conn.prepareStatement(insertSql)) {
                        ps.setString(1, recordId);
                        ps.setString(2, filename);
                        ps.setString(3, mimeType);
                        ps.setLong(4, data.length);
                        ps.setBytes(5, data);
                        ps.executeUpdate();
                    }
                    conn.commit();
                    return true;
                } catch (SQLException e) {
                    try { conn.rollback(); } catch (SQLException ignored) {}
                    return false;
                } finally {
                    try { conn.setAutoCommit(true); } catch (SQLException ignored) {}
                }
            } catch (SQLException se) {
                return false;
            }
        } catch (IOException ioe) {
            return false;
        }
    }

    private static String detectMimeType(Path path, String filename) {
        try {
            String t = Files.probeContentType(path);
            if (t != null && !t.isBlank()) return t;
        } catch (IOException ignored) {}
        String lower = filename.toLowerCase(Locale.ROOT);
        if (lower.endsWith(".png")) return "image/png";
        if (lower.endsWith(".jpg") || lower.endsWith(".jpeg")) return "image/jpeg";
        if (lower.endsWith(".gif")) return "image/gif";
        if (lower.endsWith(".bmp")) return "image/bmp";
        if (lower.endsWith(".webp")) return "image/webp";
        return "application/octet-stream";
    }

    private static boolean isSafeIdentifier(String s) {
        if (s == null || s.length() == 0 || s.length() > 64) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c >= 'a' && c <= 'z')
                    || (c >= 'A' && c <= 'Z')
                    || (c >= '0' && c <= '9')
                    || (c == '_');
            if (!ok) return false;
        }
        return true;
    }

    // Helper to create a small sample image-like byte array
    private static byte[] tinyPng() {
        // Minimal PNG header + IEND chunk (not a valid image but good for DB testing)
        return new byte[] {
                (byte)0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
                0x00,0x00,0x00,0x00, 0x49,0x45,0x4E,0x44, (byte)0xAE,0x42,0x60,(byte)0x82
        };
    }

    private static Path createTempImage(String suffix) throws IOException {
        Path p = Files.createTempFile("img_" + UUID.randomUUID(), suffix);
        Files.write(p, tinyPng(), StandardOpenOption.TRUNCATE_EXISTING);
        p.toFile().deleteOnExit();
        return p;
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases (will attempt DB connection if reachable)
        String jdbcUrl = System.getenv().getOrDefault("DB_URL", "jdbc:mysql://localhost:3306/test?useSSL=true&requireSSL=false&serverTimezone=UTC");
        String user = System.getenv().getOrDefault("DB_USER", "root");
        String pass = System.getenv().getOrDefault("DB_PASS", "password");
        String table = System.getenv().getOrDefault("DB_TABLE", "images_secure");

        Path p1 = createTempImage(".png");
        Path p2 = createTempImage(".jpg");
        Path p3 = createTempImage(".gif");
        Path p4 = createTempImage(".webp");
        Path p5 = createTempImage(".bmp");

        storeImageToMySQL(jdbcUrl, user, pass, table, "img1", p1);
        storeImageToMySQL(jdbcUrl, user, pass, table, "img2", p2);
        storeImageToMySQL(jdbcUrl, user, pass, table, "img3", p3);
        storeImageToMySQL(jdbcUrl, user, pass, table, "img4", p4);
        storeImageToMySQL(jdbcUrl, user, pass, table, "img5", p5);
    }
}