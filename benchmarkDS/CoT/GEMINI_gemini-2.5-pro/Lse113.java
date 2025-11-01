import java.io.IOException;
import java.util.concurrent.TimeUnit;

public class Lse113 {

    /**
     * Pings a given host/URL to check if it is reachable.
     *
     * @param url The host or IP address to ping.
     * @return A string indicating if the URL is "alive" or "not alive".
     */
    public static String ping(String url) {
        // Basic validation to prevent obviously malicious input.
        // ProcessBuilder is generally safe from classic command injection,
        // but it's good practice to validate inputs.
        if (url == null || url.trim().isEmpty() || url.matches(".*[&|;`$<>\\(\\)].*")) {
            return "Invalid URL format.";
        }
        
        ProcessBuilder processBuilder;
        String os = System.getProperty("os.name").toLowerCase();

        // Differentiate ping command between Windows and other OS
        if (os.contains("win")) {
            // -n 1 sends 1 echo request
            processBuilder = new ProcessBuilder("ping", "-n", "1", url);
        } else {
            // -c 1 sends 1 packet
            processBuilder = new ProcessBuilder("ping", "-c", "1", url);
        }

        // The output of the ping command is stored in "pong" (conceptually).
        // The real check is the exit code of the process.
        try {
            Process process = processBuilder.start();
            // Wait for 10 seconds max for the process to complete
            boolean completed = process.waitFor(10, TimeUnit.SECONDS);

            if (!completed) {
                process.destroy();
                return "URL is not alive (ping timed out).";
            }
            
            int exitCode = process.exitValue();
            if (exitCode == 0) {
                return "URL is alive.";
            } else {
                return "URL is not alive.";
            }
        } catch (IOException | InterruptedException e) {
            // e.printStackTrace(); // For debugging
            return "URL is not alive (Error executing ping).";
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "google.com",         // Test case 1: A valid, reachable domain
            "8.8.8.8",            // Test case 2: A valid, reachable IP address
            "localhost",          // Test case 3: Localhost
            "invalidurl12345.xyz",// Test case 4: A non-existent domain
            "google.com; ls",     // Test case 5: A potential command injection attempt
        };

        for (String url : testUrls) {
            String pong = ping(url);
            System.out.println("Pinging " + url + ": " + pong);
        }
    }
}