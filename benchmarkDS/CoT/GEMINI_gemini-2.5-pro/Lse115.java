import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class Lse115 {

    /**
     * Formats a string using placeholders, similar to printf.
     *
     * @param format The format string.
     * @param args   The arguments to be formatted into the string.
     * @return The formatted string.
     */
    public static String customPrint(String format, Object... args) {
        return String.format(format, args);
    }

    /**
     * Executes a command line utility.
     * WARNING: Executing commands from strings is inherently dangerous if any part
     * of the string comes from an untrusted source, as it can lead to command
     * injection vulnerabilities. This implementation is for demonstration purposes only.
     *
     * @param command The command string to execute.
     * @return The output of the command (both stdout and stderr).
     */
    public static String executeCommand(String command) {
        StringBuilder output = new StringBuilder();
        Process process;
        try {
            // Using "sh -c" to execute the command string in a shell.
            // This is necessary to interpret the command as a single string
            // with arguments, but it is also the source of the security risk.
            process = new ProcessBuilder("sh", "-c", command).start();

            // Capture standard output
            try (BufferedReader stdInput = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String s;
                while ((s = stdInput.readLine()) != null) {
                    output.append(s).append("\n");
                }
            }

            // Capture standard error
            try (BufferedReader stdError = new BufferedReader(new InputStreamReader(process.getErrorStream()))) {
                String s;
                while ((s = stdError.readLine()) != null) {
                    output.append("ERROR: ").append(s).append("\n");
                }
            }

            int exitVal = process.waitFor();
            output.append("Exited with error code: ").append(exitVal);

        } catch (IOException | InterruptedException e) {
            output.append("Exception occurred: ").append(e.getMessage());
            Thread.currentThread().interrupt(); // Restore interrupted status
        }
        return output.toString();
    }

    public static void main(String[] args) {
        System.out.println("--- Test Case 1: ls -l ---");
        String command1 = customPrint("%s %s", "ls", "-l");
        System.out.println("Executing: " + command1);
        System.out.println(executeCommand(command1));
        System.out.println("--------------------------\n");
        
        System.out.println("--- Test Case 2: ls -a /tmp ---");
        String command2 = customPrint("%s %s %s", "ls", "-a", "/tmp");
        System.out.println("Executing: " + command2);
        System.out.println(executeCommand(command2));
        System.out.println("--------------------------\n");

        System.out.println("--- Test Case 3: ls -lh /var ---");
        // Note: /var may not be readable by the current user
        String command3 = customPrint("%s %s %s", "ls", "-lh", "/var");
        System.out.println("Executing: " + command3);
        System.out.println(executeCommand(command3));
        System.out.println("--------------------------\n");
        
        System.out.println("--- Test Case 4: ls / ---");
        String command4 = customPrint("%s %s", "ls", "/");
        System.out.println("Executing: " + command4);
        System.out.println(executeCommand(command4));
        System.out.println("--------------------------\n");
        
        System.out.println("--- Test Case 5: ls --version ---");
        String command5 = customPrint("%s %s", "ls", "--version");
        System.out.println("Executing: " + command5);
        System.out.println(executeCommand(command5));
        System.out.println("--------------------------\n");
    }
}