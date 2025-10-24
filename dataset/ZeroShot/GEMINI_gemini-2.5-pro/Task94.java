import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Map;
import java.util.TreeMap;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.List;

public class Task94 {

    /**
     * Reads a file containing key-value pairs separated by a colon, sorts them by key,
     * and returns them in a map that maintains the sorted order.
     *
     * @param filePath The path to the file to be read. The path should be validated by the caller.
     * @return A Map containing the key-value pairs sorted by key. Returns an empty map if the file is empty or cannot be read.
     */
    public static Map<String, String> sortFileRecords(String filePath) {
        // TreeMap automatically stores entries sorted by key.
        Map<String, String> sortedRecords = new TreeMap<>();
        
        // Use try-with-resources for automatic resource management (closes the reader).
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            String line;
            while ((line = reader.readLine()) != null) {
                line = line.trim();
                if (line.isEmpty()) {
                    continue; // Skip empty lines
                }

                // Split the line into key and value at the first colon.
                // The limit parameter '2' ensures that values can contain colons.
                String[] parts = line.split(":", 2);
                
                if (parts.length == 2) {
                    String key = parts[0].trim();
                    String value = parts[1].trim();
                    if (!key.isEmpty()) { // Ensure the key is not empty
                        sortedRecords.put(key, value);
                    }
                } else {
                    // Log or handle malformed lines that do not contain a key-value pair.
                    System.err.println("Warning: Skipping malformed line: " + line);
                }
            }
        } catch (IOException e) {
            // Handle file not found or other I/O errors gracefully.
            System.err.println("Error reading file: " + e.getMessage());
        }
        
        return sortedRecords;
    }

    // Main method with test cases
    public static void main(String[] args) {
        // Test case setup
        setupTestFiles();

        String[] testFiles = {
            "test_ok.txt", 
            "test_malformed.txt", 
            "test_empty.txt", 
            "test_blank_lines.txt",
            "non_existent_file.txt"
        };

        for (int i = 0; i < testFiles.length; i++) {
            String file = testFiles[i];
            System.out.println("--- Test Case " + (i + 1) + ": Processing " + file + " ---");
            Map<String, String> result = sortFileRecords(file);
            if (result.isEmpty()) {
                System.out.println("Result is empty (as expected for empty, malformed, or non-existent files).");
            } else {
                result.forEach((key, value) -> System.out.println(key + ":" + value));
            }
            System.out.println();
        }

        // Cleanup
        cleanupTestFiles();
    }

    private static void setupTestFiles() {
        try {
            // Test Case 1: Standard file
            List<String> okLines = Arrays.asList("zulu:last", "alpha:first", "charlie:third", "bravo:second");
            Files.write(Paths.get("test_ok.txt"), okLines);

            // Test Case 2: Malformed lines
            List<String> malformedLines = Arrays.asList("key1:value1", "malformedline", "key2:value2", "another:malformed:line");
            Files.write(Paths.get("test_malformed.txt"), malformedLines);

            // Test Case 3: Empty file
            Files.createFile(Paths.get("test_empty.txt"));

            // Test Case 4: File with blank lines
            List<String> blankLines = Arrays.asList("keyA:valueA", "", "  ", "keyC:valueC", "keyB:valueB");
            Files.write(Paths.get("test_blank_lines.txt"), blankLines);

        } catch (IOException e) {
            System.err.println("Failed to set up test files: " + e.getMessage());
        }
    }

    private static void cleanupTestFiles() {
        String[] files = {"test_ok.txt", "test_malformed.txt", "test_empty.txt", "test_blank_lines.txt"};
        for (String file : files) {
            try {
                Files.deleteIfExists(Paths.get(file));
            } catch (IOException e) {
                System.err.println("Failed to delete test file " + file + ": " + e.getMessage());
            }
        }
    }
}