import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.regex.Pattern;

public class Lse111 {

    /**
     * Executes the system's ping command for a given URL or IP address.
     * To prevent command injection, this method validates the input and uses
     * ProcessBuilder, which separates the command from its arguments, thus avoiding
     * shell interpretation of the input.
     *
     * @param url The hostname or IP address to ping.
     * @return The output of the ping command, or an error message if input is invalid.
     */
    public static String ping(String url) {
        // Security: Validate input to ensure it is a plausible hostname or IP address
        // and contains no shell metacharacters. This regex allows alphanumeric characters,
        // hyphens, and dots.
        String validHostnameRegex = "^[a-zA-Z0-9\\.\\-]+$";
        if (url == null || !Pattern.matches(validHostnameRegex, url) || url.length() > 255) {
            return "Error: Invalid or potentially malicious hostname provided.";
        }

        // Determine the correct ping command parameters based on the operating system to be cross-platform.
        String os = System.getProperty("os.name").toLowerCase();
        ProcessBuilder pb;
        if (os.contains("win")) {
            // For Windows: ping -n 4 <url>
            pb = new ProcessBuilder("ping", "-n", "4", url);
        } else {
            // For Linux/macOS: ping -c 4 <url>
            pb = new ProcessBuilder("ping", "-c", "4", url);
        }

        // Redirect the error stream to the output stream to capture all output in one place.
        pb.redirectErrorStream(true);

        StringBuilder output = new StringBuilder();
        try {
            Process process = pb.start();
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append("\n");
                }
            }
            // Wait for the process to complete to ensure all output is captured.
            int exitCode = process.waitFor();
            output.append("\nProcess finished with exit code: ").append(exitCode);

        } catch (IOException | InterruptedException e) {
            output.append("Error executing ping command: ").append(e.getMessage());
            // It's good practice to restore the interrupted status if an InterruptedException is caught.
            if (e instanceof InterruptedException) {
                Thread.currentThread().interrupt();
            }
        }
        return output.toString();
    }

    public static void main(String[] args) {
        String[] testCases = {
            "8.8.8.8",                  // Test case 1: Valid IP
            "google.com",               // Test case 2: Valid domain
            "localhost",                // Test case 3: Local host
            "non-existent-domain.xyz",  // Test case 4: Non-existent but syntactically valid hostname
            "8.8.8.8; rm -rf /"         // Test case 5: Malicious input (should be rejected by validation)
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + ": Pinging '" + testCases[i] + "' ---");
            String result = ping(testCases[i]);
            System.out.println(result);
            System.out.println("--- End of Test Case " + (i + 1) + " ---\n");
        }
    }
}