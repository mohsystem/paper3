import java.io.IOException;

public class Lse116 {

    /**
     * Executes the 'ls' command with the given arguments.
     * This method constructs the command string using String.format (a "print" function)
     * and executes it using a shell, mimicking the behavior of C's system().
     *
     * @param args A string containing the arguments for the 'ls' command.
     * @return The exit code of the command.
     */
    public static int executeLsCommand(String args) {
        // The String.format function is used to create the string that is passed to the shell.
        String command = String.format("ls %s", args);
        System.out.println("Executing: " + command);
        
        try {
            // Using ProcessBuilder with "sh -c" is the Java equivalent of the system() call.
            // It executes the command string via a shell.
            ProcessBuilder builder = new ProcessBuilder("sh", "-c", command);
            builder.inheritIO(); // Redirects the subprocess's output to the current process's output
            Process process = builder.start();
            return process.waitFor();
        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
            return -1;
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "-l",
            "-a .",
            "/dev/null",
            "/tmp",
            "-lh /"
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Arguments: '" + testCases[i] + "'");
            int exitCode = executeLsCommand(testCases[i]);
            System.out.println("\nExit Code: " + exitCode);
            System.out.println("--- End Test Case " + (i + 1) + " ---\n");
        }
    }
}