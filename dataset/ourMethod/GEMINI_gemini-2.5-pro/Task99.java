import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task99 {

    /**
     * Reads a file and returns a list of lines that start with numbers and end with a dot.
     *
     * @param filePath The path to the file to read.
     * @return A list of matching lines.
     * @throws IOException If an I/O error occurs.
     */
    public static List<String> findMatchingLines(String filePath) throws IOException {
        List<String> matchingLines = new ArrayList<>();
        // Regex: ^\d+.*\.$.
        // ^     - Start of the line
        // \d+   - One or more digits
        // .*    - Any character, zero or more times
        // \.    - A literal dot
        // $     - End of the line
        Pattern pattern = Pattern.compile("^\\d+.*\\.$");
        
        File file = new File(filePath);
        if (!file.exists() || !file.isFile() || !file.canRead()) {
             // Returning empty list for non-existent, non-file, or unreadable paths
             return matchingLines;
        }

        try (BufferedReader reader = new BufferedReader(new FileReader(file, StandardCharsets.UTF_8))) {
            String line;
            while ((line = reader.readLine()) != null) {
                Matcher matcher = pattern.matcher(line);
                if (matcher.matches()) {
                    matchingLines.add(line);
                }
            }
        }
        return matchingLines;
    }

    private static void runTestCase(int testNum, String fileName, String fileContent) {
        System.out.println("--- Test Case " + testNum + " ---");
        File testFile = new File(fileName);
        try {
            // Create and write to the test file
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(testFile, StandardCharsets.UTF_8))) {
                writer.write(fileContent);
            }

            // Run the function and print results
            System.out.println("File content:\n\"\"\"\n" + fileContent + "\"\"\"");
            List<String> results = findMatchingLines(fileName);
            System.out.println("Matching lines found: " + results.size());
            for (String line : results) {
                System.out.println(" > " + line);
            }

        } catch (IOException e) {
            System.err.println("An error occurred during test case " + testNum + ": " + e.getMessage());
        } finally {
            // Clean up the test file
            if (testFile.exists()) {
                testFile.delete();
            }
        }
        System.out.println();
    }

    public static void main(String[] args) {
        String fileName = "test_file_java.txt";

        // Test Case 1: Mixed content
        String content1 = "123 this is a test.\n"
                        + "456 another line that matches.\n"
                        + "not a match\n"
                        + "789 this line does not end with a dot\n"
                        + "and another line that does not start with a number.\n"
                        + "999.\n"
                        + "1.";
        runTestCase(1, fileName, content1);

        // Test Case 2: All lines match
        String content2 = "1. First line.\n"
                        + "20. Second line.\n"
                        + "300. Third line.";
        runTestCase(2, fileName, content2);
        
        // Test Case 3: No lines match
        String content3 = "This starts with text.\n"
                        + "123 this ends with no dot\n"
                        + "Ends with a dot but no number.";
        runTestCase(3, fileName, content3);
        
        // Test Case 4: Empty file
        String content4 = "";
        runTestCase(4, fileName, content4);
        
        // Test Case 5: Lines with special regex characters
        String content5 = "123.*.\n"
                        + "456?[]\\d+.\n"
                        + "abc.";
        runTestCase(5, fileName, content5);
    }
}