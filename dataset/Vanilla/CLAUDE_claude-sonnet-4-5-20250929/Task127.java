
import java.io.*;
import java.nio.file.*;

public class Task127 {
    public static String readAndWriteFile(String sourceFilePath, String tempDirectory) {
        BufferedReader reader = null;
        BufferedWriter writer = null;
        String tempFilePath = null;
        
        try {
            // Validate input parameters
            if (sourceFilePath == null || sourceFilePath.trim().isEmpty()) {
                throw new IllegalArgumentException("Source file path cannot be null or empty");
            }
            if (tempDirectory == null || tempDirectory.trim().isEmpty()) {
                throw new IllegalArgumentException("Temp directory cannot be null or empty");
            }
            
            // Check if source file exists
            File sourceFile = new File(sourceFilePath);
            if (!sourceFile.exists()) {
                throw new FileNotFoundException("Source file does not exist: " + sourceFilePath);
            }
            if (!sourceFile.isFile()) {
                throw new IllegalArgumentException("Source path is not a file: " + sourceFilePath);
            }
            if (!sourceFile.canRead()) {
                throw new IOException("Cannot read source file: " + sourceFilePath);
            }
            
            // Create temp directory if it doesn't exist
            File tempDir = new File(tempDirectory);
            if (!tempDir.exists()) {
                if (!tempDir.mkdirs()) {
                    throw new IOException("Failed to create temp directory: " + tempDirectory);
                }
            }
            
            // Create temp file
            String fileName = sourceFile.getName();
            File tempFile = new File(tempDir, "temp_" + System.currentTimeMillis() + "_" + fileName);
            tempFilePath = tempFile.getAbsolutePath();
            
            // Read from source and write to temp
            reader = new BufferedReader(new FileReader(sourceFile));
            writer = new BufferedWriter(new FileWriter(tempFile));
            
            String line;
            while ((line = reader.readLine()) != null) {
                writer.write(line);
                writer.newLine();
            }
            
            return tempFilePath;
            
        } catch (FileNotFoundException e) {
            System.err.println("File not found error: " + e.getMessage());
            return null;
        } catch (IOException e) {
            System.err.println("IO error: " + e.getMessage());
            return null;
        } catch (IllegalArgumentException e) {
            System.err.println("Invalid argument: " + e.getMessage());
            return null;
        } catch (Exception e) {
            System.err.println("Unexpected error: " + e.getMessage());
            return null;
        } finally {
            // Close resources
            try {
                if (reader != null) {
                    reader.close();
                }
            } catch (IOException e) {
                System.err.println("Error closing reader: " + e.getMessage());
            }
            
            try {
                if (writer != null) {
                    writer.close();
                }
            } catch (IOException e) {
                System.err.println("Error closing writer: " + e.getMessage());
            }
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Testing File Read and Write with Exception Handling\\n");
        
        // Test Case 1: Valid file and temp directory
        System.out.println("Test Case 1: Valid file and temp directory");
        try {
            File testFile1 = new File("test1.txt");
            BufferedWriter writer = new BufferedWriter(new FileWriter(testFile1));
            writer.write("Hello World!\\nThis is test file 1.");
            writer.close();
            
            String result = readAndWriteFile("test1.txt", "./temp");
            System.out.println("Result: " + (result != null ? "Success - " + result : "Failed"));
            testFile1.delete();
        } catch (Exception e) {
            System.out.println("Setup error: " + e.getMessage());
        }
        System.out.println();
        
        // Test Case 2: Non-existent source file
        System.out.println("Test Case 2: Non-existent source file");
        String result2 = readAndWriteFile("nonexistent.txt", "./temp");
        System.out.println("Result: " + (result2 != null ? "Success - " + result2 : "Failed (Expected)"));
        System.out.println();
        
        // Test Case 3: Null source file path
        System.out.println("Test Case 3: Null source file path");
        String result3 = readAndWriteFile(null, "./temp");
        System.out.println("Result: " + (result3 != null ? "Success - " + result3 : "Failed (Expected)"));
        System.out.println();
        
        // Test Case 4: Empty source file path
        System.out.println("Test Case 4: Empty source file path");
        String result4 = readAndWriteFile("", "./temp");
        System.out.println("Result: " + (result4 != null ? "Success - " + result4 : "Failed (Expected)"));
        System.out.println();
        
        // Test Case 5: Valid file with special characters
        System.out.println("Test Case 5: Valid file with special characters");
        try {
            File testFile5 = new File("test5.txt");
            BufferedWriter writer = new BufferedWriter(new FileWriter(testFile5));
            writer.write("Special chars: @#$%^&*()\\nUnicode: \\u00A9 \\u00AE");
            writer.close();
            
            String result5 = readAndWriteFile("test5.txt", "./temp");
            System.out.println("Result: " + (result5 != null ? "Success - " + result5 : "Failed"));
            testFile5.delete();
        } catch (Exception e) {
            System.out.println("Setup error: " + e.getMessage());
        }
        System.out.println();
        
        // Cleanup
        try {
            File tempDir = new File("./temp");
            if (tempDir.exists()) {
                File[] files = tempDir.listFiles();
                if (files != null) {
                    for (File f : files) {
                        f.delete();
                    }
                }
                tempDir.delete();
            }
        } catch (Exception e) {
            System.err.println("Cleanup error: " + e.getMessage());
        }
    }
}
