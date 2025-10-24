import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;

public class Task100 {

    /**
     * Searches a file for lines matching a given regular expression.
     *
     * @param regexPattern The regular expression pattern to search for.
     * @param filePath     The path to the file to be searched.
     */
    public static void searchFileWithRegex(String regexPattern, String filePath) {
        // Security: Avoid ReDoS by using a timeout if possible, though Java's standard
        // library doesn't support it directly. For now, we trust the user's pattern.
        Pattern pattern;
        try {
            pattern = Pattern.compile(regexPattern);
        } catch (PatternSyntaxException e) {
            System.err.println("Error: Invalid regular expression pattern: " + e.getMessage());
            return;
        }

        File file = new File(filePath);
        // Security: Basic check to ensure we are trying to read a file, not a directory.
        if (!file.isFile()) {
            System.err.println("Error: File not found or is a directory: " + filePath);
            return;
        }

        // Security: try-with-resources ensures the reader is closed automatically.
        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
            String line;
            int lineNumber = 0;
            while ((line = reader.readLine()) != null) {
                lineNumber++;
                Matcher matcher = pattern.matcher(line);
                if (matcher.find()) {
                    System.out.println(filePath + ":" + lineNumber + ":" + line);
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading file '" + filePath + "': " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        if (args.length == 2) {
            // Use command line arguments if provided
            searchFileWithRegex(args[0], args[1]);
        } else {
            // Otherwise, run test cases
            System.out.println("No command line arguments provided. Running test cases...");
            runTests();
        }
    }

    private static void runTests() {
        String testFileName = "test_file.txt";
        // Create a test file
        try (FileWriter writer = new FileWriter(testFileName)) {
            writer.write("Hello world, this is a test file.\n");
            writer.write("The year is 2024.\n");
            writer.write("Let's test some patterns.\n");
            writer.write("Another line for testing.\n");
            writer.write("bat, bit, but, b@t.\n");
            writer.write("Goodbye!\n");
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + e.getMessage());
            return;
        }

        System.out.println("\n--- Test Case 1: Simple Match ('test') ---");
        searchFileWithRegex("test", testFileName);

        System.out.println("\n--- Test Case 2: No Match ('nomatch') ---");
        searchFileWithRegex("nomatch", testFileName);

        System.out.println("\n--- Test Case 3: Regex Special Chars ('b.t') ---");
        searchFileWithRegex("b.t", testFileName);

        System.out.println("\n--- Test Case 4: Digit Match ('\\\\d+') ---");
        searchFileWithRegex("\\d+", testFileName);

        System.out.println("\n--- Test Case 5: Non-existent File ---");
        searchFileWithRegex("hello", "nonexistent.txt");
        
        // Clean up the test file
        new File(testFileName).delete();
    }
}