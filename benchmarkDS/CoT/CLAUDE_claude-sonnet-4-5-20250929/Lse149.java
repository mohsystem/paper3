
import java.io.*;
import java.sql.*;
import java.util.*;
import javax.servlet.*;
import javax.servlet.http.*;
import com.sun.net.httpserver.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;

public class Lse149 {
    private static Connection conn;
    
    static {
        try {
            conn = DriverManager.getConnection("jdbc:sqlite:messages.db");
            Statement stmt = conn.createStatement();
            stmt.execute("CREATE TABLE IF NOT EXISTS messages (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT NOT NULL, message TEXT NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    private static boolean insertUserMessageInDb(String username, String message) {
        if (username == null || message == null || username.trim().isEmpty() || message.trim().isEmpty()) {
            return false;
        }
        
        // Input validation
        if (username.length() > 50 || message.length() > 500) {
            return false;
        }
        
        String sql = "INSERT INTO messages (username, message) VALUES (?, ?)";
        try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, username.trim());
            pstmt.setString(2, message.trim());
            pstmt.executeUpdate();
            return true;
        } catch (SQLException e) {
            e.printStackTrace();
            return false;
        }
    }
    
    private static List<Map<String, String>> getAllMessages() {
        List<Map<String, String>> messages = new ArrayList<>();
        String sql = "SELECT username, message, created_at FROM messages ORDER BY created_at DESC";
        
        try (Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            while (rs.next()) {
                Map<String, String> msg = new HashMap<>();
                msg.put("username", escapeHtml(rs.getString("username")));
                msg.put("message", escapeHtml(rs.getString("message")));
                msg.put("created_at", rs.getString("created_at"));
                messages.add(msg);
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return messages;
    }
    
    private static String escapeHtml(String text) {
        if (text == null) return "";
        return text.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    private static Map<String, String> parsePostData(String postData) {
        Map<String, String> params = new HashMap<>();
        if (postData == null || postData.isEmpty()) return params;
        
        try {
            String[] pairs = postData.split("&");
            for (String pair : pairs) {
                String[] keyValue = pair.split("=", 2);
                if (keyValue.length == 2) {
                    params.put(URLDecoder.decode(keyValue[0], "UTF-8"), 
                             URLDecoder.decode(keyValue[1], "UTF-8"));
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return params;
    }
    
    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        
        server.createContext("/", exchange -> {
            if ("GET".equals(exchange.getRequestMethod())) {
                StringBuilder html = new StringBuilder();
                html.append("<html><body><h1>Messages</h1>");
                html.append("<form method='post' action='/post'>");
                html.append("Username: <input type='text' name='username' required maxlength='50'><br>");
                html.append("Message: <textarea name='message' required maxlength='500'></textarea><br>");
                html.append("<input type='submit' value='Post Message'></form><hr>");
                
                List<Map<String, String>> messages = getAllMessages();
                for (Map<String, String> msg : messages) {
                    html.append("<div><strong>").append(msg.get("username")).append("</strong>: ");
                    html.append(msg.get("message")).append(" <em>(").append(msg.get("created_at")).append(")</em></div>");
                }
                html.append("</body></html>");
                
                exchange.sendResponseHeaders(200, html.length());
                OutputStream os = exchange.getResponseBody();
                os.write(html.toString().getBytes());
                os.close();
            }
        });
        
        server.createContext("/post", exchange -> {
            if ("POST".equals(exchange.getRequestMethod())) {
                InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), "UTF-8");
                BufferedReader br = new BufferedReader(isr);
                String postData = br.readLine();
                
                Map<String, String> params = parsePostData(postData);
                String username = params.get("username");
                String message = params.get("message");
                
                insertUserMessageInDb(username, message);
                
                exchange.getResponseHeaders().set("Location", "/");
                exchange.sendResponseHeaders(302, -1);
            }
        });
        
        server.start();
        System.out.println("Server started on port 8000");
        
        // Test cases
        System.out.println("\\nTest Cases:");
        System.out.println("1. Valid insertion: " + insertUserMessageInDb("Alice", "Hello World"));
        System.out.println("2. Valid insertion: " + insertUserMessageInDb("Bob", "Test message"));
        System.out.println("3. Empty username: " + insertUserMessageInDb("", "Message"));
        System.out.println("4. Null values: " + insertUserMessageInDb(null, null));
        System.out.println("5. XSS attempt: " + insertUserMessageInDb("Hacker", "<script>alert('xss')</script>"));
    }
}
