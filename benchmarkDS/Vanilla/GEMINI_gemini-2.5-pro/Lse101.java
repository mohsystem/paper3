import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Lse101 {

    /**
     * Opens a file and writes the given content to it, overwriting existing content.
     * @param fileName The name of the file to write to.
     * @param content The string content to write to the file.
     * @return true on success, false on failure.
     */
    public static boolean writeToFile(String fileName, String content) {
        // Using try-with-resources to ensure the writer is closed automatically.
        // The default mode for FileWriter is to overwrite.
        try (FileWriter writer = new FileWriter(fileName)) {
            writer.write(content);
            return true;
        } catch (IOException e) {
            System.err.println("An error occurred while writing to file: " + e.getMessage());
            return false;
        }
    }

    /**
     * Helper method to read a file and verify its content.
     * @param fileName The name of the file to read.
     * @param expectedContent The content expected to be in the file.
     */
    public static void verifyFileContent(String fileName, String expectedContent) {
        try {
            String fileContent = new String(Files.readAllBytes(Paths.get(fileName)));
            if (fileContent.equals(expectedContent)) {
                System.out.println("Verification SUCCESS for " + fileName);
            } else {
                System.out.println("Verification FAILED for " + fileName);
                System.out.println("Expected: '" + expectedContent + "'");
                System.out.println("Got: '" + fileContent + "'");
            }
        } catch (IOException e) {
            System.err.println("An error occurred while reading file for verification: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println("--- Test Case 1 ---");
        String file1 = "important_config";
        String content1 = "important_config";
        if (writeToFile(file1, content1)) {
            System.out.println("Successfully wrote to " + file1);
            verifyFileContent(file1, content1);
        }
        System.out.println();

        // Test Case 2
        System.out.println("--- Test Case 2 ---");
        String file2 = "test1.txt";
        String content2 = "Hello World";
        if (writeToFile(file2, content2)) {
            System.out.println("Successfully wrote to " + file2);
            verifyFileContent(file2, content2);
        }
        System.out.println();

        // Test Case 3
        System.out.println("--- Test Case 3 ---");
        String file3 = "data.log";
        String content3 = "Log entry: 2023-10-27";
        if (writeToFile(file3, content3)) {
            System.out.println("Successfully wrote to " + file3);
            verifyFileContent(file3, content3);
        }
        System.out.println();

        // Test Case 4
        System.out.println("--- Test Case 4 ---");
        String file4 = "empty_file.txt";
        String content4 = "";
        if (writeToFile(file4, content4)) {
            System.out.println("Successfully wrote to " + file4);
            verifyFileContent(file4, content4);
        }
        System.out.println();

        // Test Case 5
        System.out.println("--- Test Case 5 ---");
        String file5 = "another_config.ini";
        String content5 = "[settings]\nuser=admin";
        if (writeToFile(file5, content5)) {
            System.out.println("Successfully wrote to " + file5);
            verifyFileContent(file5, content5);
        }
        System.out.println();
    }
}