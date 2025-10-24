
import java.io.*;
import java.util.Scanner;

public class Task39 {
    public static void displayFileContents(String filename) {
        try {
            String os = System.getProperty("os.name").toLowerCase();
            String command;
            
            if (os.contains("win")) {
                command = "cmd /c type \\"" + filename + "\\"";
            } else {
                command = "cat \\"" + filename + "\\"";
            }
            
            Process process = Runtime.getRuntime().exec(command);
            
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            
            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }
            
            while ((line = errorReader.readLine()) != null) {
                System.err.println(line);
            }
            
            process.waitFor();
            reader.close();
            errorReader.close();
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Create test files
        String[] testFiles = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
        String[] testContents = {
            "Hello World!",
            "This is test file 2.\\nWith multiple lines.",
            "Test file 3 contents",
            "Line 1\\nLine 2\\nLine 3",
            "Final test file"
        };
        
        // Create test files
        for (int i = 0; i < testFiles.length; i++) {
            try {
                FileWriter writer = new FileWriter(testFiles[i]);
                writer.write(testContents[i]);
                writer.close();
            } catch (IOException e) {
                System.err.println("Error creating test file: " + e.getMessage());
            }
        }
        
        // Test cases
        for (int i = 0; i < testFiles.length; i++) {
            System.out.println("\\n--- Test Case " + (i + 1) + ": " + testFiles[i] + " ---");
            displayFileContents(testFiles[i]);
        }
        
        // Clean up test files
        for (String file : testFiles) {
            new File(file).delete();
        }
    }
}
