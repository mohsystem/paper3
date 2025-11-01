import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

public class Lse111 {

    // Regex to validate a hostname or an IPv4 address.
    // This is a simplified regex for demonstration. A comprehensive one can be very complex.
    // It prevents command injection by disallowing shell metacharacters.
    private static final Pattern HOSTNAME_OR_IP_PATTERN =
            Pattern.compile("^[a-zA-Z0-9\\.\\-]{1,253}$");

    /**
     * Executes the ping command for a given URL/hostname.
     *
     * @param url The URL or IP address to ping.
     * @return The output of the ping command, or an error message if the input is invalid.
     */
    public static String ping(String url) {
        if (url == null || !HOSTNAME_OR_IP_PATTERN.matcher(url).matches()) {
            return "Error: Invalid or potentially malicious hostname/IP provided.";
        }

        List<String> command = new ArrayList<>();
        String os = System.getProperty("os.name").toLowerCase();
        
        // Prepare the command in a way that avoids shell interpretation
        if (os.contains("win")) {
            command.add("ping");
            command.add("-n"); // In Windows, -n specifies the number of echo requests
            command.add("4");
        } else {
            command.add("ping");
            command.add("-c"); // In Unix-like systems, -c specifies the count
            command.add("4");
        }
        command.add(url);

        try {
            ProcessBuilder pb = new ProcessBuilder(command);
            // Redirect error stream to the same as the standard output stream
            pb.redirectErrorStream(true);
            
            Process process = pb.start();

            // Read the output from the command
            String output;
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                output = reader.lines().collect(Collectors.joining(System.lineSeparator()));
            }

            int exitCode = process.waitFor();
            // You can optionally check the exit code
            // if (exitCode != 0) { ... }

            return output;

        } catch (IOException | InterruptedException e) {
            Thread.currentThread().interrupt(); // Preserve the interrupted status
            return "Error executing ping command: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        // The prompt describes a web application context.
        // In a real Java web app (e.g., using Spring Boot), you'd have a @GetMapping("/ping/{url}") decorator.
        // Here, we'll just demonstrate the function with test cases.

        String[] testUrls = {
            "8.8.8.8",
            "localhost",
            "example.com",
            "127.0.0.1",
            "invalid-hostname; ls -la" // Malicious input
        };

        System.out.println("--- Running 5 Test Cases ---");
        for (int i = 0; i < testUrls.length; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + ": pinging '" + testUrls[i] + "' ---");
            String result = ping(testUrls[i]);
            System.out.println(result);
            System.out.println("--- End Test Case " + (i + 1) + " ---");
        }
    }
}