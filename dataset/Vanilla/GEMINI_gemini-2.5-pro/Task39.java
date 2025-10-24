import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

public class Task39 {

    /**
     * Displays the content of a file using a system command.
     * This method is cross-platform and uses 'type' on Windows and 'cat' on other systems.
     * @param filename The name of the file to display.
     */
    public static void displayFileContent(String filename) {
        ProcessBuilder pb;
        String os = System.getProperty("os.name").toLowerCase();

        if (os.contains("win")) {
            // For Windows, 'type' is a shell command, so we run it through cmd.exe
            pb = new ProcessBuilder("cmd.exe", "/c", "type", filename);
        } else {
            // For Unix-like systems (Linux, macOS), 'cat' is a standard executable
            pb = new ProcessBuilder("cat", filename);
        }
        
        try {
            // Redirect the process's I/O to the current Java process's I/O
            pb.inheritIO();
            Process process = pb.start();
            
            // Wait for the process to complete and check for errors
            int exitCode = process.waitFor();
            if (exitCode != 0) {
                System.err.println("Command executed with an error, exit code: " + exitCode);
            }
        } catch (IOException | InterruptedException e) {
            System.err.println("An error occurred while executing the command: " + e.getMessage());
            // e.printStackTrace(); // Uncomment for detailed stack trace
        }
    }

    public static void main(String[] args) {
        List<String> testFiles = new ArrayList<>();
        for (int i = 1; i <= 5; i++) {
            testFiles.add("java_test_file_" + i + ".txt");
        }

        System.out.println("--- Creating test files ---");
        try {
            for (int i = 0; i < testFiles.size(); i++) {
                String filename = testFiles.get(i);
                try (FileWriter writer = new FileWriter(filename)) {
                    writer.write("Hello from Java test file " + (i + 1) + ".\n");
                    writer.write("This is the content of " + filename + ".\n");
                }
                System.out.println("Created: " + filename);
            }
        } catch (IOException e) {
            System.err.println("Error creating test files: " + e.getMessage());
            return;
        }

        System.out.println("\n--- Running Java Test Cases ---");
        for (int i = 0; i < testFiles.size(); i++) {
            String filename = testFiles.get(i);
            System.out.println("\n--- Test Case " + (i + 1) + ": Displaying " + filename + " ---");
            displayFileContent(filename);
            System.out.println("--- End of " + filename + " ---");
        }

        System.out.println("\n--- Cleaning up test files ---");
        for (String filename : testFiles) {
            try {
                Files.deleteIfExists(Paths.get(filename));
                System.out.println("Deleted: " + filename);
            } catch (IOException e) {
                System.err.println("Error deleting file " + filename + ": " + e.getMessage());
            }
        }
    }
}