import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task99 {

    /**
     * Reads a file and finds lines that start with a number and end with a dot.
     * @param filePath The path to the input file.
     * @return A list of strings, where each string is a matching line.
     */
    public static List<String> findMatchingLines(String filePath) {
        List<String> matchingLines = new ArrayList<>();
        // Regex to match lines starting with a digit and ending with a literal dot.
        String regex = "^\\d.*\\.$";
        Pattern pattern = Pattern.compile(regex);

        try {
            List<String> allLines = Files.readAllLines(Paths.get(filePath));
            for (String line : allLines) {
                Matcher matcher = pattern.matcher(line);
                if (matcher.matches()) {
                    matchingLines.add(line);
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
        return matchingLines;
    }

    /**
     * Helper function to run a single test case.
     * @param fileName The name of the temporary file to create.
     * @param content The content to write to the file.
     */
    private static void runTestCase(String fileName, String content) {
        try {
            Path path = Paths.get(fileName);
            Files.write(path, content.getBytes());
            
            System.out.println("\n--- Testing with file: " + fileName + " ---");
            System.out.println("Content:\n\"" + content.replace("\n", "\\n") + "\"");
            
            List<String> result = findMatchingLines(fileName);
            System.out.println("Result:");
            if (result.isEmpty()) {
                System.out.println("(No matching lines found)");
            } else {
                result.forEach(System.out::println);
            }

            Files.delete(path);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        // Test Case 1: A file with a mix of valid and invalid lines.
        runTestCase("test_java_1.txt", 
                    "1. This is a valid line.\n" +
                    "This line is not valid.\n" +
                    "2. This is also a valid line.\n" +
                    "3 This line is not valid, no dot at the end\n" +
                    "4.Valid line.\n" +
                    "5.\n" +
                    "Invalid line.\n" +
                    "6. Another. valid. line.");

        // Test Case 2: An empty file.
        runTestCase("test_java_2.txt", "");

        // Test Case 3: A file with no matching lines.
        runTestCase("test_java_3.txt", "Hello world\nThis is a test\nNo lines match here");

        // Test Case 4: A file where all lines match.
        runTestCase("test_java_4.txt", "1. First.\n2. Second.\n3. Third.");

        // Test Case 5: A file with special characters and multiple digits.
        runTestCase("test_java_5.txt", "123. Special chars !@#$%^&*().\nAnother line\n45.Ends with a dot.");
    }
}