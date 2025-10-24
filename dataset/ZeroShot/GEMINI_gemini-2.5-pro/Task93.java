import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Map;
import java.util.TreeMap;
import java.util.stream.Collectors;

public class Task93 {

    /**
     * Reads a file containing key-value pairs separated by a colon, sorts them by key,
     * and returns them as a sorted map.
     * This implementation uses a TreeMap to automatically keep the keys sorted.
     *
     * @param filePath The path to the input file.
     * @return A TreeMap containing the sorted key-value pairs.
     * @throws IOException if an I/O error occurs reading from the file.
     * @throws SecurityException if a security manager denies read access to the file.
     */
    public static Map<String, String> readAndSortFile(String filePath) throws IOException {
        // Using TreeMap to store key-value pairs, which keeps them sorted by key automatically.
        Map<String, String> sortedRecords = new TreeMap<>();

        // Using try-with-resources to ensure the BufferedReader is closed automatically.
        try (BufferedReader reader = Files.newBufferedReader(Paths.get(filePath))) {
            String line;
            while ((line = reader.readLine()) != null) {
                // Skip empty or blank lines
                if (line.trim().isEmpty()) {
                    continue;
                }

                // Split the line into key and value at the first colon.
                // The limit parameter '2' ensures that values containing colons are handled correctly.
                String[] parts = line.split(":", 2);

                if (parts.length == 2) {
                    String key = parts[0].trim();
                    String value = parts[1].trim();
                    if (!key.isEmpty()) { // Ensure the key is not empty
                        sortedRecords.put(key, value);
                    }
                }
                // Malformed lines (without a colon) are ignored.
            }
        }
        return sortedRecords;
    }

    public static void main(String[] args) {
        // Test cases
        String[] testContents = {
            // Test Case 1: Standard case with mixed order
            "banana:yellow\napple:red\norange:orange\ngrape:purple",
            // Test Case 2: Empty file
            "",
            // Test Case 3: File with malformed lines, empty lines, and extra whitespace
            "name: Alice\n\nage: 30\noccupation :Software Engineer\nlocation\n\ncity:New York",
            // Test Case 4: Case sensitivity test
            "Apple:fruit\napple:fruit\nZebra:animal\nzoo:place",
            // Test Case 5: Special characters in values and keys
            "url:http://example.com?a=1&b=2\n$pecial-key:value with spaces\nemail:test@example.com"
        };
        
        for (int i = 0; i < testContents.length; i++) {
            String testFileName = "test_case_" + (i + 1) + ".txt";
            System.out.println("--- Running Test Case " + (i + 1) + " ---");
            
            try {
                // 1. Create and write to the test file
                Files.write(Paths.get(testFileName), testContents[i].getBytes());
                
                // 2. Process the file
                Map<String, String> sortedRecords = readAndSortFile(testFileName);
                
                // 3. Print the results
                if (sortedRecords.isEmpty()) {
                    System.out.println("No valid records found or file was empty.");
                } else {
                    sortedRecords.forEach((key, value) -> System.out.println(key + ":" + value));
                }

            } catch (IOException e) {
                System.err.println("An error occurred during test case " + (i + 1) + ": " + e.getMessage());
            } finally {
                // 4. Clean up the test file
                try {
                    Files.deleteIfExists(Paths.get(testFileName));
                } catch (IOException e) {
                    System.err.println("Failed to delete test file: " + testFileName);
                }
            }
            System.out.println();
        }
    }
}