import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.regex.Pattern;
import java.util.ArrayList;
import java.util.List;

public class Lse111 {

    // A reasonably strict regex for valid hostnames.
    // This helps prevent command injection by ensuring the input is a valid hostname.
    private static final Pattern HOSTNAME_PATTERN = 
        Pattern.compile("^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$");

    // A regex for valid IPv4 addresses.
    private static final Pattern IPV4_PATTERN = 
        Pattern.compile("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

    /**
     * Executes the ping command for a given URL in a secure way.
     * @param url The URL or IP address to ping.
     * @return The output of the ping command, or an error message if the input is invalid or an error occurs.
     */
    public static String ping(String url) {
        if (url == null || url.trim().isEmpty()) {
            return "Error: URL cannot be null or empty.";
        }

        // Security: Validate the input to ensure it is a valid hostname or IP address
        // This is a crucial step to prevent command injection vulnerabilities.
        if (!HOSTNAME_PATTERN.matcher(url).matches() && !IPV4_PATTERN.matcher(url).matches()) {
            return "Error: Invalid hostname or IP address format.";
        }

        StringBuilder output = new StringBuilder();
        try {
            // Determine OS to use appropriate ping command arguments
            String os = System.getProperty("os.name").toLowerCase();
            List<String> command = new ArrayList<>();
            command.add("ping");
            if (os.contains("win")) {
                // For Windows, -n specifies the number of echo requests to send.
                command.add("-n");
                command.add("4");
            } else {
                // For Linux/macOS, -c specifies the count.
                command.add("-c");
                command.add("4");
            }
            command.add(url);

            // Security: Use ProcessBuilder to execute the command. This is safer than
            // Runtime.exec(String) because it allows passing arguments separately,
            // preventing the shell from interpreting the 'url' string.
            ProcessBuilder pb = new ProcessBuilder(command);
            Process process = pb.start();

            // Read the output from the command
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append("\n");
                }
            }

            // Read error stream
            try (BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()))) {
                 String line;
                 while ((line = errorReader.readLine()) != null) {
                    output.append("ERROR: ").append(line).append("\n");
                }
            }
            
            int exitVal = process.waitFor();
            if (exitVal != 0) {
                 output.append("Ping command exited with error code: ").append(exitVal);
            }

        } catch (Exception e) {
            return "Exception occurred: " + e.getMessage();
        }

        return output.toString();
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "8.8.8.8",                  // Valid IP
            "google.com",               // Valid hostname
            "localhost",                // Valid hostname
            "invalid-hostname-",        // Invalid hostname
            "8.8.8.8; ls -la"           // Malicious command injection attempt
        };

        for (String url : testUrls) {
            System.out.println("--- Pinging: " + url + " ---");
            String result = ping(url);
            System.out.println(result);
            System.out.println("----------------------------------\n");
        }
    }
}