import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task99 {

    /**
     * Reads a file and finds lines that start with a number and end with a dot.
     *
     * @param filePath The path to the file to read.
     * @return A list of matching lines.
     */
    public static List<String> findMatchingLines(String filePath) {
        List<String> matchingLines = new ArrayList<>();
        // Regex: ^ (start of line), \d (a digit), .* (any characters), \. (literal dot), $ (end of line)
        // This is a secure pattern with low risk of ReDoS (Regular Expression Denial of Service).
        Pattern pattern = Pattern.compile("^\\d.*\\.$");

        // Basic path validation to prevent trivial path traversal.
        // In a real application, use a more robust library for path canonicalization.
        if (filePath == null || filePath.contains("..")) {
            System.err.println("Error: Invalid or insecure file path provided.");
            return matchingLines;
        }

        try (BufferedReader reader = Files.newBufferedReader(Paths.get(filePath))) {
            String line;
            while ((line = reader.readLine()) != null) {
                Matcher matcher = pattern.matcher(line);
                if (matcher.matches()) {
                    matchingLines.add(line);
                }
            }
        } catch (IOException | InvalidPathException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
        
        return matchingLines;
    }

    // Helper method to create a test file
    private static void createTestFile(String fileName, String content) throws IOException {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(fileName))) {
            writer.write(content);
        }
    }

    public static void main(String[] args) {
        String testFileName = "test_java.txt";

        // --- Test Cases ---

        // Test Case 1: Standard case with mixed content
        System.out.println("--- Test Case 1: Standard File ---");
        try {
            String content1 = "1. This is a matching line.\n" +
                              "Not a match.\n" +
                              "2) Also not a match.\n" +
                              "3. Another match here.\n" +
                              "4 This line does not end with a dot\n" +
                              "This line does not start with a number.\n" +
                              "55. This one is also a match.";
            createTestFile(testFileName, content1);
            List<String> matches1 = findMatchingLines(testFileName);
            System.out.println("Found " + matches1.size() + " matches:");
            matches1.forEach(System.out::println);
        } catch (IOException e) {
            System.err.println("Test Case 1 failed: " + e.getMessage());
        }
        System.out.println();

        // Test Case 2: Empty file
        System.out.println("--- Test Case 2: Empty File ---");
        try {
            createTestFile(testFileName, "");
            List<String> matches2 = findMatchingLines(testFileName);
            System.out.println("Found " + matches2.size() + " matches:");
            matches2.forEach(System.out::println);
        } catch (IOException e) {
            System.err.println("Test Case 2 failed: " + e.getMessage());
        }
        System.out.println();

        // Test Case 3: File with no matching lines
        System.out.println("--- Test Case 3: No Matches ---");
        try {
            String content3 = "Hello world\n" +
                              "Another line without a dot\n" +
                              "Ends with a dot but no number.";
            createTestFile(testFileName, content3);
            List<String> matches3 = findMatchingLines(testFileName);
            System.out.println("Found " + matches3.size() + " matches:");
            matches3.forEach(System.out::println);
        } catch (IOException e) {
            System.err.println("Test Case 3 failed: " + e.getMessage());
        }
        System.out.println();
        
        // Test Case 4: File where all lines match
        System.out.println("--- Test Case 4: All Lines Match ---");
        try {
            String content4 = "1. First line.\n" +
                              "2. Second line.\n" +
                              "3. Third line.";
            createTestFile(testFileName, content4);
            List<String> matches4 = findMatchingLines(testFileName);
            System.out.println("Found " + matches4.size() + " matches:");
            matches4.forEach(System.out::println);
        } catch (IOException e) {
            System.err.println("Test Case 4 failed: " + e.getMessage());
        }
        System.out.println();

        // Test Case 5: Non-existent file
        System.out.println("--- Test Case 5: Non-existent File ---");
        List<String> matches5 = findMatchingLines("nonexistentfile.txt");
        System.out.println("Found " + matches5.size() + " matches.");
        
        // Cleanup the test file
        new File(testFileName).delete();
    }
}