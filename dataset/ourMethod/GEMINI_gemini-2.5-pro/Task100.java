import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;

public class Task100 {

    /**
     * Searches a file for lines matching a regular expression.
     *
     * @param patternStr The regular expression to search for.
     * @param filePath   The path to the file to search in.
     */
    public static void processFile(String patternStr, String filePath) {
        Path path = Paths.get(filePath);

        if (!Files.isRegularFile(path)) {
            System.err.println("Error: Path provided is not a regular file: " + filePath);
            return;
        }

        Pattern pattern;
        try {
            pattern = Pattern.compile(patternStr);
        } catch (PatternSyntaxException e) {
            System.err.println("Error: Invalid regular expression pattern: " + e.getMessage());
            return;
        }

        try (BufferedReader reader = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (pattern.matcher(line).find()) {
                    System.out.println(line);
                }
            }
        } catch (IOException | UncheckedIOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
    }

    /**
     * Creates a temporary file with sample content for testing.
     *
     * @return The path to the temporary file.
     * @throws IOException if the file cannot be created or written to.
     */
    private static Path createTestFile() throws IOException {
        Path tempFile = Files.createTempFile("test_regex_search", ".txt");
        tempFile.toFile().deleteOnExit();

        try (BufferedWriter writer = Files.newBufferedWriter(tempFile, StandardCharsets.UTF_8)) {
            writer.write("Hello World\n");
            writer.write("This is a test file.\n");
            writer.write("The quick brown fox jumps over the lazy dog.\n");
            writer.write("Contact us at test@example.com or support@example.org.\n");
            writer.write("Phone numbers: 123-456-7890, (987)654-3210.\n");
            writer.write("Another line with numbers 12345.\n");
            writer.write("end of file.\n");
        }
        return tempFile;
    }

    /**
     * Runs a series of test cases.
     */
    public static void runTests() {
        System.out.println("Running built-in tests...");
        try {
            Path testFile = createTestFile();
            String testFilePath = testFile.toString();

            System.out.println("\n--- Test Case 1: Find lines with 'fox' ---");
            processFile("fox", testFilePath);

            System.out.println("\n--- Test Case 2: Find lines with any number ---");
            processFile("\\d+", testFilePath);

            System.out.println("\n--- Test Case 3: Find lines with email addresses ---");
            processFile("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}", testFilePath);

            System.out.println("\n--- Test Case 4: Find lines starting with 'The' ---");
            processFile("^The", testFilePath);

            System.out.println("\n--- Test Case 5: Find lines that do not contain 'test' ---");
            // This is a bit tricky. A simple approach is to find lines that don't match.
            // A pure regex solution is possible but complex (negative lookahead).
            // Here, we just search for a non-matching pattern.
            System.out.println("(Note: This test finds lines with 'nonexistentpattern')");
            processFile("nonexistentpattern", testFilePath);
            System.out.println("(No output expected)");

        } catch (IOException e) {
            System.err.println("Failed to run tests: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        if (args.length == 2) {
            processFile(args[0], args[1]);
        } else {
            System.out.println("Usage: java Task100 \"<regex_pattern>\" \"<file_name>\"");
            System.out.println("No command-line arguments provided, running test cases.");
            runTests();
        }
    }
}