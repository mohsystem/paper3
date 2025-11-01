import java.sql.*;
import java.util.Base64;
import java.util.Arrays;

public class Lse68 {
    private static final int MAX_IMAGE_SIZE = 5 * 1024 * 1024; // 5 MB

    public static void initDb(Connection conn) throws SQLException {
        String create = "CREATE TABLE IF NOT EXISTS images (" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                "name TEXT NOT NULL, " +
                "data TEXT NOT NULL, " +
                "created_at TEXT DEFAULT CURRENT_TIMESTAMP)";
        try (Statement st = conn.createStatement()) {
            st.execute(create);
        }
    }

    private static boolean isValidImageName(String name) {
        if (name == null) return false;
        if (name.length() == 0 || name.length() > 255) return false;
        // Allow only alphanumeric, dot, underscore, hyphen
        return name.matches("^[A-Za-z0-9._-]+$");
    }

    public static long upload_image(Connection conn, String imageName, byte[] imageBytes) throws SQLException {
        if (conn == null) throw new IllegalArgumentException("Connection cannot be null");
        if (!isValidImageName(imageName)) return -1;
        if (imageBytes == null || imageBytes.length == 0 || imageBytes.length > MAX_IMAGE_SIZE) return -1;

        String b64 = Base64.getEncoder().encodeToString(imageBytes);
        String sql = "INSERT INTO images(name, data) VALUES(?, ?)";
        boolean oldAutoCommit = conn.getAutoCommit();
        try {
            conn.setAutoCommit(false);
            try (PreparedStatement ps = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS)) {
                ps.setString(1, imageName);
                ps.setString(2, b64);
                int affected = ps.executeUpdate();
                if (affected != 1) {
                    conn.rollback();
                    return -1;
                }
                try (ResultSet rs = ps.getGeneratedKeys()) {
                    if (rs.next()) {
                        long id = rs.getLong(1);
                        conn.commit();
                        return id;
                    } else {
                        conn.rollback();
                        return -1;
                    }
                }
            }
        } finally {
            conn.setAutoCommit(oldAutoCommit);
        }
    }

    private static byte[] repeat(byte b, int count) {
        byte[] arr = new byte[count];
        Arrays.fill(arr, b);
        return arr;
    }

    public static void main(String[] args) {
        try {
            try {
                Class.forName("org.sqlite.JDBC");
            } catch (ClassNotFoundException ignored) {
                // Driver may be auto-loaded in newer JDBC implementations
            }
            try (Connection conn = DriverManager.getConnection("jdbc:sqlite::memory:")) {
                initDb(conn);

                byte[] img1 = new byte[] {(byte)0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A}; // PNG header
                byte[] img2 = new byte[] {(byte)0xFF, (byte)0xD8, (byte)0xFF, (byte)0xE0}; // JPEG header
                byte[] img3 = new byte[] { 'G','I','F','8','9','a' }; // GIF header
                byte[] img4 = repeat((byte)0xAB, 1024); // 1KB data
                byte[] img5 = repeat((byte)0xCD, 2048); // 2KB data

                long id1 = upload_image(conn, "sample1.png", img1);
                long id2 = upload_image(conn, "photo-02.jpg", img2);
                long id3 = upload_image(conn, "icon.gif", img3);
                long id4 = upload_image(conn, "large_block.bin", img4);
                long id5 = upload_image(conn, "data_chunk.dat", img5);

                System.out.println("Inserted IDs: " + id1 + ", " + id2 + ", " + id3 + ", " + id4 + ", " + id5);

                try (Statement st = conn.createStatement();
                     ResultSet rs = st.executeQuery("SELECT COUNT(*) FROM images")) {
                    if (rs.next()) {
                        System.out.println("Total rows: " + rs.getInt(1));
                    }
                }
            }
        } catch (SQLException e) {
            System.err.println("DB Error: " + e.getMessage());
        }
    }
}