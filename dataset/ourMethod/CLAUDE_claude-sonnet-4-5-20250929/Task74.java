
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class Task74 {
    private static final int MIN_PORT = 1;
    private static final int MAX_PORT = 65535;
    private static final int DEFAULT_POSTGRES_PORT = 5432;
    
    public static Connection createPostgresConnection(String host, int port, String database, 
                                                      String username, String password) throws SQLException {
        if (host == null || host.trim().isEmpty()) {
            throw new IllegalArgumentException("Host cannot be null or empty");
        }
        if (database == null || database.trim().isEmpty()) {
            throw new IllegalArgumentException("Database name cannot be null or empty");
        }
        if (username == null || username.trim().isEmpty()) {
            throw new IllegalArgumentException("Username cannot be null or empty");
        }
        if (password == null) {
            throw new IllegalArgumentException("Password cannot be null");
        }
        if (port < MIN_PORT || port > MAX_PORT) {
            throw new IllegalArgumentException("Port must be between " + MIN_PORT + " and " + MAX_PORT);
        }
        
        String sanitizedHost = host.trim();
        String sanitizedDatabase = database.trim();
        String sanitizedUsername = username.trim();
        
        if (sanitizedHost.contains("/") || sanitizedHost.contains("\\\\") || \n            sanitizedDatabase.contains("/") || sanitizedDatabase.contains("\\\\")) {\n            throw new IllegalArgumentException("Invalid characters in host or database name");\n        }\n        \n        String url = String.format("jdbc:postgresql://%s:%d/%s", \n                                   sanitizedHost, port, sanitizedDatabase);\n        \n        Properties props = new Properties();\n        props.setProperty("user", sanitizedUsername);\n        props.setProperty("password", password);\n        props.setProperty("ssl", "true");\n        props.setProperty("sslmode", "verify-full");\n        props.setProperty("connectTimeout", "10");\n        \n        return DriverManager.getConnection(url, props);\n    }\n    \n    public static void main(String[] args) {\n        System.out.println("Test 1: Valid connection parameters");\n        try {\n            Connection conn = createPostgresConnection(\n                System.getenv("DB_HOST") != null ? System.getenv("DB_HOST") : "localhost",\n                DEFAULT_POSTGRES_PORT,\n                System.getenv("DB_NAME") != null ? System.getenv("DB_NAME") : "testdb",\n                System.getenv("DB_USER") != null ? System.getenv("DB_USER") : "testuser",\n                System.getenv("DB_PASSWORD") != null ? System.getenv("DB_PASSWORD") : "testpass"\n            );\n            System.out.println("Connection created successfully (will fail without real DB)");\n            if (conn != null) {\n                conn.close();\n            }\n        } catch (Exception e) {\n            System.out.println("Expected: " + e.getClass().getSimpleName());\n        }\n        \n        System.out.println("\
Test 2: Null host");\n        try {\n            createPostgresConnection(null, DEFAULT_POSTGRES_PORT, "testdb", "user", "pass");\n            System.out.println("FAIL: Should throw exception");\n        } catch (IllegalArgumentException e) {\n            System.out.println("PASS: " + e.getMessage());\n        } catch (Exception e) {\n            System.out.println("FAIL: Wrong exception type");\n        }\n        \n        System.out.println("\
Test 3: Invalid port");\n        try {\n            createPostgresConnection("localhost", 99999, "testdb", "user", "pass");\n            System.out.println("FAIL: Should throw exception");\n        } catch (IllegalArgumentException e) {\n            System.out.println("PASS: " + e.getMessage());\n        } catch (Exception e) {\n            System.out.println("FAIL: Wrong exception type");\n        }\n        \n        System.out.println("\
Test 4: Empty database name");\n        try {\n            createPostgresConnection("localhost", DEFAULT_POSTGRES_PORT, "", "user", "pass");\n            System.out.println("FAIL: Should throw exception");\n        } catch (IllegalArgumentException e) {\n            System.out.println("PASS: " + e.getMessage());\n        } catch (Exception e) {\n            System.out.println("FAIL: Wrong exception type");\n        }\n        \n        System.out.println("\
Test 5: Null password");\n        try {\n            createPostgresConnection("localhost", DEFAULT_POSTGRES_PORT, "testdb", "user", null);\n            System.out.println("FAIL: Should throw exception");\n        } catch (IllegalArgumentException e) {\n            System.out.println("PASS: " + e.getMessage());\n        } catch (Exception e) {\n            System.out.println("FAIL: Wrong exception type");
        }
    }
}
