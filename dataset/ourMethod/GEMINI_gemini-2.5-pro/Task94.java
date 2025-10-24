import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.InvalidPathException;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

public class Task94 {

    public static class Record implements Comparable<Record> {
        private final String key;
        private final String value;

        public Record(String key, String value) {
            // Rule #4: Ensure inputs are not null
            this.key = Objects.requireNonNull(key, "Key cannot be null");
            this.value = Objects.requireNonNull(value, "Value cannot be null");
        }

        public String getKey() {
            return key;
        }

        public String getValue() {
            return value;
        }

        @Override
        public int compareTo(Record other) {
            return this.key.compareTo(other.getKey());
        }

        @Override
        public String toString() {
            return key + ":" + value;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            Record record = (Record) o;
            return key.equals(record.key) && value.equals(record.value);
        }

        @Override
        public int hashCode() {
            return Objects.hash(key, value);
        }
    }

    /**
     * Reads key-value pairs from a file, sorts them by key, and returns them.
     *
     * @param filePath The path to the file. Must be a relative path within the current directory.
     * @return A sorted list of Record objects.
     * @throws IOException if an I/O error occurs.
     * @throws IllegalArgumentException for invalid file paths.
     */
    public static List<Record> sortRecordsFromFile(String filePath) throws IOException {
        // Rule #7: Validate and sanitize all external input used in path construction.
        if (filePath == null || filePath.trim().isEmpty()) {
            throw new IllegalArgumentException("File path cannot be null or empty.");
        }
        try {
            File f = new File(filePath);
            // Prohibit absolute paths and path traversal
            if (f.isAbsolute() || filePath.contains("..")) {
                 throw new IllegalArgumentException("Invalid file path: Only relative paths within the current directory are allowed.");
            }
            Paths.get(filePath); // Check for other invalid path characters
        } catch (InvalidPathException e) {
            throw new IllegalArgumentException("Invalid file path format.", e);
        }


        List<Record> records = new ArrayList<>();
        File file = new File(filePath);

        // Rule #8: Use try-with-resources to ensure resources are closed.
        try (BufferedReader reader = new BufferedReader(new FileReader(file, StandardCharsets.UTF_8))) {
            String line;
            int lineNumber = 0;
            while ((line = reader.readLine()) != null) {
                lineNumber++;
                String trimmedLine = line.trim();
                if (trimmedLine.isEmpty() || trimmedLine.startsWith("#")) {
                    continue;
                }

                int separatorIndex = trimmedLine.indexOf(':');
                // Rule #4: Validate that input conforms to the expected format.
                if (separatorIndex <= 0 || separatorIndex == trimmedLine.length() - 1) {
                    System.err.println("Warning: Malformed line " + lineNumber + ": " + line);
                    continue;
                }

                String key = trimmedLine.substring(0, separatorIndex).trim();
                String value = trimmedLine.substring(separatorIndex + 1).trim();

                if (key.isEmpty()) {
                    System.err.println("Warning: Malformed line (empty key) " + lineNumber + ": " + line);
                    continue;
                }
                records.add(new Record(key, value));
            }
        } catch (FileNotFoundException e) {
            // Re-throw with a more informative message or handle as per application logic
            throw new FileNotFoundException("File not found: " + filePath);
        }

        Collections.sort(records);
        return records;
    }

    private static void createTestFile(String fileName, String content) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(fileName, StandardCharsets.UTF_8))) {
            writer.write(content);
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + fileName);
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        String[] testFiles = {"test1.txt", "test2.txt", "test3.txt"};

        // Test Case 1: Normal file
        createTestFile("test1.txt", "c:3\n a: 1 \n b:2\n");
        // Test Case 2: Empty file
        createTestFile("test2.txt", "");
        // Test Case 3: File with malformed lines
        createTestFile("test3.txt", "apple:red\n:empty_key\nbanana:\n# a comment\nmalformed line\norange: fruit\n");

        String[] testCases = {
            "test1.txt",
            "test2.txt",
            "test3.txt",
            "non_existent_file.txt",
            "../forbidden.txt"
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + ": " + testCases[i] + " ---");
            try {
                List<Record> records = sortRecordsFromFile(testCases[i]);
                if (records.isEmpty()) {
                    System.out.println("No records found or file was empty/invalid.");
                } else {
                    records.forEach(System.out::println);
                }
            } catch (IOException | IllegalArgumentException e) {
                System.err.println("Error: " + e.getMessage());
            }
            System.out.println();
        }

        // Cleanup
        for (String fileName : testFiles) {
            new File(fileName).delete();
        }
    }
}