import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

public class Lse113 {

    // Security: A regex pattern to validate the hostname.
    // Allows alphanumeric characters, hyphens, and dots.
    // This prevents shell injection characters.
    private static final Pattern HOSTNAME_PATTERN = Pattern.compile(
        "^[a-zA-Z0-9-.]+$"
    );

    /**
     * A simple container for the ping result.
     */
    public static class PingResult {
        public String status;
        public String pong;

        public PingResult(String status, String pong) {
            this.status = status;
            this.pong = pong;
        }

        @Override
        public String toString() {
            return "Status: " + status + "\nOutput (pong):\n" + pong;
        }
    }

    /**
     * Executes the ping command for a given URL in a secure way.
     * @param url The URL or IP address to ping.
     * @return A PingResult object containing the status and command output.
     */
    public static PingResult pingUrl(String url) {
        // Security: Validate the input against the allowed pattern to prevent command injection.
        if (url == null || url.trim().isEmpty() || !HOSTNAME_PATTERN.matcher(url).matches()) {
            String pong = "Error: Invalid characters in URL. Only alphanumerics, hyphens, and dots are allowed.";
            return new PingResult("URL '" + url + "' is invalid.", pong);
        }

        String os = System.getProperty("os.name").toLowerCase();
        ProcessBuilder processBuilder;

        // Create a platform-specific command.
        // Using ProcessBuilder with a list of arguments is the secure way to execute
        // commands, as it avoids shell interpretation.
        if (os.contains("win")) {
            processBuilder = new ProcessBuilder("ping", "-n", "4", url);
        } else {
            processBuilder = new ProcessBuilder("ping", "-c", "4", url);
        }

        // Redirect error stream to the standard output stream for easier capture
        processBuilder.redirectErrorStream(true);

        StringBuilder pongBuilder = new StringBuilder();
        try {
            Process process = processBuilder.start();

            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    pongBuilder.append(line).append(System.lineSeparator());
                }
            }

            // Wait for the process to complete, with a timeout
            if (!process.waitFor(10, TimeUnit.SECONDS)) {
                 process.destroyForcibly();
                 String pong = pongBuilder.toString() + "\nError: Ping command timed out.";
                 return new PingResult("The URL '" + url + "' is not alive (timeout).", pong);
            }

            int exitCode = process.exitValue();
            String pong = pongBuilder.toString();

            if (exitCode == 0) {
                return new PingResult("The URL '" + url + "' is alive.", pong);
            } else {
                return new PingResult("The URL '" + url + "' is not alive.", pong);
            }

        } catch (Exception e) {
            String pong = pongBuilder.toString() + "\nAn error occurred: " + e.getMessage();
            return new PingResult("The URL '" + url + "' could not be checked.", pong);
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "8.8.8.8",
            "invalid-hostname-that-does-not-exist-12345.com",
            "127.0.0.1",
            "google.com",
            "google.com; ls -la" // Malicious input
        };

        for (String url : testUrls) {
            PingResult result = pingUrl(url);
            System.out.println("--- Testing URL: " + url + " ---");
            System.out.println(result);
            System.out.println("---------------------------");
        }
    }
}