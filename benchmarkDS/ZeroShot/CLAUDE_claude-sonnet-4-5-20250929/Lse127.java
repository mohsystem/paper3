
import java.io.*;
import java.net.*;
import java.util.*;

public class Lse127 {
    public static void httpHelloPage(BufferedReader request, PrintWriter response) throws IOException {
        String username = null;
        String line;
        
        // Read headers
        while ((line = request.readLine()) != null && !line.isEmpty()) {
            // Skip headers
        }
        
        // Read the request body to get username
        StringBuilder body = new StringBuilder();
        while (request.ready() && (line = request.readLine()) != null) {
            body.append(line);
        }
        
        // Parse username from query string or body
        String bodyStr = body.toString();
        if (bodyStr.contains("username=")) {
            String[] parts = bodyStr.split("username=");
            if (parts.length > 1) {
                username = parts[1].split("&")[0];
                // Decode URL encoding and sanitize
                username = URLDecoder.decode(username, "UTF-8");
                username = sanitizeInput(username);
            }
        }
        
        if (username == null || username.isEmpty()) {
            username = "Guest";
        }
        
        // Construct and write response
        response.println("HTTP/1.1 200 OK");
        response.println("Content-Type: text/html");
        response.println();
        response.println("<html><body>");
        response.println("<h1>Hello " + escapeHtml(username) + "</h1>");
        response.println("</body></html>");
        response.flush();
    }
    
    private static String sanitizeInput(String input) {
        if (input == null) return "";
        // Remove any control characters and limit length
        return input.replaceAll("[\\\\p{Cntrl}]", "").substring(0, Math.min(input.length(), 100));
    }
    
    private static String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    public static void main(String[] args) {
        System.out.println("Test cases for httpHelloPage:");
        
        String[] testCases = {
            "username=John",
            "username=Alice%20Smith",
            "username=%3Cscript%3Ealert%28%29%3C%2Fscript%3E",
            "username=",
            "username=Bob&other=data"
        };
        
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\\nTest Case " + (i + 1) + ":");
            try {
                StringReader reqReader = new StringReader("POST / HTTP/1.1\\r\\n\\r\\n" + testCases[i]);
                BufferedReader request = new BufferedReader(reqReader);
                StringWriter respWriter = new StringWriter();
                PrintWriter response = new PrintWriter(respWriter);
                
                httpHelloPage(request, response);
                System.out.println(respWriter.toString());
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
