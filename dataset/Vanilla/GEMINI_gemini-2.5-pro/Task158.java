import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class Task158 {

    /**
     * Executes an OS command using user-provided input.
     * This method is vulnerable to command injection because it concatenates
     * user input directly into a command string that is executed by a shell.
     *
     * @param userInput The data provided by the user, intended to be a hostname or IP.
     */
    public static void executeCommand(String userInput) {
        System.out.println("--- Executing with input: \"" + userInput + "\" ---");

        // Determine the operating system to construct the appropriate command
        String os = System.getProperty("os.name").toLowerCase();
        ProcessBuilder pb;

        try {
            if (os.contains("win")) {
                // For Windows, we use "cmd.exe /c" to execute the concatenated command string.
                // This allows command chaining with '&'.
                pb = new ProcessBuilder("cmd.exe", "/c", "ping -n 1 " + userInput);
            } else {
                // For Unix-like systems (Linux, macOS), we use "/bin/sh -c".
                // This allows command chaining with ';'.
                pb = new ProcessBuilder("/bin/sh", "-c", "ping -c 1 " + userInput);
            }
            
            // Redirect error stream to the same as the standard output stream
            pb.redirectErrorStream(true);
            Process process = pb.start();

            // Read and print the output of the command
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }

            int exitCode = process.waitFor();
            System.out.println("\nExited with code: " + exitCode);

        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        }
        System.out.println("--------------------------------------------------\n");
    }

    public static void main(String[] args) {
        // 5 test cases to demonstrate the functionality and vulnerability
        String[] testCases = {
            "google.com",                       // Case 1: Valid hostname
            "127.0.0.1",                        // Case 2: Valid IP address
            "invalidhostname",                  // Case 3: Invalid input
            "8.8.8.8; whoami",                  // Case 4: Command injection for Unix-like systems
            "8.8.8.8 & ver"                     // Case 5: Command injection for Windows
        };

        for (String testCase : testCases) {
            executeCommand(testCase);
        }
    }
}