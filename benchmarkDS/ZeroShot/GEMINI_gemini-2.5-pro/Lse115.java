import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Lse115 {

    /**
     * Note: The prompt describes a pattern where a single formatted string is executed as a command.
     * This is a classic command injection vulnerability. A secure implementation, as provided below,
     * avoids this by never executing a command through a shell. Instead, the command ("ls") and its
     * arguments are passed as a distinct list of strings to the underlying execution mechanism
     * (ProcessBuilder). This ensures that arguments, even if they contain shell metacharacters
     * like ';' or '|', are treated as literal strings and not as separate commands to be executed.
     */

    /**
     * Creates a formatted string for use as a command argument.
     * This function corresponds to the "print function" described in the prompt,
     * but is used securely to format individual arguments, not the entire command string.
     *
     * @param format A format string.
     * @param args   Arguments referenced by the format specifiers in the format string.
     * @return A formatted string.
     */
    public static String formatArgument(String format, Object... args) {
        return String.format(format, args);
    }

    /**
     * Executes the 'ls' command with the given arguments in a secure way.
     *
     * @param args A variable number of string arguments for the 'ls' command.
     * @return The combined standard output and standard error of the command.
     */
    public static String executeLs(String... args) {
        List<String> command = new ArrayList<>();
        command.add("ls");
        command.addAll(Arrays.asList(args));

        StringBuilder output = new StringBuilder();
        try {
            ProcessBuilder pb = new ProcessBuilder(command);
            // Redirect error stream to the standard output stream
            pb.redirectErrorStream(true);
            Process process = pb.start();

            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append(System.lineSeparator());
                }
            }

            // Wait for the process to complete and check the exit code
            int exitCode = process.waitFor();
            output.append("---").append(System.lineSeparator());
            output.append("Exit Code: ").append(exitCode).append(System.lineSeparator());

        } catch (IOException | InterruptedException e) {
            // Restore interrupted status
            if (e instanceof InterruptedException) {
                Thread.currentThread().interrupt();
            }
            return "Error executing command: " + e.getMessage() + System.lineSeparator();
        }
        return output.toString();
    }

    public static void main(String[] args) {
        System.out.println("--- Test Case 1: ls -l ---");
        System.out.println(executeLs("-l"));

        System.out.println("--- Test Case 2: ls -a / ---");
        System.out.println(executeLs("-a", "/"));

        System.out.println("--- Test Case 3: ls /nonexistent_dir ---");
        System.out.println(executeLs("/nonexistent_dir"));

        // Using the formatArgument function to build an argument
        String formattedArg = formatArgument("test_file_%d.txt", 123);
        System.out.println("--- Test Case 4: ls " + formattedArg + " ---");
        System.out.println(executeLs(formattedArg));
        
        // Security Test Case: Attempting command injection.
        // The secure implementation will pass "; id" as a single argument to 'ls',
        // which will result in an error from 'ls', rather than executing the 'id' command.
        System.out.println("--- Test Case 5 (Security): ls '; id' ---");
        System.out.println(executeLs("; id"));
    }
}