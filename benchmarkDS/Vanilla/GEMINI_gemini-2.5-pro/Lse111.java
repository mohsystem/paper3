import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class Lse111 {

    /**
     * Executes the system's ping command for a given URL.
     *
     * @param url The URL or IP address to ping.
     * @return The output of the ping command as a String.
     */
    public static String ping(String url) {
        StringBuilder output = new StringBuilder();
        // Use a platform-independent way to determine the ping command
        String command;
        String os = System.getProperty("os.name").toLowerCase();
        if (os.contains("win")) {
            // For Windows, use -n for count
            command = "ping -n 2 " + url;
        } else {
            // For Linux, macOS, etc., use -c for count
            command = "ping -c 2 " + url;
        }

        try {
            Process process = Runtime.getRuntime().exec(command);
            
            // Capture the output from the command
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\n");
            }

            // Capture error stream as well
            BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            while ((line = errorReader.readLine()) != null) {
                output.append(line).append("\n");
            }

            // Wait for the process to finish and get exit value
            int exitVal = process.waitFor();
            if (exitVal == 0) {
                // success
            } else {
                // abnormal termination
                output.append("Ping command exited with error code: ").append(exitVal).append("\n");
            }

        } catch (IOException | InterruptedException e) {
            output.append("Exception: ").append(e.getMessage());
        }

        return output.toString();
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "google.com",
            "localhost",
            "127.0.0.1",
            "nonexistentdomain12345.xyz",
            "8.8.8.8"
        };

        for (String url : testUrls) {
            System.out.println("--- Pinging " + url + " ---");
            String result = ping(url);
            System.out.println(result);
            System.out.println("--- End of Ping for " + url + " ---\n");
        }
    }
}