public class Lse126 {

    /**
     * Simulates handling a request for the /hello page.
     *
     * @param username The request body, expected to be a username.
     * @return The constructed HTML response body.
     */
    public static String http_hello_page(String username) {
        // Construct the response body (the HTML page) into the outputbuf variable.
        String outputbuf = "<html><body><h1>Hello, " + username + "!</h1></body></html>";
        
        // In a real server, we would write this buffer to the response stream.
        // Here, we return it as a string to simulate the process.
        // The "closing" of the stream is handled by the calling function's scope ending.
        return outputbuf;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] testUsernames = {"Alice", "Bob", "Charlie", "David", "Eve"};

        for (int i = 0; i < testUsernames.length; i++) {
            // The server reads the request body (the username) into the username variable.
            String username = testUsernames[i];
            
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Received username: " + username);

            // The function http_hello_page is called when the server receives a request for the URL /hello.
            String response = http_hello_page(username);

            // The server writes the response body to the response stream.
            // (We print it to the console to simulate this)
            System.out.println("Generated response: " + response);
            System.out.println();
        }
        // The server is now ready to handle the next request (simulated by the loop).
    }
}