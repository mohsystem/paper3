import java.io.IOException;
import java.io.InputStream;
import java.nio.file.*;
import java.security.MessageDigest;
import java.sql.*;
import java.util.Arrays;
import java.util.HexFormat;

public class Lse67 {
    private static final long MAX_FILE_SIZE = 20L * 1024L * 1024L; // 20 MB
    private static final String TABLE_NAME = "images";

    public static long storeImageToDb(String host, int port, String database, String user, String password, String imagePath) throws Exception {
        Path path = Paths.get(imagePath).normalize().toAbsolutePath();
        if (!Files.exists(path) || !Files.isReadable(path) || !Files.isRegularFile(path)) {
            throw new IllegalArgumentException("File does not exist or is not readable: " + path);
        }
        long size = Files.size(path);
        if (size <= 0 || size > MAX_FILE_SIZE) {
            throw new IllegalArgumentException("Invalid file size. Must be > 0 and <= " + MAX_FILE_SIZE + " bytes");
        }

        byte[] head = readHead(path, 64);
        String mime = detectImageMime(head, path.getFileName().toString());
        if (mime == null) {
            throw new IllegalArgumentException("Unsupported or unrecognized image format");
        }

        byte[] data = Files.readAllBytes(path);
        String sha256 = sha256Hex(data);

        String url = String.format(
                "jdbc:mysql://%s:%d/%s?useSSL=true&requireSSL=false&allowPublicKeyRetrieval=true&serverTimezone=UTC&useUnicode=true&characterEncoding=UTF-8",
                host, port, database
        );

        DriverManager.setLoginTimeout(10);
        try (Connection conn = DriverManager.getConnection(url, user, password)) {
            conn.setAutoCommit(false);
            try {
                ensureTable(conn);
                long id = insertImage(conn, path.getFileName().toString(), mime, data, sha256);
                conn.commit();
                return id;
            } catch (Exception ex) {
                conn.rollback();
                throw ex;
            }
        }
    }

    private static long insertImage(Connection conn, String name, String mime, byte[] data, String sha256) throws SQLException {
        String sql = "INSERT INTO " + TABLE_NAME + " (name, mime_type, data, sha256) VALUES (?, ?, ?, ?)";
        try (PreparedStatement ps = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS)) {
            ps.setString(1, name);
            ps.setString(2, mime);
            ps.setBinaryStream(3, new java.io.ByteArrayInputStream(data), data.length);
            ps.setString(4, sha256);
            int affected = ps.executeUpdate();
            if (affected != 1) {
                throw new SQLException("Insert failed, affected rows: " + affected);
            }
            try (ResultSet rs = ps.getGeneratedKeys()) {
                if (rs.next()) {
                    return rs.getLong(1);
                }
            }
            throw new SQLException("Failed to retrieve generated id");
        }
    }

    private static void ensureTable(Connection conn) throws SQLException {
        String ddl = "CREATE TABLE IF NOT EXISTS " + TABLE_NAME + " (" +
                "id BIGINT NOT NULL AUTO_INCREMENT PRIMARY KEY," +
                "name VARCHAR(255) NOT NULL," +
                "mime_type VARCHAR(100) NOT NULL," +
                "data LONGBLOB NOT NULL," +
                "sha256 CHAR(64) NOT NULL," +
                "created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP," +
                "UNIQUE KEY uniq_sha256 (sha256)" +
                ") CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci";
        try (Statement st = conn.createStatement()) {
            st.execute(ddl);
        }
    }

    private static byte[] readHead(Path path, int n) throws IOException {
        try (InputStream is = Files.newInputStream(path)) {
            byte[] buf = new byte[n];
            int read = is.read(buf);
            if (read <= 0) return new byte[0];
            return Arrays.copyOf(buf, read);
        }
    }

    private static String detectImageMime(byte[] b, String fileName) {
        if (b.length >= 8 &&
                (b[0] & 0xFF) == 0x89 && b[1] == 0x50 && b[2] == 0x4E && b[3] == 0x47 &&
                b[4] == 0x0D && b[5] == 0x0A && b[6] == 0x1A && b[7] == 0x0A) {
            return "image/png";
        }
        if (b.length >= 3 && (b[0] & 0xFF) == 0xFF && (b[1] & 0xFF) == 0xD8 && (b[2] & 0xFF) == 0xFF) {
            return "image/jpeg";
        }
        if (b.length >= 6 &&
                b[0] == 'G' && b[1] == 'I' && b[2] == 'F' &&
                b[3] == '8' && (b[4] == '7' || b[4] == '9') && b[5] == 'a') {
            return "image/gif";
        }
        if (b.length >= 12 &&
                b[0] == 'R' && b[1] == 'I' && b[2] == 'F' && b[3] == 'F' &&
                b[8] == 'W' && b[9] == 'E' && b[10] == 'B' && b[11] == 'P') {
            return "image/webp";
        }
        if (b.length >= 2 && b[0] == 'B' && b[1] == 'M') {
            return "image/bmp";
        }
        if (b.length >= 4 &&
                ((b[0] == 'I' && b[1] == 'I' && b[2] == 0x2A && b[3] == 0x00) ||
                 (b[0] == 'M' && b[1] == 'M' && b[2] == 0x00 && b[3] == 0x2A))) {
            return "image/tiff";
        }
        try {
            String probed = Files.probeContentType(Paths.get(fileName));
            if (probed != null && probed.startsWith("image/")) {
                return probed;
            }
        } catch (Exception ignored) {}
        return null;
    }

    private static String sha256Hex(byte[] data) throws Exception {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        byte[] digest = md.digest(data);
        return HexFormat.of().formatHex(digest);
    }

    public static void main(String[] args) {
        String host = System.getenv().getOrDefault("DB_HOST", "127.0.0.1");
        int port = Integer.parseInt(System.getenv().getOrDefault("DB_PORT", "3306"));
        String database = System.getenv().getOrDefault("DB_NAME", "test");
        String user = System.getenv().getOrDefault("DB_USER", "root");
        String pass = System.getenv().getOrDefault("DB_PASS", "");

        String[] tests = new String[] {
                "sample1.jpg",
                "sample2.png",
                "not_found_image.bin",
                "test.txt",
                "small.gif"
        };

        for (int i = 0; i < tests.length; i++) {
            try {
                long id = storeImageToDb(host, port, database, user, pass, tests[i]);
                System.out.println("Test " + (i+1) + ": Inserted ID = " + id + " for file " + tests[i]);
            } catch (Exception e) {
                System.out.println("Test " + (i+1) + ": Failed for file " + tests[i] + " -> " + e.getMessage());
            }
        }
    }
}