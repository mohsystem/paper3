import java.io.IOException;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.security.SecureRandom;
import java.sql.*;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;

public class Lse64 {

    // ==============================
    // Chain-of-Through process:
    // 1) Problem understanding
    // Purpose: Save an uploaded PDF into "uploads" directory, store its filepath in MySQL, and use the saved filepath to download it.
    // Inputs: file bytes, original filename, optional DB credentials.
    // Outputs: stored filepath string (upload), bytes when downloading.
    //
    // 2) Security requirements
    // - Validate file type (PDF signature) and size
    // - Sanitize filename (prevent traversal, restricted charset)
    // - Enforce storage within controlled "uploads" directory
    // - Use parameterized SQL (prepared statements)
    // - Minimal file permissions
    // - Robust error handling without leaking sensitive info
    //
    // 3) Secure coding generation integrated below
    // 4) Code review mentally iterated within comments
    // 5) Secure code output finalized
    // ==============================

    private static final Path UPLOAD_DIR = Paths.get("uploads").toAbsolutePath().normalize();
    private static final long MAX_BYTES = 10L * 1024L * 1024L; // 10 MiB max

    // In-memory fallback "DB" when MySQL is unavailable (for tests/demo)
    private static final Map<Long, String> MEMORY_DB = new ConcurrentHashMap<>();
    private static final AtomicLong MEMORY_ID = new AtomicLong(1);

    private static final SecureRandom RNG = new SecureRandom();

    // Ensures the uploads directory exists and is secure
    private static void ensureUploadsDir() throws IOException {
        Files.createDirectories(UPLOAD_DIR);
        // Try to set restrictive perms on POSIX systems
        try {
            Set<PosixFilePermission> perms = EnumSet.of(
                    PosixFilePermission.OWNER_READ,
                    PosixFilePermission.OWNER_WRITE,
                    PosixFilePermission.OWNER_EXECUTE
            );
            Files.setPosixFilePermissions(UPLOAD_DIR, perms);
        } catch (UnsupportedOperationException ignored) {
            // Non-POSIX (Windows) - ignore
        }
    }

    // Sanitize filename to safe characters only and ensure .pdf extension
    private static String sanitizeFilename(String original) {
        if (original == null) original = "file.pdf";
        String name = original.replace("\\", "/");
        int slash = name.lastIndexOf('/');
        if (slash >= 0) name = name.substring(slash + 1);

        // Remove dangerous characters
        name = name.replaceAll("[^A-Za-z0-9._-]", "_");

        // Enforce extension .pdf
        if (!name.toLowerCase(Locale.ROOT).endsWith(".pdf")) {
            name = name + ".pdf";
        }

        // Prevent reserved names and empties
        if (name.equals(".pdf") || name.equals("_") || name.length() == 0) {
            name = "file.pdf";
        }
        return name;
    }

    // Generate a unique safe filename
    private static String uniqueFilename(String baseSafeName) {
        String ts = String.valueOf(Instant.now().toEpochMilli());
        byte[] rand = new byte[6];
        RNG.nextBytes(rand);
        String suffix = Base64.getUrlEncoder().withoutPadding().encodeToString(rand);
        String ext = ".pdf";
        String stem = baseSafeName;
        int dot = baseSafeName.lastIndexOf('.');
        if (dot > 0) {
            stem = baseSafeName.substring(0, dot);
            ext = baseSafeName.substring(dot);
        }
        // truncate stem to avoid filesystem limits
        if (stem.length() > 64) stem = stem.substring(0, 64);
        return stem + "_" + ts + "_" + suffix + ext;
    }

    // Basic PDF validation: header and size limits
    private static boolean isPdf(byte[] data) {
        if (data == null || data.length < 5) return false;
        if (data.length > MAX_BYTES) return false;
        String header = new String(Arrays.copyOfRange(data, 0, Math.min(8, data.length)), StandardCharsets.US_ASCII);
        return header.startsWith("%PDF-");
    }

    private static Path safeResolveInUploadDir(String filename) throws IOException {
        Path p = UPLOAD_DIR.resolve(filename).normalize().toAbsolutePath();
        if (!p.startsWith(UPLOAD_DIR)) {
            throw new SecurityException("Invalid path (path traversal detected)");
        }
        return p;
    }

    private static void writeSecureFile(Path target, byte[] data) throws IOException {
        // create file with restrictive perms
        try {
            Set<PosixFilePermission> perms = EnumSet.of(
                    PosixFilePermission.OWNER_READ,
                    PosixFilePermission.OWNER_WRITE
            );
            Files.createFile(target);
            try {
                Files.setPosixFilePermissions(target, perms);
            } catch (UnsupportedOperationException ignored) {}
        } catch (FileAlreadyExistsException e) {
            throw new IOException("File already exists unexpectedly");
        }
        Files.write(target, data, StandardOpenOption.TRUNCATE_EXISTING);
    }

    private static Connection getMySqlConnection(String url, String user, String pass) throws SQLException, ClassNotFoundException {
        if (url == null || url.isBlank()) return null;
        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
        } catch (ClassNotFoundException e) {
            // allow propagation; caller will fallback to memory DB
            throw e;
        }
        Properties props = new Properties();
        if (user != null) props.setProperty("user", user);
        if (pass != null) props.setProperty("password", pass);
        props.setProperty("useSSL", "true");
        props.setProperty("requireSSL", "false");
        props.setProperty("allowPublicKeyRetrieval", "true");
        return DriverManager.getConnection(url, props);
    }

    private static void ensureTable(Connection c) throws SQLException {
        try (Statement s = c.createStatement()) {
            s.execute("CREATE TABLE IF NOT EXISTS uploads (" +
                    "id BIGINT PRIMARY KEY AUTO_INCREMENT," +
                    "filepath VARCHAR(1024) NOT NULL" +
                    ")");
        }
    }

    private static long insertFilepath(Connection c, String filepath) throws SQLException {
        ensureTable(c);
        try (PreparedStatement ps = c.prepareStatement("INSERT INTO uploads(filepath) VALUES(?)", Statement.RETURN_GENERATED_KEYS)) {
            ps.setString(1, filepath);
            ps.executeUpdate();
            try (ResultSet rs = ps.getGeneratedKeys()) {
                if (rs.next()) return rs.getLong(1);
            }
        }
        // Fallback when no generated keys available
        return -1L;
    }

    // Public API: upload_pdf
    // Saves PDF to uploads folder, stores filepath in MySQL (if configured), and returns filepath.
    public static String upload_pdf(byte[] pdfBytes, String originalFilename, String jdbcUrl, String dbUser, String dbPass) throws Exception {
        if (!isPdf(pdfBytes)) throw new IllegalArgumentException("Invalid PDF or size exceeds limit");
        ensureUploadsDir();

        String safe = sanitizeFilename(originalFilename);
        String unique = uniqueFilename(safe);
        Path target = safeResolveInUploadDir(unique);

        writeSecureFile(target, pdfBytes);

        String storedPath = target.toString();

        // Try MySQL, fallback to in-memory
        try {
            Connection c = getMySqlConnection(jdbcUrl, dbUser, dbPass);
            if (c != null) {
                try (c) {
                    long id = insertFilepath(c, storedPath);
                    // We also store in memory to support download tests w/o DB lookup
                    if (id > 0) {
                        MEMORY_DB.put(id, storedPath);
                    }
                }
            } else {
                long id = MEMORY_ID.getAndIncrement();
                MEMORY_DB.put(id, storedPath);
            }
        } catch (Throwable t) {
            long id = MEMORY_ID.getAndIncrement();
            MEMORY_DB.put(id, storedPath);
        }
        return storedPath;
    }

    // Public API: download_pdf
    // Uses the provided filepath (as retrieved from DB) and returns the bytes if safe and exists.
    public static byte[] download_pdf(String filepath) throws Exception {
        if (filepath == null || filepath.isBlank()) throw new IllegalArgumentException("Invalid filepath");
        Path p = Paths.get(filepath).toAbsolutePath().normalize();
        if (!p.startsWith(UPLOAD_DIR)) throw new SecurityException("Access denied");
        if (!Files.isRegularFile(p) || !Files.isReadable(p)) throw new IOException("File not found or not readable");
        // Basic guard: read bounded to MAX_BYTES + small margin
        long size = Files.size(p);
        if (size > MAX_BYTES) throw new IOException("File too large");
        return Files.readAllBytes(p);
    }

    // Helper for tests: create minimal valid PDF bytes
    private static byte[] samplePdf(String title) {
        String pdf = "%PDF-1.4\n" +
                "1 0 obj<< /Type /Catalog /Pages 2 0 R >>endobj\n" +
                "2 0 obj<< /Type /Pages /Kids [3 0 R] /Count 1 >>endobj\n" +
                "3 0 obj<< /Type /Page /Parent 2 0 R /MediaBox [0 0 200 200] /Contents 4 0 R >>endobj\n" +
                "4 0 obj<< /Length 44 >>stream\n" +
                "BT /F1 12 Tf 72 120 Td (" + title.replace(")", "\\)") + ") Tj ET\n" +
                "endstream endobj\n" +
                "xref\n0 5\n0000000000 65535 f \n" +
                "trailer<< /Root 1 0 R /Size 5 >>\nstartxref\n0\n%%EOF\n";
        return pdf.getBytes(StandardCharsets.US_ASCII);
    }

    public static void main(String[] args) throws Exception {
        PrintStream out = System.out;

        // 5 test cases demonstrating secure handling and usage
        // DB credentials left null to exercise in-memory fallback for demo
        String url = null, user = null, pass = null;

        List<String> paths = new ArrayList<>();

        // 1) Valid simple name
        byte[] pdf1 = samplePdf("Test1");
        String p1 = upload_pdf(pdf1, "report.pdf", url, user, pass);
        paths.add(p1);
        out.println("Uploaded: " + p1 + " size=" + download_pdf(p1).length);

        // 2) Attempted traversal
        byte[] pdf2 = samplePdf("Test2");
        String p2 = upload_pdf(pdf2, "../evil.pdf", url, user, pass);
        paths.add(p2);
        out.println("Uploaded: " + p2 + " size=" + download_pdf(p2).length);

        // 3) Uppercase extension and spaces
        byte[] pdf3 = samplePdf("Test3");
        String p3 = upload_pdf(pdf3, "My Doc.PDF", url, user, pass);
        paths.add(p3);
        out.println("Uploaded: " + p3 + " size=" + download_pdf(p3).length);

        // 4) Long name
        byte[] pdf4 = samplePdf("Test4");
        String p4 = upload_pdf(pdf4, "a_very_long_filename_that_needs_truncation_because_of_fs_limits_and_should_end_with_pdf.pdf", url, user, pass);
        paths.add(p4);
        out.println("Uploaded: " + p4 + " size=" + download_pdf(p4).length);

        // 5) Name with unsafe chars
        byte[] pdf5 = samplePdf("Test5");
        String p5 = upload_pdf(pdf5, "inv@lid name (1).pdf", url, user, pass);
        paths.add(p5);
        out.println("Uploaded: " + p5 + " size=" + download_pdf(p5).length);
    }
}