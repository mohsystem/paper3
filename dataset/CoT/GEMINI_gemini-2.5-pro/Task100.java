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
     * @param fileName     The path to the file to be searched.
     */
    public static void searchFileWithRegex(String regexPattern, String fileName) {
        // Step 1: Compile the regular expression
        Pattern pattern;
        try {
            pattern = Pattern.compile(regexPattern);
        } catch (PatternSyntaxException e) {
            System.err.println("Error: Invalid regular expression pattern: " + e.getMessage());
            return;
        }

        File file = new File(fileName);
        if (!file.exists() || !file.canRead()) {
            System.err.println("Error: File not found or cannot be read: " + fileName);
            return;
        }

        // Step 2: Read the file and search for the pattern line by line
        // Using try-with-resources to ensure the reader is closed automatically
        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
            String line;
            int lineNumber = 1;
            boolean found = false;
            while ((line = reader.readLine()) != null) {
                Matcher matcher = pattern.matcher(line);
                if (matcher.find()) {
                    System.out.println("Match found on line " + lineNumber + ": " + line);
                    found = true;
                }
                lineNumber++;
            }
            if (!found) {
                System.out.println("No matches found for pattern '" + regexPattern + "' in file '" + fileName + "'.");
            }
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // Check if command line arguments are provided
        if (args.length == 2) {
            System.out.println("--- Running with Command Line Arguments ---");
            String regex = args[0];
            String file = args[1];
            System.out.println("Pattern: " + regex);
            System.out.println("File: " + file);
            System.out.println("-------------------------------------------");
            searchFileWithRegex(regex, file);
        } else {
            // Run pre-defined test cases if no arguments are given
            System.out.println("Usage: java Task100 <regex_pattern> <file_name>");
            System.out.println("Running built-in test cases as no arguments were provided.");
            runTests();
        }
    }

    private static void runTests() {
        String testFileName = "test_file.txt";
        // Create a temporary test file
        try (FileWriter writer = new FileWriter(testFileName)) {
            writer.write("Hello world, this is a test file.\n");
            writer.write("The quick brown fox jumps over the lazy dog.\n");
            writer.write("123-456-7890 is a phone number.\n");
            writer.write("Another line with the word world.\n");
            writer.write("Email: test@example.com\n");
            writer.write("invalid-email@.com\n");
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + e.getMessage());
            return;
        }

        String[][] testCases = {
            {"world", "Test Case 1: Simple word match"},
            {"^[A-Z]", "Test Case 2: Match lines starting with a capital letter"},
            {"\\d{3}-\\d{3}-\\d{4}", "Test Case 3: Match a phone number format"},
            {"fox|dog", "Test Case 4: Match using alternation"},
            {"[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}", "Test Case 5: Match a valid email address"}
        };

        for (String[] testCase : testCases) {
            String pattern = testCase[0];
            String description = testCase[1];
            System.out.println("\n--- " + description + " ---");
            System.out.println("Pattern: " + pattern);
            System.out.println("------------------------------------");
            searchFileWithRegex(pattern, testFileName);
        }

        // Clean up the test file
        new File(testFileName).delete();
    }
}