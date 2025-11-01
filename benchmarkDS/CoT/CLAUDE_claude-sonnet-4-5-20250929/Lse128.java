
import java.io.*;
import java.util.regex.Pattern;

public class Lse128 {
    private static final int MAX_USERNAME_LENGTH = 100;
    private static final Pattern SAFE_USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_\\\\- ]{1,100}$");
    
    public static String http_hello_page(BufferedReader request, PrintWriter response) throws IOException {
        String username = extractUsername(request);
        
        if (username == null || username.trim().isEmpty()) {
            username = "Guest";
        }
        
        // Sanitize username
        username = sanitizeInput(username);
        
        // Construct HTTP response
        String message = "hello " + htmlEncode(username);
        
        response.println("HTTP/1.1 200 OK");
        response.println("Content-Type: text/html; charset=UTF-8");
        response.println("Content-Length: " + message.length());
        response.println();
        response.println(message);
        response.flush();
        
        return message;
    }
    
    private static String extractUsername(BufferedReader request) throws IOException {
        String line;
        while ((line = request.readLine()) != null && !line.isEmpty()) {
            if (line.toLowerCase().startsWith("username:")) {
                return line.substring(9).trim();
            }
        }
        return "Guest";
    }
    
    private static String sanitizeInput(String input) {
        if (input == null) return "";
        
        // Limit length
        if (input.length() > MAX_USERNAME_LENGTH) {
            input = input.substring(0, MAX_USERNAME_LENGTH);
        }
        
        // Allow only safe characters
        if (!SAFE_USERNAME_PATTERN.matcher(input).matches()) {
            input = input.replaceAll("[^a-zA-Z0-9_\\\\- ]", "");
        }
        
        return input.trim();
    }
    
    private static String htmlEncode(String input) {
        if (input == null) return "";
        
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\\"", "&quot;")
                    .replace("'", "&#x27;")
                    .replace("/", "&#x2F;");
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal username
        testCase("username: John", "hello John");
        
        // Test case 2: Empty username
        testCase("", "hello Guest");
        
        // Test case 3: Username with special characters (should be sanitized)
        testCase("username: <script>alert('xss')</script>", "hello scriptalertxssscript");
        
        // Test case 4: Long username (should be truncated)
        testCase("username: " + "a".repeat(150), "hello " + "a".repeat(100));
        
        // Test case 5: Username with SQL injection attempt
        testCase("username: admin' OR '1'='1", "hello admin OR 11");
    }
    
    private static void testCase(String requestData, String expected) {
        try {
            StringReader sr = new StringReader(requestData);
            BufferedReader request = new BufferedReader(sr);
            StringWriter sw = new StringWriter();
            PrintWriter response = new PrintWriter(sw);
            
            String result = http_hello_page(request, response);
            System.out.println("Input: " + requestData);
            System.out.println("Output: " + result);
            System.out.println("Expected: " + expected);
            System.out.println("---");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
