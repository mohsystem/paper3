import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.nio.file.Paths;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.Statement;

public class Lse67 {

    public static boolean storeImage(String filePath, String jdbcUrl, String user, String password, String table, int id) {
        if (!isValidIdentifier(table)) {
            throw new IllegalArgumentException("Invalid table name");
        }

        File file = new File(filePath);
        if (!file.exists() || !file.isFile()) {
            System.err.println("File not found: " + filePath);
            return false;
        }

        String filename = Paths.get(filePath).getFileName().toString();

        String createSql = "CREATE TABLE IF NOT EXISTS " + table + " ("
                + "id INT PRIMARY KEY, "
                + "filename VARCHAR(255), "
                + "data LONGBLOB"
                + ")";

        String insertSql = "INSERT INTO " + table + " (id, filename, data) VALUES (?, ?, ?) "
                + "ON DUPLICATE KEY UPDATE filename=VALUES(filename), data=VALUES(data)";

        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
        } catch (ClassNotFoundException ignore) {
            // Driver auto-registered in JDBC 4+, continue
        }

        try (Connection conn = DriverManager.getConnection(jdbcUrl, user, password)) {
            try (Statement st = conn.createStatement()) {
                st.execute(createSql);
            }

            try (PreparedStatement ps = conn.prepareStatement(insertSql);
                 InputStream fis = new FileInputStream(file)) {
                ps.setInt(1, id);
                ps.setString(2, filename);
                ps.setBinaryStream(3, fis, file.length());
                int updated = ps.executeUpdate();
                return updated > 0;
            }
        } catch (Exception e) {
            System.err.println("Error storing image: " + e.getMessage());
            return false;
        }
    }

    private static boolean isValidIdentifier(String s) {
        return s != null && s.matches("[A-Za-z0-9_]+");
    }

    public static void main(String[] args) {
        String jdbcUrl = "jdbc:mysql://localhost:3306/testdb?useSSL=false&serverTimezone=UTC";
        String user = "root";
        String password = "root";
        String table = "images";

        boolean r1 = storeImage("test1.jpg", jdbcUrl, user, password, table, 1);
        System.out.println("Test 1: " + r1);

        boolean r2 = storeImage("test2.png", jdbcUrl, user, password, table, 2);
        System.out.println("Test 2: " + r2);

        boolean r3 = storeImage("test3.gif", jdbcUrl, user, password, table, 3);
        System.out.println("Test 3: " + r3);

        boolean r4 = storeImage("test4.bmp", jdbcUrl, user, password, table, 4);
        System.out.println("Test 4: " + r4);

        boolean r5 = storeImage("test5.webp", jdbcUrl, user, password, table, 5);
        System.out.println("Test 5: " + r5);
    }
}