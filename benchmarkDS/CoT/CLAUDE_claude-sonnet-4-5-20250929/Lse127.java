
import java.io.*;
import java.util.Scanner;

public class Lse127 {
    private static final int MAX_USERNAME_LENGTH = 100;
    
    public static String httpHelloPage(InputStream request, OutputStream response) throws IOException {
        BufferedReader reader = new BufferedReader(new InputStreamReader(request));
        PrintWriter writer = new PrintWriter(new OutputStreamWriter(response), true);
        
        try {
            String username = reader.readLine();
            
            // Input validation and sanitization
            if (username == null || username.trim().isEmpty()) {
                username = "Guest";
            } else {
                username = username.trim();
                if (username.length() > MAX_USERNAME_LENGTH) {
                    username = username.substring(0, MAX_USERNAME_LENGTH);
                }
                username = sanitizeInput(username);
            }
            
            String responseMessage = "Hello " + username;
            writer.println(responseMessage);
            writer.flush();
            
            return responseMessage;
        } finally {
            writer.close();
            reader.close();
        }
    }
    
    private static String sanitizeInput(String input) {
        if (input == null) return "";
        
        // Remove/encode dangerous characters
        StringBuilder sanitized = new StringBuilder();
        for (char c : input.toCharArray()) {
            switch (c) {
                case '<':
                    sanitized.append("&lt;");
                    break;
                case '>':
                    sanitized.append("&gt;");
                    break;
                case '&':
                    sanitized.append("&amp;");
                    break;
                case '"':
                    sanitized.append("&quot;");
                    break;
                case '\\'':
                    sanitized.append("&#x27;");
                    break;
                case '/':
                    sanitized.append("&#x2F;");
                    break;
                default:
                    if (c >= 32 && c <= 126) {
                        sanitized.append(c);
                    }
                    break;
            }
        }
        return sanitized.toString();
    }
    
    public static void main(String[] args) {
        String[] testCases = {
            "Alice",
            "Bob<script>alert('xss')</script>",
            "Charlie&Drop Table",
            "",
            "A".repeat(150)
        };
        
        for (int i = 0; i < testCases.length; i++) {
            try {
                ByteArrayInputStream request = new ByteArrayInputStream(testCases[i].getBytes());
                ByteArrayOutputStream response = new ByteArrayOutputStream();
                
                String result = httpHelloPage(request, response);
                System.out.println("Test " + (i + 1) + ": " + result);
            } catch (IOException e) {
                System.err.println("Test " + (i + 1) + " failed: " + e.getMessage());
            }
        }
    }
}
