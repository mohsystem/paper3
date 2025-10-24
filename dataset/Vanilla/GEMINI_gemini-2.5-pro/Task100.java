import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;
import java.util.stream.Stream;

public class Task100 {

    /**
     * Searches for a regex pattern within a file and prints matching lines.
     * @param regexPattern The regular expression pattern to search for.
     * @param fileName The path to the file to search in.
     */
    public static void searchInFile(String regexPattern, String fileName) {
        System.out.println("Searching for pattern '" + regexPattern + "' in file '" + fileName + "'...");
        try {
            Pattern pattern = Pattern.compile(regexPattern);
            try (Stream<String> lines = Files.lines(Paths.get(fileName))) {
                lines.forEach(line -> {
                    Matcher matcher = pattern.matcher(line);
                    if (matcher.find()) {
                        System.out.println("Found match: " + line);
                    }
                });
            } catch (IOException e) {
                System.err.println("Error reading file: " + e.getMessage());
            }
        } catch (PatternSyntaxException e) {
            System.err.println("Invalid regular expression: " + e.getMessage());
        }
        System.out.println("------------------------------------");
    }

    /**
     * Sets up test data and runs test cases.
     */
    public static void runTestCases() {
        String testFileName = "test_file_java.txt";
        try (FileWriter writer = new FileWriter(testFileName)) {
            writer.write("Hello world, this is a test file.\n");
            writer.write("The year is 2024.\n");
            writer.write("Contact us at test@example.com for more info.\n");
            writer.write("Or call 123-456-7890.\n");
            writer.write("This line contains no special characters or numbers\n");
            writer.write("Another email: another.email@domain.org.\n");
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + e.getMessage());
            return;
        }

        // Test Case 1: Match a 4-digit number
        searchInFile("\\d{4}", testFileName);

        // Test Case 2: Match an email address
        searchInFile("\\w+@\\w+\\.\\w+", testFileName);

        // Test Case 3: Match a line starting with "Hello"
        searchInFile("^Hello", testFileName);

        // Test Case 4: Match a line ending with "numbers"
        searchInFile("numbers$", testFileName);

        // Test Case 5: A pattern that won't match anything
        searchInFile("NonExistentPatternXYZ", testFileName);

        // Cleanup the test file
        new File(testFileName).delete();
    }

    public static void main(String[] args) {
        if (args.length == 2) {
            // Use command-line arguments
            String regexPattern = args[0];
            String fileName = args[1];
            searchInFile(regexPattern, fileName);
        } else {
            // Run built-in test cases
            System.out.println("Usage: java Task100 <regex_pattern> <file_name>");
            System.out.println("Running test cases instead...\n");
            runTestCases();
        }
    }
}