import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task99 {

    /**
     * Reads a file and returns a list of lines that start with numbers and end with a dot.
     *
     * @param filePath The path to the file to read.
     * @return A list of matching lines.
     */
    public static List<String> findMatchingLines(String filePath) {
        List<String> matchingLines = new ArrayList<>();
        // Regex: ^\d+ -> starts with one or more digits
        //        .*   -> followed by any character, zero or more times
        //        \.   -> a literal dot
        //        $    -> end of the line
        String regex = "^\\d+.*\\.$";
        Pattern pattern = Pattern.compile(regex);

        // Using try-with-resources to ensure the reader is closed automatically
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            String line;
            while ((line = reader.readLine()) != null) {
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

    public static void main(String[] args) {
        String testFileName = "test_Task99.txt";

        // Create a test file with some content
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(testFileName))) {
            writer.write("1. This is the first test case.\n");
            writer.write("This line does not start with a number.\n");
            writer.write("2. Second test case.\n");
            writer.write("3 A line without a dot at the end\n");
            writer.write("4. Test case three is here.\n");
            writer.write("Another non-matching line.\n");
            writer.write("55. Test case four.\n");
            writer.write("99999. Final test case.\n");
            writer.write("The end.\n");
            writer.write("6 is not a match either\n");
        } catch (IOException e) {
            System.err.println("Error creating test file: " + e.getMessage());
            return;
        }

        System.out.println("--- Reading from file: " + testFileName + " ---");
        List<String> matched = findMatchingLines(testFileName);

        System.out.println("Found " + matched.size() + " matching lines:");
        for (String line : matched) {
            System.out.println(line);
        }

        // Clean up the test file
        new File(testFileName).delete();
    }
}