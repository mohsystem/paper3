import java.io.BufferedReader;
import java.io.PrintWriter;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.IOException;

public class Lse127 {

    /**
     * Reads a username from the request stream, constructs a "Hello [username]" message,
     * and writes it to the response stream.
     *
     * @param request  A BufferedReader to read the client's request from.
     * @param response A PrintWriter to write the response to.
     */
    public static void http_hello_page(BufferedReader request, PrintWriter response) {
        try {
            String username = request.readLine();
            if (username != null) {
                response.println("Hello " + username);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        // Test cases
        String[] testUsernames = {"Alice", "Bob", "Charlie", "David Smith", "Eve123"};

        System.out.println("--- Java Test Cases ---");
        for (int i = 0; i < testUsernames.length; i++) {
            String username = testUsernames[i];
            
            // Simulate the request stream
            StringReader requestInput = new StringReader(username);
            BufferedReader request = new BufferedReader(requestInput);

            // Simulate the response stream
            StringWriter responseOutput = new StringWriter();
            PrintWriter response = new PrintWriter(responseOutput);

            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input Username: " + username);
            
            // Call the function
            http_hello_page(request, response);
            response.flush(); // Ensure the output is written to the StringWriter

            // Get the result and print it
            String result = responseOutput.toString().trim();
            System.out.println("Generated Response: " + result);
            System.out.println("Expected Response: Hello " + username);
            System.out.println("-------------------------");
        }
    }
}