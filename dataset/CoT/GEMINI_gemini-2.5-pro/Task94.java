import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Map;
import java.util.TreeMap;

public class Task94 {

    /**
     * Reads a file containing key-value pairs separated by a colon,
     * sorts them by key, and prints the result.
     *
     * @param filePath The path to the input file.
     */
    public static void sortKeyValueFile(String filePath) {
        // TreeMap automatically sorts the entries by key.
        Map<String, String> records = new TreeMap<>();

        // Use try-with-resources to ensure the reader is closed automatically.
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            String line;
            while ((line = reader.readLine()) != null) {
                // Split the line only on the first occurrence of the delimiter.
                String[] parts = line.split(":", 2);
                if (parts.length == 2) {
                    String key = parts[0].trim();
                    String value = parts[1].trim();
                    if (!key.isEmpty()) { // Ensure the key is not empty
                        records.put(key, value);
                    }
                }
            }

            // Print the sorted records
            for (Map.Entry<String, String> entry : records.entrySet()) {
                System.out.println(entry.getKey() + ": " + entry.getValue());
            }

        } catch (IOException e) {
            System.err.println("Error processing file " + filePath + ": " + e.getMessage());
        }
    }

    // Helper method to create a test file with given content.
    private static void createTestFile(String fileName, String content) {
        try (PrintWriter out = new PrintWriter(fileName)) {
            out.print(content);
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---

        // Test Case 1: Normal file with unsorted data
        System.out.println("--- Test Case 1: Normal File ---");
        String testFile1 = "test1.txt";
        createTestFile(testFile1, "banana:fruit\napple:fruit\ncarrot:vegetable\nzucchini:vegetable\n");
        sortKeyValueFile(testFile1);
        System.out.println();

        // Test Case 2: Empty file
        System.out.println("--- Test Case 2: Empty File ---");
        String testFile2 = "test2.txt";
        createTestFile(testFile2, "");
        sortKeyValueFile(testFile2);
        System.out.println();

        // Test Case 3: File with malformed lines
        System.out.println("--- Test Case 3: Malformed Lines ---");
        String testFile3 = "test3.txt";
        createTestFile(testFile3, "one:1\ntwo_malformed\nthree:3\n:missing_key\nfour::4\n");
        sortKeyValueFile(testFile3);
        System.out.println();

        // Test Case 4: Non-existent file
        System.out.println("--- Test Case 4: Non-existent File ---");
        String testFile4 = "non_existent_file.txt";
        sortKeyValueFile(testFile4);
        System.out.println();

        // Test Case 5: File with duplicate keys (last one should win)
        System.out.println("--- Test Case 5: Duplicate Keys ---");
        String testFile5 = "test5.txt";
        createTestFile(testFile5, "key1:valueA\nkey2:valueB\nkey1:valueC\n");
        sortKeyValueFile(testFile5);
        System.out.println();

        // Cleanup test files
        try {
            Files.deleteIfExists(Paths.get(testFile1));
            Files.deleteIfExists(Paths.get(testFile2));
            Files.deleteIfExists(Paths.get(testFile3));
            Files.deleteIfExists(Paths.get(testFile5));
        } catch (IOException e) {
            System.err.println("Failed to delete test files.");
        }
    }
}