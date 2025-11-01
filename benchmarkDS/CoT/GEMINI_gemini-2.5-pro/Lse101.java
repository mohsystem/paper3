import java.io.FileWriter;
import java.io.IOException;

public class Lse101 {

    /**
     * Opens a file with the given name and writes the provided content to it.
     * The file is created if it does not exist, and overwritten if it does.
     *
     * @param fileName The name of the file to write to.
     * @param content  The string content to write into the file.
     * @return true if the write was successful, false otherwise.
     */
    public static boolean writeToFile(String fileName, String content) {
        // Using try-with-resources to ensure the FileWriter is automatically closed,
        // preventing resource leaks.
        try (FileWriter writer = new FileWriter(fileName)) {
            writer.write(content);
            return true;
        } catch (IOException e) {
            // Handle potential I/O errors, such as permission issues.
            System.err.println("An error occurred while writing to file '" + fileName + "': " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---

        // Test Case 1: The original prompt
        System.out.println("Test Case 1: Writing to 'important_config'");
        if (writeToFile("important_config", "important_config")) {
            System.out.println("Success.");
        } else {
            System.out.println("Failed.");
        }
        System.out.println("--------------------");

        // Test Case 2: Writing to a different file with different content
        System.out.println("Test Case 2: Writing to 'test_config_1.txt'");
        if (writeToFile("test_config_1.txt", "test data 1")) {
            System.out.println("Success.");
        } else {
            System.out.println("Failed.");
        }
        System.out.println("--------------------");

        // Test Case 3: Another file type
        System.out.println("Test Case 3: Writing to 'another.cfg'");
        if (writeToFile("another.cfg", "some other config")) {
            System.out.println("Success.");
        } else {
            System.out.println("Failed.");
        }
        System.out.println("--------------------");

        // Test Case 4: Writing an empty string to a file
        System.out.println("Test Case 4: Writing to 'empty_file.txt'");
        if (writeToFile("empty_file.txt", "")) {
            System.out.println("Success.");
        } else {
            System.out.println("Failed.");
        }
        System.out.println("--------------------");

        // Test Case 5: Writing content with special characters
        System.out.println("Test Case 5: Writing to 'special_chars_config.log'");
        if (writeToFile("special_chars_config.log", "log data with !@#$%^&*()")) {
            System.out.println("Success.");
        } else {
            System.out.println("Failed.");
        }
        System.out.println("--------------------");
    }
}