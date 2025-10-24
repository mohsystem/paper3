import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class Task93 {

    /**
     * Reads key-value pairs from a file, with each line formatted as "key:value".
     * Malformed lines are skipped.
     *
     * @param filename The path to the file to read.
     * @return A list of key-value pairs.
     */
    public static List<Map.Entry<String, String>> readFileRecords(String filename) {
        List<Map.Entry<String, String>> records = new ArrayList<>();
        // Use try-with-resources for automatic resource management.
        // Directly open the file and handle exceptions to avoid TOCTOU race conditions.
        try (BufferedReader reader = Files.newBufferedReader(Paths.get(filename), StandardCharsets.UTF_8)) {
            String line;
            while ((line = reader.readLine()) != null) {
                // Validate input format.
                String[] parts = line.split(":", 2);
                if (parts.length == 2) {
                    String key = parts[0].trim();
                    String value = parts[1].trim();
                    if (!key.isEmpty() && !value.isEmpty()) {
                        records.add(new AbstractMap.SimpleEntry<>(key, value));
                    }
                }
                // Silently ignore malformed lines.
            }
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        } catch (SecurityException e) {
            System.err.println("Security error accessing file: " + e.getMessage());
        }
        return records;
    }

    /**
     * Sorts a list of key-value pairs by key.
     *
     * @param records The list of records to sort.
     */
    public static void sortRecords(List<Map.Entry<String, String>> records) {
        if (records != null) {
            records.sort(Map.Entry.comparingByKey());
        }
    }

    /**
     * Prints a list of key-value pairs.
     *
     * @param records The list of records to print.
     */
    public static void printRecords(List<Map.Entry<String, String>> records) {
        if (records == null || records.isEmpty()) {
            System.out.println("No records to display.");
            return;
        }
        for (Map.Entry<String, String> entry : records) {
            System.out.println(entry.getKey() + ": " + entry.getValue());
        }
    }

    // Helper method to create a test file
    private static void createTestFile(String filename, String content) throws IOException {
        try (BufferedWriter writer = Files.newBufferedWriter(Paths.get(filename), StandardCharsets.UTF_8)) {
            writer.write(content);
        }
    }

    public static void main(String[] args) {
        String[] testFilenames = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
        String[] testContents = {
            "c:3\na:1\nb:2", // Test Case 1: Standard file
            "", // Test Case 2: Empty file
            "z:26", // Test Case 3: Single entry
            "b:1\na:2\nb:3", // Test Case 4: Duplicate keys
            "key1:value1\n:onlyvalue\nkeyonly:\nmalformed\nkey2:value2" // Test Case 5: Malformed lines
        };

        for (int i = 0; i < testFilenames.length; i++) {
            String filename = testFilenames[i];
            String content = testContents[i];
            System.out.println("----- Test Case " + (i + 1) + ": " + filename + " -----");
            try {
                // Setup: create the test file
                createTestFile(filename, content);

                // Execute: read records from file
                List<Map.Entry<String, String>> records = readFileRecords(filename);
                
                // Process and Print
                sortRecords(records);
                printRecords(records);

            } catch (IOException e) {
                System.err.println("Failed to run test case: " + e.getMessage());
            } finally {
                // Teardown: delete the test file
                try {
                    Files.deleteIfExists(Paths.get(filename));
                } catch (IOException e) {
                    System.err.println("Failed to delete test file: " + e.getMessage());
                }
            }
            System.out.println();
        }
    }
}