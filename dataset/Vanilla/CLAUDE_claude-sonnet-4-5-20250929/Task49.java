
import java.sql.*;
import java.util.*;
import com.sun.net.httpserver.*;
import java.net.InetSocketAddress;
import java.io.*;
import org.json.*;

public class Task49 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    static {
        try {
            Class.forName("org.sqlite.JDBC");
            initDatabase();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    private static void initDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "name TEXT NOT NULL," +
                        "email TEXT NOT NULL," +
                        "age INTEGER)";
            stmt.execute(sql);
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    public static String addUser(String name, String email, int age) {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(
                 "INSERT INTO users (name, email, age) VALUES (?, ?, ?)",
                 Statement.RETURN_GENERATED_KEYS)) {
            
            pstmt.setString(1, name);
            pstmt.setString(2, email);
            pstmt.setInt(3, age);
            
            int affectedRows = pstmt.executeUpdate();
            if (affectedRows > 0) {
                ResultSet rs = pstmt.getGeneratedKeys();
                if (rs.next()) {
                    return "{\\"status\\":\\"success\\",\\"id\\":" + rs.getInt(1) + "}";
                }
            }
            return "{\\"status\\":\\"error\\",\\"message\\":\\"Failed to insert user\\"}";
        } catch (SQLException e) {
            return "{\\"status\\":\\"error\\",\\"message\\":\\"" + e.getMessage() + "\\"}";
        }
    }
    
    public static String getUsers() {
        StringBuilder result = new StringBuilder("[");
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery("SELECT * FROM users")) {
            
            boolean first = true;
            while (rs.next()) {
                if (!first) result.append(",");
                result.append("{\\"id\\":").append(rs.getInt("id"))
                      .append(",\\"name\\":\\"").append(rs.getString("name"))
                      .append("\\",\\"email\\":\\"").append(rs.getString("email"))
                      .append("\\",\\"age\\":").append(rs.getInt("age"))
                      .append("}");
                first = false;
            }
        } catch (SQLException e) {
            return "{\\"status\\":\\"error\\",\\"message\\":\\"" + e.getMessage() + "\\"}";
        }
        result.append("]");
        return result.toString();
    }
    
    public static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        server.createContext("/api/users", exchange -> {
            if ("POST".equals(exchange.getRequestMethod())) {
                InputStreamReader isr = new InputStreamReader(exchange.getRequestBody());
                BufferedReader br = new BufferedReader(isr);
                StringBuilder body = new StringBuilder();
                String line;
                while ((line = br.readLine()) != null) {
                    body.append(line);
                }
                
                try {
                    JSONObject json = new JSONObject(body.toString());
                    String name = json.getString("name");
                    String email = json.getString("email");
                    int age = json.getInt("age");
                    
                    String response = addUser(name, email, age);
                    exchange.sendResponseHeaders(200, response.length());
                    OutputStream os = exchange.getResponseBody();
                    os.write(response.getBytes());
                    os.close();
                } catch (Exception e) {
                    String response = "{\\"status\\":\\"error\\",\\"message\\":\\"Invalid JSON\\"}";
                    exchange.sendResponseHeaders(400, response.length());
                    OutputStream os = exchange.getResponseBody();
                    os.write(response.getBytes());
                    os.close();
                }
            } else if ("GET".equals(exchange.getRequestMethod())) {
                String response = getUsers();
                exchange.sendResponseHeaders(200, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        });
        
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port " + port);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Add user John");
        System.out.println(addUser("John Doe", "john@example.com", 30));
        
        System.out.println("\\nTest Case 2: Add user Jane");
        System.out.println(addUser("Jane Smith", "jane@example.com", 25));
        
        System.out.println("\\nTest Case 3: Add user Bob");
        System.out.println(addUser("Bob Johnson", "bob@example.com", 35));
        
        System.out.println("\\nTest Case 4: Add user Alice");
        System.out.println(addUser("Alice Brown", "alice@example.com", 28));
        
        System.out.println("\\nTest Case 5: Get all users");
        System.out.println(getUsers());
        
        // Uncomment to start server
        // try { startServer(8080); } catch (IOException e) { e.printStackTrace(); }
    }
}
