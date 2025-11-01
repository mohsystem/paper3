import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.channels.Channels;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.sql.*;
import java.util.Arrays;
import java.util.Locale;

public class Lse67 {
    // Max image size: 10 MiB
    private static final long MAX_IMAGE_BYTES = 10L * 1024L * 1024L;

    public static boolean storeImageToMySQL(
            String host,
            int port,
            String database,
            String user,
            String password,
            String tableName,
            String baseDir,
            String relativePath,
            String nameForDB
    ) {
        if (!isSafeTableName(tableName)) {
            System.err.println("Error: invalid table name.");
            return false;
        }
        if (host == null || host.isEmpty() || database == null || database.isEmpty() ||
                user == null || user.isEmpty() || password == null ||
                baseDir == null || baseDir.isEmpty() || relativePath == null || relativePath.isEmpty() ||
                nameForDB == null || nameForDB.isEmpty()) {
            System.err.println("Error: invalid parameters.");
            return false;
        }

        byte[] data;
        String mime;
        try {
            Path base = Paths.get(baseDir).toRealPath(LinkOption.NOFOLLOW_LINKS);
            if (!Files.isDirectory(base)) {
                System.err.println("Error: base directory invalid.");
                return false;
            }
            // Normalize relative path (must remain relative)
            if (Paths.get(relativePath).isAbsolute()) {
                System.err.println("Error: absolute paths are not allowed.");
                return false;
            }
            Path candidate = base.resolve(relativePath).normalize();
            Path realCandidate;
            try {
                realCandidate = candidate.toRealPath(LinkOption.NOFOLLOW_LINKS);
            } catch (IOException e) {
                System.err.println("Error: file not found or not accessible.");
                return false;
            }
            if (!realCandidate.startsWith(base)) {
                System.err.println("Error: path escapes base directory.");
                return false;
            }
            if (Files.isDirectory(realCandidate, LinkOption.NOFOLLOW_LINKS)) {
                System.err.println("Error: path is a directory.");
                return false;
            }
            long size = Files.size(realCandidate);
            if (size <= 0 || size > MAX_IMAGE_BYTES) {
                System.err.println("Error: invalid file size.");
                return false;
            }

            // Read file via channel
            try (SeekableByteChannel ch = Files.newByteChannel(realCandidate, StandardOpenOption.READ)) {
                ByteArrayOutputStream bos = new ByteArrayOutputStream((int)Math.min(size, Integer.MAX_VALUE));
                try (InputStream in = Channels.newInputStream(ch)) {
                    byte[] buf = new byte[8192];
                    long total = 0;
                    int r;
                    while ((r = in.read(buf)) != -1) {
                        total += r;
                        if (total > MAX_IMAGE_BYTES) {
                            System.err.println("Error: file too large.");
                            return false;
                        }
                        bos.write(buf, 0, r);
                    }
                }
                data = bos.toByteArray();
            }

            // Validate MIME and extension
            mime = detectMime(data);
            if (mime == null) {
                System.err.println("Error: unsupported or unknown image format.");
                return false;
            }
            if (!isAllowedExtension(relativePath, mime)) {
                System.err.println("Error: file extension does not match allowed types.");
                return false;
            }
        } catch (IOException e) {
            System.err.println("Error: file I/O failure.");
            return false;
        }

        // JDBC connection string enforcing TLS and hostname verification
        String url = String.format(
                Locale.ROOT,
                "jdbc:mysql://%s:%d/%s?sslMode=VERIFY_IDENTITY&serverTimezone=UTC&useUnicode=true&characterEncoding=UTF-8",
                host, port, database
        );

        String createSql = String.format(
                Locale.ROOT,
                "CREATE TABLE IF NOT EXISTS `%s` (" +
                        "id BIGINT AUTO_INCREMENT PRIMARY KEY," +
                        "name VARCHAR(255) NOT NULL," +
                        "mime VARCHAR(64) NOT NULL," +
                        "data LONGBLOB NOT NULL," +
                        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP" +
                        ") CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci",
                tableName
        );
        String insertSql = String.format(
                Locale.ROOT,
                "INSERT INTO `%s` (name, mime, data) VALUES (?, ?, ?)",
                tableName
        );

        try (Connection conn = DriverManager.getConnection(url, user, password)) {
            conn.setAutoCommit(false);
            try (Statement st = conn.createStatement()) {
                st.execute(createSql);
            }
            try (PreparedStatement ps = conn.prepareStatement(insertSql)) {
                ps.setString(1, nameForDB);
                ps.setString(2, mime);
                ps.setBinaryStream(3, new java.io.ByteArrayInputStream(data), data.length);
                ps.executeUpdate();
            }
            conn.commit();
            return true;
        } catch (SQLException sqle) {
            System.err.println("Error: database operation failed.");
            return false;
        }
    }

    private static boolean isSafeTableName(String table) {
        if (table == null || table.isEmpty() || table.length() > 64) return false;
        for (int i = 0; i < table.length(); i++) {
            char c = table.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_')) return false;
        }
        return true;
    }

    private static boolean isAllowedExtension(String filename, String mime) {
        String fn = filename.toLowerCase(Locale.ROOT);
        if (mime.equals("image/png")) return fn.endsWith(".png");
        if (mime.equals("image/jpeg")) return fn.endsWith(".jpg") || fn.endsWith(".jpeg");
        if (mime.equals("image/gif")) return fn.endsWith(".gif");
        if (mime.equals("image/bmp")) return fn.endsWith(".bmp");
        if (mime.equals("image/webp")) return fn.endsWith(".webp");
        return false;
    }

    private static String detectMime(byte[] data) {
        if (data.length >= 8) {
            byte[] png = new byte[]{(byte)0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
            if (startsWith(data, png)) return "image/png";
        }
        if (data.length >= 3) {
            if ((data[0] & 0xFF) == 0xFF && (data[1] & 0xFF) == 0xD8 && (data[2] & 0xFF) == 0xFF) {
                return "image/jpeg";
            }
        }
        if (data.length >= 6) {
            if ((data[0] == 'G') && (data[1] == 'I') && (data[2] == 'F') && (data[3] == '8') && (data[5] == 'a')) {
                return "image/gif";
            }
        }
        if (data.length >= 2) {
            if (data[0] == 'B' && data[1] == 'M') return "image/bmp";
        }
        if (data.length >= 12) {
            if (data[0]=='R' && data[1]=='I' && data[2]=='F' && data[3]=='F' &&
                data[8]=='W' && data[9]=='E' && data[10]=='B' && data[11]=='P') {
                return "image/webp";
            }
        }
        return null;
    }

    private static boolean startsWith(byte[] data, byte[] prefix) {
        if (data.length < prefix.length) return false;
        for (int i = 0; i < prefix.length; i++) {
            if (data[i] != prefix[i]) return false;
        }
        return true;
    }

    private static byte[] minimalPNG() {
        // 1x1 transparent PNG
        return new byte[]{
                (byte)0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
                0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
                0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,
                0x08,0x06,0x00,0x00,0x00,0x1F,0x15,(byte)0xC4,(byte)0x89,
                0x00,0x00,0x00,0x0A,0x49,0x44,0x41,0x54,
                0x78,(byte)0x9C,0x63,0x00,0x01,0x00,0x00,0x05,0x00,0x01,
                0x0D,0x0A,0x2D,(byte)0xB4,0x00,0x00,0x00,0x00,0x49,0x45,0x4E,0x44,
                (byte)0xAE,0x42,0x60,(byte)0x82
        };
    }

    private static byte[] minimalJPEG() {
        // Not a valid image but sufficient for magic check (JPEG SOI + APP0 + EOI)
        return new byte[]{
                (byte)0xFF,(byte)0xD8,(byte)0xFF,(byte)0xE0,0x00,0x10,0x4A,0x46,0x49,0x46,0x00,0x01,0x01,0x00,0x00,0x01,0x00,0x01,0x00,0x00,
                (byte)0xFF,(byte)0xD9
        };
    }

    private static void writeFile(Path p, byte[] data) throws IOException {
        Files.createDirectories(p.getParent());
        Files.write(p, data, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
    }

    public static void main(String[] args) throws Exception {
        // DB parameters from environment to avoid hardcoding secrets
        String host = System.getenv().getOrDefault("DB_HOST", "localhost");
        int port = Integer.parseInt(System.getenv().getOrDefault("DB_PORT", "3306"));
        String db = System.getenv().getOrDefault("DB_NAME", "test");
        String user = System.getenv().getOrDefault("DB_USER", "testuser");
        String pass = System.getenv().getOrDefault("DB_PASS", "testpass");
        String table = "images_secure";

        Path base = Files.createTempDirectory("safe_images");
        // Test files
        Path png = base.resolve("pic1.png");
        Path jpg = base.resolve("nested/dir/pic2.jpg");
        Path badExt = base.resolve("note.txt");
        Path traversal = base.resolve("ok/../../evil.jpg").normalize(); // won't be created inside base
        Path big = base.resolve("big.webp");

        // Create small valid "image" files for testing
        writeFile(png, minimalPNG());
        writeFile(jpg, minimalJPEG());
        writeFile(badExt, "hello".getBytes(StandardCharsets.UTF_8));
        // big file ~10 MiB + 1
        byte[] chunk = new byte[1024 * 1024];
        Arrays.fill(chunk, (byte) 0x41);
        try (java.io.OutputStream os = Files.newOutputStream(big, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING)) {
            for (int i = 0; i < 10; i++) os.write(chunk);
            os.write(new byte[1]); // exceed limit
        }

        // Five test cases
        System.out.println("Test1 (valid PNG): " + storeImageToMySQL(host, port, db, user, pass, table, base.toString(), base.relativize(png).toString(), "png_test"));
        System.out.println("Test2 (valid JPEG): " + storeImageToMySQL(host, port, db, user, pass, table, base.toString(), base.relativize(jpg).toString(), "jpeg_test"));
        System.out.println("Test3 (bad extension): " + storeImageToMySQL(host, port, db, user, pass, table, base.toString(), base.relativize(badExt).toString(), "badext_test"));
        // This will be rejected because normalized path escapes base
        System.out.println("Test4 (path traversal attempt): " + storeImageToMySQL(host, port, db, user, pass, table, base.toString(), "../etc/passwd", "traversal"));
        System.out.println("Test5 (too large): " + storeImageToMySQL(host, port, db, user, pass, table, base.toString(), base.relativize(big).toString(), "big_test"));
    }
}