import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.sql.*;
import java.util.*;

public class Lse64 {

    public static class DBConfig {
        public final String url;
        public final String user;
        public final String password;

        public DBConfig(String url, String user, String password) {
            this.url = url;
            this.user = user;
            this.password = password;
        }

        public Connection getConnection() throws SQLException {
            return DriverManager.getConnection(url, user, password);
        }
    }

    private static final long MAX_SIZE = 10L * 1024L * 1024L; // 10 MB
    private static final String UPLOADS_DIR = "uploads";

    private static String sanitizeFilename(String original) {
        if (original == null || original.isEmpty()) {
            original = "file.pdf";
        }
        String name = original.replace('\\', '/');
        int idx = name.lastIndexOf('/');
        if (idx >= 0) name = name.substring(idx + 1);
        name = name.replaceAll("[^A-Za-z0-9._-]", "_");
        if (!name.toLowerCase(Locale.ROOT).endsWith(".pdf")) {
            name = name + ".pdf";
        }
        if (name.length() > 100) {
            name = name.substring(name.length() - 100);
        }
        return name;
    }

    private static boolean isValidPdf(byte[] data) {
        if (data == null) return false;
        if (data.length == 0 || data.length > MAX_SIZE) return false;
        String head = new String(Arrays.copyOfRange(data, 0, Math.min(8, data.length)), StandardCharsets.ISO_8859_1);
        if (!head.startsWith("%PDF-")) return false;
        String content = new String(data, StandardCharsets.ISO_8859_1);
        if (!content.contains("%%EOF")) return false;
        return true;
    }

    private static Path ensureUploadsDir() throws IOException {
        Path dir = Paths.get(UPLOADS_DIR);
        if (!Files.exists(dir)) {
            Files.createDirectories(dir);
            try {
                Set<PosixFilePermission> perms = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE,
                        PosixFilePermission.OWNER_EXECUTE
                );
                Files.setPosixFilePermissions(dir, perms);
            } catch (UnsupportedOperationException ignored) {
                File f = dir.toFile();
                f.setReadable(true, true);
                f.setWritable(true, true);
                f.setExecutable(true, true);
            }
        }
        return dir.toRealPath();
    }

    private static void secureSetFilePermissions(Path p) {
        try {
            Set<PosixFilePermission> perms = EnumSet.of(
                    PosixFilePermission.OWNER_READ,
                    PosixFilePermission.OWNER_WRITE
            );
            Files.setPosixFilePermissions(p, perms);
        } catch (UnsupportedOperationException | IOException ignored) {
            File f = p.toFile();
            f.setReadable(true, true);
            f.setWritable(true, true);
            f.setExecutable(false, false);
        }
    }

    private static void saveFilepathToDB(DBConfig cfg, String path) throws SQLException {
        if (cfg == null) return;
        String createTable = "CREATE TABLE IF NOT EXISTS documents (" +
                "id INT AUTO_INCREMENT PRIMARY KEY," +
                "path VARCHAR(1024) NOT NULL," +
                "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP" +
                ")";
        String insert = "INSERT INTO documents (path) VALUES (?)";
        try (Connection conn = cfg.getConnection()) {
            try (Statement st = conn.createStatement()) {
                st.execute(createTable);
            }
            try (PreparedStatement ps = conn.prepareStatement(insert)) {
                ps.setString(1, path);
                ps.executeUpdate();
            }
        }
    }

    public static String upload_pdf(byte[] fileBytes, String originalFilename, DBConfig cfg) throws IOException, SQLException {
        if (!isValidPdf(fileBytes)) {
            throw new SecurityException("Invalid PDF file.");
        }
        Path uploadsRoot = ensureUploadsDir();
        String safe = sanitizeFilename(originalFilename);
        String base = safe;
        if (safe.toLowerCase(Locale.ROOT).endsWith(".pdf")) {
            base = safe.substring(0, safe.length() - 4);
        }
        String finalName;
        Path target;
        int attempts = 0;
        do {
            String unique = UUID.randomUUID().toString().replace("-", "");
            finalName = base + "_" + unique + ".pdf";
            target = uploadsRoot.resolve(finalName);
            attempts++;
            if (attempts > 10) throw new IOException("Failed to allocate unique filename.");
        } while (Files.exists(target));
        byte[] bytes = Arrays.copyOf(fileBytes, fileBytes.length);
        Files.write(target, bytes, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);
        secureSetFilePermissions(target);
        Path real = target.toRealPath();
        if (!real.startsWith(uploadsRoot)) {
            Files.deleteIfExists(real);
            throw new SecurityException("Path traversal detected.");
        }
        String storedPath = uploadsRoot.relativize(real).toString().replace(File.separatorChar, '/');
        saveFilepathToDB(cfg, storedPath);
        return storedPath;
    }

    public static byte[] download_pdf(String storedPath) throws IOException {
        Path uploadsRoot = ensureUploadsDir();
        Path target = uploadsRoot.resolve(storedPath).normalize();
        Path real = target.toRealPath();
        if (!real.startsWith(uploadsRoot)) {
            throw new SecurityException("Invalid file path.");
        }
        return Files.readAllBytes(real);
    }

    private static DBConfig cfgFromEnv() {
        String url = System.getenv("DB_URL"); // e.g., jdbc:mysql://localhost:3306/testdb?useSSL=true&allowPublicKeyRetrieval=true
        String user = System.getenv("DB_USER");
        String pass = System.getenv("DB_PASS");
        if (url == null || url.isEmpty() || user == null || pass == null) {
            return null;
        }
        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
        } catch (ClassNotFoundException ignored) {}
        return new DBConfig(url, user, pass);
    }

    public static void main(String[] args) {
        DBConfig cfg = cfgFromEnv();
        List<String> filenames = Arrays.asList(
                "report.pdf",
                "invoice..pdf",
                "../../evil.pdf",
                "résumé.pdf",
                "noext"
        );
        List<byte[]> pdfs = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            String content = "%PDF-1.4\n% test " + i + "\n1 0 obj\n<<>>\nendobj\n%%EOF\n";
            pdfs.add(content.getBytes(StandardCharsets.ISO_8859_1));
        }

        List<String> storedPaths = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            try {
                String path = upload_pdf(pdfs.get(i), filenames.get(i), cfg);
                System.out.println("Uploaded: " + path);
                storedPaths.add(path);
            } catch (Exception e) {
                System.out.println("Upload failed for test " + i + ": " + e.getMessage());
                storedPaths.add(null);
            }
        }

        for (int i = 0; i < 5; i++) {
            try {
                if (storedPaths.get(i) == null) {
                    System.out.println("Skipping download for test " + i);
                    continue;
                }
                byte[] got = download_pdf(storedPaths.get(i));
                boolean ok = Arrays.equals(got, pdfs.get(i));
                System.out.println("Download " + i + " ok=" + ok);
            } catch (Exception e) {
                System.out.println("Download failed for test " + i + ": " + e.getMessage());
            }
        }
    }
}