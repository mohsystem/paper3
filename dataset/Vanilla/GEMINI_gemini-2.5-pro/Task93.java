import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

public class Task93 {

    /**
     * Reads a file containing key-value pairs (formatted as key:value),
     * sorts them by key, and returns them as a list of Map.Entry objects.
     *
     * @param filePath The path to the file to be read.
     * @return A sorted list of key-value entries.
     */
    public static List<Map.Entry<String, String>> sortFileRecords(String filePath) {
        List<Map.Entry<String, String>> records = new ArrayList<>();
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            String line;
            while ((line = reader.readLine()) != null) {
                String[] parts = line.split(":", 2);
                if (parts.length == 2) {
                    String key = parts[0].trim();
                    String value = parts[1].trim();
                    records.add(new AbstractMap.SimpleEntry<>(key, value));
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            return Collections.emptyList(); // Return empty list on error
        }

        // Sort the list based on the keys
        records.sort(Map.Entry.comparingByKey());

        return records;
    }

    public static void main(String[] args) {
        // 5 Test cases setup
        String[] testFileNames = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
        String[] testFileContents = {
            "banana:fruit\napple:fruit\n carrot : vegetable ", // Basic test with whitespace
            "zeta:26\nalpha:1\nbeta:2", // Alphabetical order
            "", // Empty file
            "10:ten\n2:two\n1:one", // Numeric keys (sorted as strings)
            "apple:red\nApple:green\napple:yellow" // Case sensitivity and duplicate keys
        };

        for (int i = 0; i < testFileNames.length; i++) {
            String fileName = testFileNames[i];
            String content = testFileContents[i];
            System.out.println("--- Test Case " + (i + 1) + " (" + fileName + ") ---");

            try {
                // Create and write to the test file
                Files.write(Paths.get(fileName), content.getBytes());

                // Process the file
                List<Map.Entry<String, String>> sortedRecords = sortFileRecords(fileName);

                // Print the results
                if (sortedRecords.isEmpty()) {
                    System.out.println("No records found or file is empty.");
                } else {
                    for (Map.Entry<String, String> entry : sortedRecords) {
                        System.out.println(entry.getKey() + ":" + entry.getValue());
                    }
                }

                // Clean up the test file
                Files.delete(Paths.get(fileName));

            } catch (IOException e) {
                System.err.println("An error occurred during test case " + (i + 1) + ": " + e.getMessage());
            }
            System.out.println();
        }
    }
}