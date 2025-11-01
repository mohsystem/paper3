import java.io.*;
import java.nio.file.*;
import java.sql.*;
import java.time.*;
import java.time.format.DateTimeFormatter;
import java.util.*;

public class Lse64 {

    // Database configuration
    private static final String DB_HOST = "127.0.0.1";
    private static final int DB_PORT = 3306;
    private static final String DB_NAME = "test";
    private static final String DB_USER = "root";
    private static final String DB_PASS = "";
    private static final String JDBC_URL = "jdbc:mysql://" + DB_HOST + ":" + DB_PORT + "/" + DB_NAME + "?useSSL=false&allowPublicKeyRetrieval=true&serverTimezone=UTC";

    private static final String UPLOAD_DIR = "uploads";

    public static void main(String[] args) throws Exception {
        ensureUploadsDir();
        try (Connection conn = getConnection()) {
            ensureTable(conn);

            // 5 test cases
            for (int i = 1; i <= 5; i++) {
                byte[] pdfData = makeMinimalPdf(("Test PDF " + i).getBytes());
                String originalFilename = "file" + i + ".pdf";
                long id = upload_pdf(conn, pdfData, originalFilename);
                byte[] downloaded = download_pdf(conn, id);
                System.out.println("Inserted id=" + id + ", downloaded bytes=" + downloaded.length);
            }
        }
    }

    public static long upload_pdf(Connection conn, byte[] fileBytes, String originalFilename) throws Exception {
        ensureUploadsDir();

        String safeName = deriveSafeFilename(originalFilename);
        String unique = UUID.randomUUID().toString();
        String finalName = unique + "-" + safeName;
        Path uploadPath = Paths.get(UPLOAD_DIR).resolve(finalName).toAbsolutePath();

        Files.write(uploadPath, fileBytes);

        String sql = "INSERT INTO pdf_uploads (filename, filepath) VALUES (?, ?)";
        try (PreparedStatement ps = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS)) {
            ps.setString(1, originalFilename);
            ps.setString(2, uploadPath.toString());
            ps.executeUpdate();
            try (ResultSet rs = ps.getGeneratedKeys()) {
                if (rs.next()) return rs.getLong(1);
            }
        }
        throw new SQLException("Failed to insert record");
    }

    public static byte[] download_pdf(Connection conn, long id) throws Exception {
        String sql = "SELECT filepath FROM pdf_uploads WHERE id = ?";
        String filepath = null;
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setLong(1, id);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) filepath = rs.getString(1);
            }
        }
        if (filepath == null) throw new FileNotFoundException("No record for id " + id);
        Path path = Paths.get(filepath);
        return Files.readAllBytes(path);
    }

    private static void ensureUploadsDir() throws IOException {
        Path dir = Paths.get(UPLOAD_DIR);
        if (!Files.exists(dir)) {
            Files.createDirectories(dir);
        }
    }

    private static void ensureTable(Connection conn) throws SQLException {
        String ddl = ""
                + "CREATE TABLE IF NOT EXISTS pdf_uploads ("
                + "  id BIGINT AUTO_INCREMENT PRIMARY KEY,"
                + "  filename VARCHAR(255) NOT NULL,"
                + "  filepath VARCHAR(1024) NOT NULL,"
                + "  uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                + ")";
        try (Statement st = conn.createStatement()) {
            st.execute(ddl);
        }
    }

    private static Connection getConnection() throws Exception {
        Class.forName("com.mysql.cj.jdbc.Driver");
        return DriverManager.getConnection(JDBC_URL, DB_USER, DB_PASS);
    }

    private static String deriveSafeFilename(String name) {
        if (name == null || name.isEmpty()) {
            return "upload.pdf";
        }
        String base = name.replaceAll("[\\\\/:*?\"<>|\\s]+", "_");
        if (!base.toLowerCase(Locale.ROOT).endsWith(".pdf")) {
            base = base + ".pdf";
        }
        return base;
    }

    private static byte[] makeMinimalPdf(byte[] message) throws IOException {
        // Construct a minimal valid PDF with embedded message as a comment
        String header = "%PDF-1.4\n";
        String obj1 = "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";
        String obj2 = "2 0 obj\n<< /Type /Pages /Count 0 >>\nendobj\n";
        String trailer = "trailer\n<< /Root 1 0 R >>\n%%EOF\n";
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        baos.write(header.getBytes());
        baos.write(("% " + Base64.getEncoder().encodeToString(message) + "\n").getBytes());
        baos.write(obj1.getBytes());
        baos.write(obj2.getBytes());
        baos.write(trailer.getBytes());
        return baos.toByteArray();
    }
}