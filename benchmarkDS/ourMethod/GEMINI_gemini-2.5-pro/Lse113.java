import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class Lse113 {

    // Rule #1: Validate input. A regex to allow only valid hostname characters.
    // This prevents command injection by disallowing shell metacharacters.
    private static final Pattern HOSTNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9.-]+$");

    /**
     * The function ping() is called when the URL <url> ping is requested.
     * This method simulates that behavior.
     * @param url The hostname or IP address to ping.
     */
    public static void ping(String url) {
        System.out.println("--- Pinging " + url + " ---");

        if (url == null || url.trim().isEmpty() || !HOSTNAME_PATTERN.matcher(url).matches()) {
            System.out.println("Invalid or potentially malicious URL/hostname format: " + url);
            System.out.println("Result: URL is not alive.");
            System.out.println("------------------------------");
            return;
        }

        // Rule #2: Use safer alternatives. ProcessBuilder is the safe way to run
        // external commands in Java, as it treats arguments separately and does not
        // invoke a shell by default.
        List<String> command = new ArrayList<>();
        String os = System.getProperty("os.name").toLowerCase();

        if (os.contains("win")) {
            command.add("ping");
            command.add("-n");
            command.add("1");
            command.add("-w");
            command.add("1000"); // 1000 milliseconds timeout
            command.add(url);
        } else { // For Linux, macOS, etc.
            command.add("ping");
            command.add("-c");
            command.add("1");
            command.add("-W");
            command.add("1"); // 1 second timeout
            command.add(url);
        }

        ProcessBuilder pb = new ProcessBuilder(command);
        StringBuilder pongBuilder = new StringBuilder(); // To store the command's output

        try {
            Process process = pb.start();

            // Read the output from the command (stdout)
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    pongBuilder.append(line).append(System.lineSeparator());
                }
            }
            
            // Read the error output from the command (stderr)
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getErrorStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    pongBuilder.append(line).append(System.lineSeparator());
                }
            }

            int exitCode = process.waitFor();
            String pong = pongBuilder.toString();
            
            System.out.println("Command output (pong):");
            System.out.print(pong);

            // If the ping command returns 0 the URL is alive.
            if (exitCode == 0) {
                System.out.println("Result: URL '" + url + "' is alive.");
            } else {
                // Else the URL is not alive.
                System.out.println("Result: URL '" + url + "' is not alive (exit code: " + exitCode + ").");
            }

        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
        } catch (InterruptedException e) {
            System.err.println("The ping process was interrupted: " + e.getMessage());
            Thread.currentThread().interrupt(); // Restore the interrupted status
        }
        System.out.println("------------------------------");
    }

    /**
     * The main method runs test cases. In a real web application, a framework
     * would call the ping method based on a URL request, similar to a decorator.
     */
    public static void main(String[] args) {
        System.out.println("The prompt mentions a decorator to bind a function to a URL.");
        System.out.println("In Java, this is conceptually similar to annotations in frameworks like Spring or JAX-RS.");
        System.out.println("The `ping(String url)` method represents the function that would be bound to a URL endpoint.");
        System.out.println();
        
        String[] testUrls = {
            "8.8.8.8",                  // A known alive IP
            "google.com",               // A known alive hostname
            "invalid-hostname-that-does-not-exist.com", // A likely dead hostname
            "127.0.0.1",                // Localhost, should be alive
            "google.com; rm -rf /"      // Malicious input to test validation
        };

        for (String url : testUrls) {
            ping(url);
        }
    }
}