
import java.io.*;
import java.util.regex.*;
import java.util.*;

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
        // Test cases - creating test files
        String[] testFiles = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
        
        // Test case 1: Simple numbered list
        try (PrintWriter writer = new PrintWriter(testFiles[0])) {
            writer.println("1 This is a sentence.");
            writer.println("2 Another line.");
            writer.println("Not matching");
            writer.println("3 Third line.");
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 2: Multi-digit numbers
        try (PrintWriter writer = new PrintWriter(testFiles[1])) {
            writer.println("123 Large number line.");
            writer.println("456 Another large number.");
            writer.println("No number here.");
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 3: Lines without dots
        try (PrintWriter writer = new PrintWriter(testFiles[2])) {
            writer.println("1 This has no dot");
            writer.println("2 This has a dot.");
            writer.println("3 No dot here");
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 4: Mixed content
        try (PrintWriter writer = new PrintWriter(testFiles[3])) {
            writer.println("99 Special characters !@#$.");
            writer.println("Start with text 100.");
            writer.println("0 Zero starts this.");
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 5: Empty and edge cases
        try (PrintWriter writer = new PrintWriter(testFiles[4])) {
            writer.println("1.");
            writer.println("2");
            writer.println(".");
            writer.println("12345 Multiple words here.");
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Execute test cases
        for (int i = 0; i < testFiles.length; i++) {
            System.out.println("Test case " + (i + 1) + " - File: " + testFiles[i]);
            List<String> results = matchLines(testFiles[i]);
            System.out.println("Matched lines: " + results);
            System.out.println();
        }
    }
}
