
import java.io.*;
import java.util.regex.*;

public class Lse128 {
    public static void httpHelloPage(BufferedReader request, PrintWriter response) throws IOException {
        String username = "";
        String line;
        
        // Read the request to extract username
        while ((line = request.readLine()) != null && !line.isEmpty()) {
            // Look for username parameter in the request line or headers
            if (line.contains("username=")) {
                Pattern pattern = Pattern.compile("username=([^&\\\\s]+)");
                Matcher matcher = pattern.matcher(line);
                if (matcher.find()) {
                    username = matcher.group(1);
                    // URL decode the username
                    username = java.net.URLDecoder.decode(username, "UTF-8");
                }
            }
        }
        
        // Construct and write the response
        response.println("HTTP/1.1 200 OK");
        response.println("Content-Type: text/html");
        response.println();
        response.println("<html><body>");
        response.println("<h1>hello " + username + "</h1>");
        response.println("</body></html>");
        response.flush();
    }
    
    public static void main(String[] args) {
        System.out.println("Testing HTTP Hello Page Function:\\n");
        
        // Test case 1
        testCase("GET /hello?username=John HTTP/1.1", "John");
        
        // Test case 2
        testCase("GET /hello?username=Alice HTTP/1.1", "Alice");
        
        // Test case 3
        testCase("GET /hello?username=Bob123 HTTP/1.1", "Bob123");
        
        // Test case 4
        testCase("POST /hello HTTP/1.1\\nusername=Charlie", "Charlie");
        
        // Test case 5
        testCase("GET /hello?username=Test%20User HTTP/1.1", "Test User");
    }
    
    private static void testCase(String requestStr, String expectedUser) {
        try {
            StringReader stringReader = new StringReader(requestStr);
            BufferedReader request = new BufferedReader(stringReader);
            
            StringWriter stringWriter = new StringWriter();
            PrintWriter response = new PrintWriter(stringWriter);
            
            httpHelloPage(request, response);
            
            System.out.println("Request: " + requestStr.replace("\\n", " "));
            System.out.println("Response:\\n" + stringWriter.toString());
            System.out.println("---");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
