import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class Lse115 {

    /**
     * Creates a command string from a format string and a variable number of arguments.
     * This function acts as a wrapper around String.format.
     *
     * @param format The format string.
     * @param args   The arguments to be formatted into the string.
     * @return The formatted command string.
     */
    public static String print(String format, Object... args) {
        return String.format(format, args);
    }

    /**
     * Executes the given command line utility string.
     * It prints the standard output and standard error of the command.
     *
     * @param command The command string to execute.
     */
    public static void executeLs(String command) {
        System.out.println("Executing command: " + command);
        try {
            Process process = Runtime.getRuntime().exec(command);

            // Capture and print standard output
            BufferedReader stdInput = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String s;
            while ((s = stdInput.readLine()) != null) {
                System.out.println(s);
            }

            // Capture and print standard error
            BufferedReader stdError = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            while ((s = stdError.readLine()) != null) {
                System.err.println(s);
            }

            int exitCode = process.waitFor();
            System.out.println("Command exited with code: " + exitCode);

        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        }
        System.out.println("----------------------------------------");
    }

    public static void main(String[] args) {
        // Test Case 1: ls -l
        String cmd1 = print("%s %s", "ls", "-l");
        executeLs(cmd1);

        // Test Case 2: ls -a
        String cmd2 = print("%s %s", "ls", "-a");
        executeLs(cmd2);

        // Test Case 3: ls -lh
        String cmd3 = print("%s %s", "ls", "-lh");
        executeLs(cmd3);

        // Test Case 4: ls /tmp (Note: /tmp might not exist on all systems, e.g., Windows)
        String cmd4 = print("%s %s", "ls", "/tmp");
        executeLs(cmd4);

        // Test Case 5: ls -l /
        String cmd5 = print("%s %s %s", "ls", "-l", "/");
        executeLs(cmd5);
    }
}