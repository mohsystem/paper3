import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class Lse116 {

    /**
     * Securely executes the 'ls' command using ProcessBuilder to prevent shell injection.
     * Arguments are passed directly to the command, bypassing shell interpretation.
     *
     * @param directory The directory path to list.
     * @return The exit code of the process, or -1 on error.
     */
    public static int executeLsCommand(String directory) {
        if (directory == null || directory.isEmpty()) {
            System.err.println("Error: Directory path cannot be null or empty.");
            return -1;
        }

        System.out.println("--- Executing 'ls " + directory + "' ---");
        try {
            // Using ProcessBuilder is the secure way to run external commands in Java.
            // It avoids invoking a shell, thus preventing command injection.
            List<String> command = new ArrayList<>();
            command.add("ls");
            command.add(directory);
            
            ProcessBuilder pb = new ProcessBuilder(command);
            Process process = pb.start();

            // Capture and print standard output
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }

            // Capture and print standard error
            BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            while ((line = errorReader.readLine()) != null) {
                System.err.println(line);
            }

            int exitCode = process.waitFor();
            System.out.println("Exited with code: " + exitCode);
            System.out.println("------------------------------------");
            return exitCode;

        } catch (IOException | InterruptedException e) {
            System.err.println("An exception occurred: " + e.getMessage());
            e.printStackTrace();
            System.out.println("------------------------------------");
            return -1;
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Current directory
        executeLsCommand(".");

        // Test Case 2: A common, valid directory like /tmp
        executeLsCommand("/tmp");

        // Test Case 3: A non-existent directory
        executeLsCommand("/non_existent_dir_12345");

        // Test Case 4: A potentially malicious string.
        // ProcessBuilder treats this as a single, literal argument. `ls` will safely
        // report that the file/directory does not exist, and no command is injected.
        executeLsCommand("&& whoami");

        // Test Case 5: Another potentially malicious string.
        // This is also handled safely as a literal filename.
        executeLsCommand(";/bin/bash");
    }
}