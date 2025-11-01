import java.sql.*;
import java.nio.file.*;
import java.util.Base64;
import java.io.IOException;
import java.util.Random;

public class Lse68 {

    public static long upload_image(String dbPath, String imagePath, String imageName) {
        Connection conn = null;
        try {
            byte[] data = Files.readAllBytes(Paths.get(imagePath));
            String b64 = Base64.getEncoder().encodeToString(data);

            conn = DriverManager.getConnection("jdbc:sqlite:" + dbPath);
            try (Statement st = conn.createStatement()) {
                st.executeUpdate("CREATE TABLE IF NOT EXISTS images (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, data_base64 TEXT NOT NULL)");
            }

            try (PreparedStatement ps = conn.prepareStatement("INSERT INTO images(name, data_base64) VALUES (?, ?)")) {
                ps.setString(1, imageName);
                ps.setString(2, b64);
                ps.executeUpdate();
            }

            try (Statement st = conn.createStatement();
                 ResultSet rs = st.executeQuery("SELECT last_insert_rowid()")) {
                if (rs.next()) {
                    return rs.getLong(1);
                }
            }
        } catch (Exception e) {
            return -1;
        } finally {
            try { if (conn != null) conn.close(); } catch (SQLException ignore) {}
        }
        return -1;
    }

    private static void writeDummy(String path, int size, int seed) throws IOException {
        byte[] b = new byte[size];
        Random r = new Random(seed);
        r.nextBytes(b);
        Files.write(Paths.get(path), b);
    }

    public static void main(String[] args) throws Exception {
        String dbPath = "java_images.db";
        for (int i = 1; i <= 5; i++) {
            String imgPath = "java_img_" + i + ".bin";
            writeDummy(imgPath, 20 + i, 100 + i);
            long id = upload_image(dbPath, imgPath, "java_image_" + i);
            System.out.println("Java inserted row id: " + id);
        }
    }
}