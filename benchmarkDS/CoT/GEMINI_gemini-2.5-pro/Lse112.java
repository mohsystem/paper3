import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class Lse112 {

    // A simple regex to validate the hostname. It allows alphanumeric characters, hyphens, and dots.
    // This is a security measure to prevent command injection.
    private static final Pattern HOSTNAME_VALIDATION_PATTERN = Pattern.compile("^[a-zA-Z0-9.-]+$");

    /**
     * Executes the ping command for a given URL in a secure way.
     *
     * @param url The URL or IP address to ping.
     * @return The output of the ping command, or an error message if the input is invalid or an error occurs.
     */
    public static String pingUrl(String url) {
        if (url == null || !HOSTNAME_VALIDATION_PATTERN.matcher(url).matches()) {
            return "Error: Invalid characters in URL. Possible command injection attempt blocked.";
        }

        StringBuilder output = new StringBuilder();
        try {
            List<String> command = new ArrayList<>();
            String os = System.getProperty("os.name").toLowerCase();

            command.add("ping");
            if (os.contains("win")) {
                // For Windows, use -n for count
                command.add("-n");
                command.add("4");
            } else {
                // For Linux, macOS, etc., use -c for count
                command.add("-c");
                command.add("4");
            }
            command.add(url);

            // Using ProcessBuilder is the secure way to run external commands,
            // as it handles arguments correctly and prevents injection.
            ProcessBuilder processBuilder = new ProcessBuilder(command);
            processBuilder.redirectErrorStream(true); // Combine stdout and stderr
            Process process = processBuilder.start();

            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append("\n");
                }
            }

            // Wait for the process to complete
            int exitCode = process.waitFor();
            output.append("\nProcess finished with exit code: ").append(exitCode);

        } catch (IOException | InterruptedException e) {
            output.append("Error executing ping command: ").append(e.getMessage());
            Thread.currentThread().interrupt(); // set the interrupt flag
        }
        return output.toString();
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "google.com",                  // Valid domain
            "8.8.8.8",                     // Valid IP address
            "nonexistent.domain.xyz",      // Should fail to resolve
            "127.0.0.1",                   // Localhost
            "google.com; ls"               // Command injection attempt
        };

        for (String url : testUrls) {
            System.out.println("--- Pinging: " + url + " ---");
            String result = pingUrl(url);
            System.out.println(result);
            System.out.println("----------------------------------\n");
        }
    }
}