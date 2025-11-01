import java.io.*;
import java.nio.charset.StandardCharsets;

public class Lse128 {

    /**
     * Reads a username from a request stream and writes a greeting to a response stream.
     * Assumes the request body is in the format "username=[some_name]".
     * @param request The input stream containing the client request.
     * @param response The output stream to write the response to.
     */
    public static void http_hello_page(InputStream request, OutputStream response) throws IOException {
        // Use try-with-resources to ensure streams are closed and handled properly
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(request, StandardCharsets.UTF_8));
             BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(response, StandardCharsets.UTF_8))) {

            String requestLine = reader.readLine();
            String username = "guest"; // Default username

            if (requestLine != null && requestLine.startsWith("username=")) {
                String[] parts = requestLine.split("=", 2);
                if (parts.length == 2) {
                    username = parts[1].trim();
                }
            }
            
            String responseBody = "hello " + username;
            writer.write(responseBody);
            writer.flush();
        }
    }

    // Main method with 5 test cases
    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Standard username
        test("username=Alice", "hello Alice");

        // Test Case 2: Username with spaces
        test("username=  Bob  ", "hello Bob");
        
        // Test Case 3: Empty username
        test("username=", "hello ");
        
        // Test Case 4: Malformed request (doesn't start with "username=")
        test("user=Charlie", "hello guest");
        
        // Test Case 5: Empty request
        test("", "hello guest");
    }
    
    private static void test(String requestBody, String expectedResponse) {
        try {
            // Simulate request stream using ByteArrayInputStream
            InputStream request = new ByteArrayInputStream(requestBody.getBytes(StandardCharsets.UTF_8));
            
            // Simulate response stream using ByteArrayOutputStream
            ByteArrayOutputStream response = new ByteArrayOutputStream();
            
            // Call the function
            http_hello_page(request, response);
            
            // Get the actual response from the output stream
            String actualResponse = response.toString(StandardCharsets.UTF_8.name());
            
            // Print results
            System.out.println("Request: \"" + requestBody + "\"");
            System.out.println("Expected: \"" + expectedResponse + "\"");
            System.out.println("Actual  : \"" + actualResponse + "\"");
            System.out.println("Result  : " + (expectedResponse.equals(actualResponse) ? "PASS" : "FAIL"));
            System.out.println();

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}