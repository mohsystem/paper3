
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;

public class Lse128 {
    public static void httpHelloPage(BufferedReader request, PrintWriter response) {
        try {
            String username = "";
            String line;
            
            // Read the request headers
            while ((line = request.readLine()) != null && !line.isEmpty()) {
                // Look for username parameter in the request
                if (line.contains("username=")) {
                    int startIndex = line.indexOf("username=") + 9;
                    int endIndex = line.indexOf("&", startIndex);
                    if (endIndex == -1) {
                        endIndex = line.indexOf(" ", startIndex);
                    }
                    if (endIndex == -1) {
                        endIndex = line.length();
                    }
                    username = line.substring(startIndex, endIndex);
                    username = URLDecoder.decode(username, StandardCharsets.UTF_8.name());
                    break;
                }
            }
            
            // Sanitize username to prevent XSS attacks
            username = sanitizeInput(username);
            
            // Construct and send response
            response.println("HTTP/1.1 200 OK");
            response.println("Content-Type: text/html; charset=UTF-8");
            response.println();
            response.println("<html><body>");
            response.println("<h1>hello " + username + "</h1>");
            response.println("</body></html>");
            response.flush();
            
        } catch (Exception e) {
            response.println("HTTP/1.1 500 Internal Server Error");
            response.println();
            response.flush();
        }
    }
    
    private static String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        // Escape HTML special characters to prevent XSS
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal username
        testHttpHelloPage("GET /?username=John HTTP/1.1");
        
        // Test case 2: Username with spaces
        testHttpHelloPage("GET /?username=John+Doe HTTP/1.1");
        
        // Test case 3: Empty username
        testHttpHelloPage("GET /?username= HTTP/1.1");
        
        // Test case 4: Username with special characters (XSS attempt)
        testHttpHelloPage("GET /?username=<script>alert('xss')</script> HTTP/1.1");
        
        // Test case 5: Username with encoded characters
        testHttpHelloPage("GET /?username=Alice%20Smith HTTP/1.1");
    }
    
    private static void testHttpHelloPage(String requestLine) {
        try {
            StringReader stringReader = new StringReader(requestLine + "\\n\\n");
            BufferedReader request = new BufferedReader(stringReader);
            
            StringWriter stringWriter = new StringWriter();
            PrintWriter response = new PrintWriter(stringWriter);
            
            httpHelloPage(request, response);
            
            System.out.println("Test: " + requestLine);
            System.out.println(stringWriter.toString());
            System.out.println("---");
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
