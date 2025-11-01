import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.SecureRandom;
import java.sql.*;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class Lse64 {

    private static final long MAX_UPLOAD_SIZE = 32L * 1024 * 1024; // 32 MiB
    private static final SecureRandom RNG = new SecureRandom();

    public static String upload_pdf(String baseDir, String originalFilename, byte[] content,
                                    String jdbcUrl, String dbUser, String dbPass) throws IOException {
        if (content == null) throw new IllegalArgumentException("content is null");
        if (content.length == 0) throw new IllegalArgumentException("empty content");
        if (content.length > MAX_UPLOAD_SIZE) throw new IllegalArgumentException("file too large");
        if (originalFilename == null) throw new IllegalArgumentException("filename is null");

        Path base = Paths.get(baseDir).normalize();
        if (!Files.exists(base)) Files.createDirectories(base);
        if (!Files.isDirectory(base, LinkOption.NOFOLLOW_LINKS)) throw new IOException("baseDir not a directory");
        if (Files.isSymbolicLink(base)) throw new IOException("baseDir must not be a symlink");

        Path uploads = base.resolve("uploads").normalize();
        if (!uploads.startsWith(base)) throw new IOException("path traversal blocked");
        if (!Files.exists(uploads)) Files.createDirectories(uploads);
        if (!Files.isDirectory(uploads, LinkOption.NOFOLLOW_LINKS)) throw new IOException("uploads not a directory");
        if (Files.isSymbolicLink(uploads)) throw new IOException("uploads must not be a symlink");

        String safeName = sanitizeFilename(originalFilename);
        if (!safeName.toLowerCase().endsWith(".pdf")) {
            safeName = safeName.replaceAll("\\.[^.]*$", "") + ".pdf";
        }
        String uniquePrefix = UUID.randomUUID().toString().replace("-", "").substring(0, 12);
        String finalName = uniquePrefix + "_" + safeName;
        Path finalPath = uploads.resolve(finalName).normalize();
        if (!finalPath.startsWith(uploads)) throw new IOException("path traversal blocked");

        // Write atomically: write to temp then move
        String tempName = finalName + ".part_" + Long.toUnsignedString(RNG.nextLong(), 36);
        Path tempPath = uploads.resolve(tempName).normalize();
        if (!tempPath.startsWith(uploads)) throw new IOException("temp path traversal blocked");

        try (FileChannel fc = FileChannel.open(tempPath,
                StandardOpenOption.CREATE_NEW,
                StandardOpenOption.WRITE)) {
            int offset = 0;
            int remaining = content.length;
            while (remaining > 0) {
                int chunk = Math.min(64 * 1024, remaining);
                fc.write(ByteBuffer.wrap(content, offset, chunk));
                offset += chunk;
                remaining -= chunk;
            }
            fc.force(true);
        } catch (FileAlreadyExistsException e) {
            throw new IOException("collision, retry upload");
        } catch (IOException e) {
            // Clean up temp on failure
            try { Files.deleteIfExists(tempPath); } catch (IOException ignored) {}
            throw e;
        }

        try {
            Files.move(tempPath, finalPath, StandardCopyOption.ATOMIC_MOVE);
        } catch (AtomicMoveNotSupportedException e) {
            Files.move(tempPath, finalPath, StandardCopyOption.REPLACE_EXISTING);
        }

        String relPath = "uploads/" + finalName;

        // Store filepath in MySQL (best-effort if driver available)
        storePathInMySQL(jdbcUrl, dbUser, dbPass, relPath);

        return relPath;
    }

    public static byte[] download_pdf(String baseDir, String storedRelativePath) throws IOException {
        if (storedRelativePath == null || storedRelativePath.isEmpty())
            throw new IllegalArgumentException("path is empty");
        if (storedRelativePath.startsWith("/") || storedRelativePath.contains(".."))
            throw new IllegalArgumentException("invalid stored path");

        Path base = Paths.get(baseDir).normalize();
        Path uploads = base.resolve("uploads").normalize();
        if (!Files.isDirectory(uploads, LinkOption.NOFOLLOW_LINKS))
            throw new IOException("uploads missing");
        if (Files.isSymbolicLink(uploads)) throw new IOException("uploads must not be a symlink");

        Path filePath = base.resolve(storedRelativePath).normalize();
        if (!filePath.startsWith(uploads)) throw new IOException("path traversal blocked");
        if (Files.isSymbolicLink(filePath)) throw new IOException("refuses symlink");
        if (!Files.exists(filePath)) throw new IOException("not found");

        long size = Files.size(filePath);
        if (size < 0 || size > MAX_UPLOAD_SIZE) throw new IOException("file size invalid");

        try (FileChannel fc = FileChannel.open(filePath, StandardOpenOption.READ)) {
            if (size == 0) return new byte[0];
            if (size > Integer.MAX_VALUE) throw new IOException("too large");
            byte[] out = new byte[(int) size];
            ByteBuffer buf = ByteBuffer.wrap(out);
            while (buf.hasRemaining()) {
                int r = fc.read(buf);
                if (r < 0) break;
            }
            return out;
        }
    }

    private static String sanitizeFilename(String name) {
        String base = Paths.get(name).getFileName().toString();
        base = base.replace('\u0000', '_');
        // allow letters, digits, dot, dash, underscore
        base = base.replaceAll("[^A-Za-z0-9._-]", "_");
        if (base.length() > 100) base = base.substring(base.length() - 100); // keep tail
        if (!base.toLowerCase().endsWith(".pdf")) base = base + ".pdf";
        return base;
    }

    private static void storePathInMySQL(String jdbcUrl, String user, String pass, String relPath) {
        if (jdbcUrl == null || jdbcUrl.isEmpty()) return;
        try (Connection conn = DriverManager.getConnection(jdbcUrl, user, pass)) {
            conn.setAutoCommit(false);
            try (Statement st = conn.createStatement()) {
                st.executeUpdate("CREATE TABLE IF NOT EXISTS documents (" +
                        "id INT AUTO_INCREMENT PRIMARY KEY," +
                        "path VARCHAR(1024) NOT NULL" +
                        ")");
            }
            try (PreparedStatement ps = conn.prepareStatement("INSERT INTO documents(path) VALUES (?)")) {
                ps.setString(1, relPath);
                ps.executeUpdate();
            }
            conn.commit();
        } catch (SQLException e) {
            // best-effort only; do not leak details to callers
        }
    }

    private static byte[] samplePdfBytes(String title) {
        String pdf = "%PDF-1.4\n" +
                "1 0 obj <</Type /Catalog /Pages 2 0 R>> endobj\n" +
                "2 0 obj <</Type /Pages /Kids [3 0 R] /Count 1>> endobj\n" +
                "3 0 obj <</Type /Page /Parent 2 0 R /MediaBox [0 0 200 200] /Contents 4 0 R>> endobj\n" +
                "4 0 obj <</Length 44>> stream\n" +
                "BT /F1 12 Tf 72 120 Td (" + title.replace(")", "\\)") + ") Tj ET\n" +
                "endstream endobj\n" +
                "xref\n0 5\n0000000000 65535 f \n" +
                "trailer <</Root 1 0 R>>\nstartxref\n0\n%%EOF\n";
        return pdf.getBytes(StandardCharsets.US_ASCII);
    }

    public static void main(String[] args) {
        String baseDir = "secure_base";
        List<String> inputs = new ArrayList<>();
        inputs.add("report.pdf");
        inputs.add("../evil.pdf");
        inputs.add("weird name.PDF");
        inputs.add("semi;colon.pdf");
        inputs.add("a/bad/path.pdf");

        for (int i = 0; i < inputs.size(); i++) {
            String name = inputs.get(i);
            try {
                byte[] data = samplePdfBytes("Test " + (i + 1));
                String stored = upload_pdf(baseDir, name, data,
                        "", "", ""); // no DB configured in tests
                byte[] back = download_pdf(baseDir, stored);
                System.out.println("OK: " + name + " -> " + stored + " size=" + back.length);
            } catch (Exception e) {
                System.out.println("FAIL: " + name + " -> " + e.getMessage());
            }
        }
    }
}