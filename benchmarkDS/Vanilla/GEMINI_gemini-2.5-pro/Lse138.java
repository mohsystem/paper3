import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.io.File;
import java.util.HashMap;
import java.util.Map;
import com.google.gson.Gson;
import static spark.Spark.*;

// Note: This code requires external libraries (SparkJava, SQLite-JDBC, Gson).
// You must have them in your project's classpath. If using Maven, add these to your pom.xml:
//
// <dependencies>
//     <dependency>
//         <groupId>com.sparkjava</groupId>
//         <artifactId>spark-core</artifactId>
//         <version>2.9.4</version>
//     </dependency>
//     <dependency>
//         <groupId>org.xerial</groupId>
//         <artifactId>sqlite-jdbc</artifactId>
//         <version>3.41.2.2</version>
//     </dependency>
//     <dependency>
//         <groupId>com.google.code.gson</groupId>
//         <artifactId>gson</artifactId>
//         <version>2.10.1</version>
//     </dependency>
// </dependencies>

public class Lse138 {

    private static final String DB_URL = "jdbc:sqlite:webapp.db";

    /**
     * Sets up the database by creating a table and populating it with sample data.
     */
    public static void setupDatabase() {
        // Delete the old DB file to ensure a clean start
        new File("webapp.db").delete();

        String createTableSQL = "CREATE TABLE IF NOT EXISTS users ("
                + " id INTEGER PRIMARY KEY,"
                + " username TEXT NOT NULL,"
                + " is_moderator INTEGER NOT NULL" // Using INTEGER 0 for false, 1 for true
                + ");";

        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            // Create table
            stmt.execute(createTableSQL);

            // Populate table with sample data
            stmt.execute("INSERT INTO users(id, username, is_moderator) VALUES(1, 'alice', 1)");
            stmt.execute("INSERT INTO users(id, username, is_moderator) VALUES(2, 'bob', 0)");
            stmt.execute("INSERT INTO users(id, username, is_moderator) VALUES(3, 'charlie', 1)");
            stmt.execute("INSERT INTO users(id, username, is_moderator) VALUES(4, 'diana', 0)");
            stmt.execute("INSERT INTO users(id, username, is_moderator) VALUES(5, 'eve', 0)");

        } catch (SQLException e) {
            System.err.println("Database setup failed: " + e.getMessage());
        }
    }

    /**
     * Connects to the database and checks if a user is a moderator.
     * @param userId The ID of the user to check.
     * @return Boolean.TRUE if moderator, Boolean.FALSE if not, null if user not found.
     */
    public static Boolean isUserModerator(int userId) {
        String sql = "SELECT is_moderator FROM users WHERE id = ?";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setInt(1, userId);
            ResultSet rs = pstmt.executeQuery();
            
            if (rs.next()) {
                // Return true if is_moderator is 1
                return rs.getInt("is_moderator") == 1;
            } else {
                // User not found
                return null;
            }
        } catch (SQLException e) {
            System.err.println("Query failed: " + e.getMessage());
            return null;
        }
    }

    public static void main(String[] args) {
        // 1. Set up the database first
        setupDatabase();

        // 2. Run test cases by directly calling the logic function
        System.out.println("--- Running 5 Test Cases ---");
        int[] testUserIds = {1, 2, 5, 3, 99}; // 5 test cases
        for (int i = 0; i < testUserIds.length; i++) {
            int userId = testUserIds[i];
            Boolean isMod = isUserModerator(userId);
            System.out.printf("Test Case %d: Checking user_id = %d\n", i + 1, userId);
            if (isMod == null) {
                System.out.println("Result: User not found.");
            } else {
                System.out.println("Result: Is moderator? " + isMod);
            }
            System.out.println("----------------------------");
        }

        // 3. Define the web app route and start the server
        port(4567);
        
        // Define route: /is_moderator/:user_id
        get("/is_moderator/:user_id", (req, res) -> {
            res.type("application/json");
            Map<String, Object> responseMap = new HashMap<>();
            Gson gson = new Gson();
            
            try {
                int userId = Integer.parseInt(req.params(":user_id"));
                Boolean isMod = isUserModerator(userId);
                
                if (isMod == null) {
                    res.status(404); // Not Found
                    responseMap.put("error", "User not found");
                } else {
                    res.status(200); // OK
                    responseMap.put("user_id", userId);
                    responseMap.put("is_moderator", isMod);
                }
            } catch (NumberFormatException e) {
                res.status(400); // Bad Request
                responseMap.put("error", "Invalid user ID format.");
            }
            return gson.toJson(responseMap);
        });

        System.out.println("\n--- Web Server Started on http://localhost:4567 ---");
        System.out.println("Try accessing: http://localhost:4567/is_moderator/1");
        System.out.println("Or: http://localhost:4567/is_moderator/2");
    }
}