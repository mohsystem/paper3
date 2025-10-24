import java.io.*;
import java.util.*;

public class Task94 {

    static class KeyValuePair implements Comparable<KeyValuePair> {
        String key;
        String value;

        public KeyValuePair(String key, String value) {
            this.key = key;
            this.value = value;
        }

        @Override
        public int compareTo(KeyValuePair other) {
            return this.key.compareTo(other.key);
        }

        @Override
        public String toString() {
            return key + ":" + value;
        }
    }

    public static List<KeyValuePair> sortFileRecords(String filename) {
        List<KeyValuePair> records = new ArrayList<>();
        try (BufferedReader reader = new BufferedReader(new FileReader(filename))) {
            String line;
            while ((line = reader.readLine()) != null) {
                // Split only on the first colon to allow colons in the value
                String[] parts = line.split(":", 2);
                if (parts.length == 2) {
                    records.add(new KeyValuePair(parts[0].trim(), parts[1].trim()));
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
            return null;
        }
        Collections.sort(records);
        return records;
    }

    public static void main(String[] args) {
        String[] testContents = {
            // Test 1: Standard case
            "banana:fruit\napple:fruit\ncarrot:vegetable",
            // Test 2: Already sorted
            "alpha:1\nbeta:2\ngamma:3",
            // Test 3: Reverse sorted
            "zulu:end\nyankee:middle\nxray:start",
            // Test 4: Empty file
            "",
            // Test 5: Single entry
            "one:1"
        };

        for (int i = 0; i < testContents.length; i++) {
            String testFilename = "test_file_java_" + i + ".txt";
            File testFile = new File(testFilename);
            try {
                // Create a temporary file for the test case
                try (FileWriter writer = new FileWriter(testFile)) {
                    writer.write(testContents[i]);
                }

                System.out.println("--- Test Case " + (i + 1) + " (Java) ---");
                System.out.println("Input from " + testFilename + ":");
                System.out.println(testContents[i].isEmpty() ? "<empty>" : testContents[i]);

                List<KeyValuePair> sortedRecords = sortFileRecords(testFilename);

                System.out.println("\nSorted Output:");
                if (sortedRecords != null && !sortedRecords.isEmpty()) {
                    for (KeyValuePair kvp : sortedRecords) {
                        System.out.println(kvp);
                    }
                } else {
                    System.out.println("<no records to sort>");
                }
                System.out.println();

            } catch (IOException e) {
                System.err.println("Test setup failed: " + e.getMessage());
            } finally {
                // Clean up the temporary file
                testFile.delete();
            }
        }
    }
}