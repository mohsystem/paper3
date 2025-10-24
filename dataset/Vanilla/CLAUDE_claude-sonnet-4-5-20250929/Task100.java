
import java.io.*;
import java.util.regex.*;
import java.nio.file.*;

public class Task100 {
    public static void searchPatternInFile(String patternStr, String fileName) {
        try {
            Pattern pattern = Pattern.compile(patternStr);
            String content = new String(Files.readAllBytes(Paths.get(fileName)));
            Matcher matcher = pattern.matcher(content);
            
            System.out.println("Searching for pattern: " + patternStr + " in file: " + fileName);
            int count = 0;
            while (matcher.find()) {
                count++;
                System.out.println("Match " + count + ": " + matcher.group() + " at position " + matcher.start());
            }
            
            if (count == 0) {
                System.out.println("No matches found.");
            } else {
                System.out.println("Total matches: " + count);
            }
        } catch (PatternSyntaxException e) {
            System.out.println("Invalid regex pattern: " + e.getMessage());
        } catch (IOException e) {
            System.out.println("Error reading file: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        if (args.length >= 2) {
            searchPatternInFile(args[0], args[1]);
        } else {
            System.out.println("Running test cases...\\n");
            
            // Create test files
            try {
                Files.write(Paths.get("test1.txt"), "Hello World! Hello Java!".getBytes());
                Files.write(Paths.get("test2.txt"), "abc123def456ghi789".getBytes());
                Files.write(Paths.get("test3.txt"), "email@example.com test@domain.org".getBytes());
                Files.write(Paths.get("test4.txt"), "The quick brown fox jumps over the lazy dog".getBytes());
                Files.write(Paths.get("test5.txt"), "Line1\\nLine2\\nLine3".getBytes());
                
                // Test case 1
                System.out.println("Test 1:");
                searchPatternInFile("Hello", "test1.txt");
                System.out.println();
                
                // Test case 2
                System.out.println("Test 2:");
                searchPatternInFile("\\\\d+", "test2.txt");
                System.out.println();
                
                // Test case 3
                System.out.println("Test 3:");
                searchPatternInFile("[a-z]+@[a-z]+\\\\.[a-z]+", "test3.txt");
                System.out.println();
                
                // Test case 4
                System.out.println("Test 4:");
                searchPatternInFile("\\\\b\\\\w{3}\\\\b", "test4.txt");
                System.out.println();
                
                // Test case 5
                System.out.println("Test 5:");
                searchPatternInFile("Line\\\\d", "test5.txt");
                System.out.println();
                
            } catch (IOException e) {
                System.out.println("Error creating test files: " + e.getMessage());
            }
        }
    }
}
