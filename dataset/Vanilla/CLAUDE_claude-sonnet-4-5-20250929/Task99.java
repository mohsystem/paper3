
import java.io.*;
import java.util.regex.*;
import java.util.ArrayList;
import java.util.List;

public class Task99 {
    public static List<String> matchLines(String filename) {
        List<String> matchedLines = new ArrayList<>();
        Pattern pattern = Pattern.compile("^\\\\d+.*\\\\.$");
        
        try (BufferedReader br = new BufferedReader(new FileReader(filename))) {
            String line;
            while ((line = br.readLine()) != null) {
                Matcher matcher = pattern.matcher(line);
                if (matcher.matches()) {
                    matchedLines.add(line);
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
        
        return matchedLines;
    }
    
    public static void main(String[] args) {
        // Create test files for demonstration
        String[] testFiles = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
        String[][] testData = {
            {"123 This is a test.", "456 Another line.", "Not matching", "789 End with dot."},
            {"1 First line.", "Second line", "2 Third line.", "3 Fourth."},
            {"99 Single digit.", "100 Triple digits.", "No number here.", "5."},
            {"0 Zero start.", "abc", "42 The answer.", ""},
            {"7777 Multiple digits here.", "8888.", "Not starting with number.", "9 Final."}
        };
        
        // Create test files
        for (int i = 0; i < testFiles.length; i++) {
            try (PrintWriter writer = new PrintWriter(new FileWriter(testFiles[i]))) {
                for (String line : testData[i]) {
                    writer.println(line);
                }
            } catch (IOException e) {
                System.err.println("Error creating test file: " + e.getMessage());
            }
        }
        
        // Test cases
        for (int i = 0; i < testFiles.length; i++) {
            System.out.println("Test Case " + (i + 1) + " - File: " + testFiles[i]);
            List<String> results = matchLines(testFiles[i]);
            System.out.println("Matched lines:");
            for (String line : results) {
                System.out.println("  " + line);
            }
            System.out.println();
        }
    }
}
