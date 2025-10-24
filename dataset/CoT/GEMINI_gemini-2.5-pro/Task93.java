import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.AbstractMap;
import java.util.Comparator;

public class Task93 {

    /**
     * Reads a file containing key-value pairs, sorts them by key, and returns them.
     *
     * @param filePath The path to the file.
     * @return A sorted list of Map.Entry objects representing the key-value pairs, or null on error.
     */
    public static List<Map.Entry<String, String>> sortFileRecords(String filePath) {
        List<Map.Entry<String, String>> records = new ArrayList<>();
        // Use try-with-resources to ensure the reader is closed automatically.
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            String line;
            while ((line = reader.readLine()) != null) {
                // Split the line into key and value at the first colon.
                String[] parts = line.split(":", 2);
                if (parts.length == 2) {
                    String key = parts[0].trim();
                    String value = parts[1].trim();
                    if (!key.isEmpty()) { // Ensure the key is not empty after trimming
                        records.add(new AbstractMap.SimpleEntry<>(key, value));
                    }
                }
            }
        } catch (IOException e) {
            // In case of file not found or other I/O errors, print an error and return null.
            System.err.println("Error reading file: " + e.getMessage());
            return null;
        }

        // Sort the list of entries based on the key.
        records.sort(Comparator.comparing(Map.Entry::getKey));
        
        return records;
    }

    // Main method with test cases
    public static void main(String[] args) {
        // --- Test Cases ---
        String testFileName = "test_records.txt";

        // Test Case 1: Standard case with unsorted records
        System.out.println("--- Test Case 1: Standard unsorted file ---");
        createTestFile(testFileName, "banana: 10\napple: 5\ncherry: 20\ndate: 15");
        List<Map.Entry<String, String>> sortedRecords1 = sortFileRecords(testFileName);
        printRecords(sortedRecords1);
        System.out.println();

        // Test Case 2: File with malformed lines, empty lines, and whitespace
        System.out.println("--- Test Case 2: File with malformed lines and whitespace ---");
        createTestFile(testFileName, "  fig: 30  \n\ngrape: 25\njust_a_key\n:empty_key\nelderberry:");
        List<Map.Entry<String, String>> sortedRecords2 = sortFileRecords(testFileName);
        printRecords(sortedRecords2);
        System.out.println();

        // Test Case 3: Empty file
        System.out.println("--- Test Case 3: Empty file ---");
        createTestFile(testFileName, "");
        List<Map.Entry<String, String>> sortedRecords3 = sortFileRecords(testFileName);
        printRecords(sortedRecords3);
        System.out.println();

        // Test Case 4: Non-existent file
        System.out.println("--- Test Case 4: Non-existent file ---");
        new File(testFileName).delete(); // Ensure file does not exist
        List<Map.Entry<String, String>> sortedRecords4 = sortFileRecords(testFileName);
        printRecords(sortedRecords4);
        System.out.println();

        // Test Case 5: Already sorted file
        System.out.println("--- Test Case 5: Already sorted file ---");
        createTestFile(testFileName, "alpha: 1\nbeta: 2\ngamma: 3");
        List<Map.Entry<String, String>> sortedRecords5 = sortFileRecords(testFileName);
        printRecords(sortedRecords5);
        System.out.println();

        // Cleanup the test file
        new File(testFileName).delete();
    }

    private static void createTestFile(String fileName, String content) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(fileName))) {
            writer.write(content);
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + e.getMessage());
        }
    }

    private static void printRecords(List<Map.Entry<String, String>> records) {
        if (records == null) {
            System.out.println("Function returned null (likely an error occurred).");
            return;
        }
        if (records.isEmpty()) {
            System.out.println("No valid records found or processed.");
        } else {
            for (Map.Entry<String, String> entry : records) {
                System.out.println(entry.getKey() + ": " + entry.getValue());
            }
        }
    }
}