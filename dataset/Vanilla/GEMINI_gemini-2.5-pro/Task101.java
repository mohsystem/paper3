import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;

public class Task101 {

    /**
     * Creates a file with the given name, writes a simple shell script to it,
     * and makes it executable.
     *
     * @param filename The name of the file to create.
     * @return true if successful, false otherwise.
     */
    public static boolean createAndMakeExecutable(String filename) {
        try {
            File file = new File(filename);

            // Create and write to the file
            try (PrintWriter writer = new PrintWriter(file)) {
                writer.println("#!/bin/bash");
                writer.println("echo \"Hello from '" + filename + "'!\"");
            }
            System.out.println("File created: " + file.getName());

            // Set permissions to rwxr-xr-x (equivalent to chmod 755)
            // This is generally portable across POSIX-like systems.
            file.setReadable(true, false);
            file.setWritable(true, true);
            boolean success = file.setExecutable(true, false);
            
            if (success) {
                System.out.println("File permissions set to executable.");
                return true;
            } else {
                System.out.println("Failed to set file permissions to executable.");
                return false;
            }
        } catch (IOException e) {
            System.out.println("An error occurred: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        // 5 Test Cases
        String[] testFiles = {"script.sh", "test_script_1.sh", "test_script_2.sh", "another.sh", "final.sh"};
        for (int i = 0; i < testFiles.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + ": " + testFiles[i] + " ---");
            boolean success = createAndMakeExecutable(testFiles[i]);
            System.out.println("Result: " + (success ? "SUCCESS" : "FAILURE"));
            System.out.println();
        }
    }
}