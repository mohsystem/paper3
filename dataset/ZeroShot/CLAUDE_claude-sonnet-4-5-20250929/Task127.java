
import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.UUID;

public class Task127 {
    private static final String TEMP_DIR = System.getProperty("java.io.tmpdir");
    
    public static String readAndWriteFile(String sourceFilePath) {
        if (sourceFilePath == null || sourceFilePath.trim().isEmpty()) {
            throw new IllegalArgumentException("Source file path cannot be null or empty");
        }
        
        File sourceFile = new File(sourceFilePath);
        String tempFilePath = null;
        BufferedReader reader = null;
        BufferedWriter writer = null;
        
        try {
            // Validate source file exists and is readable
            if (!sourceFile.exists()) {
                throw new FileNotFoundException("Source file does not exist: " + sourceFilePath);
            }
            
            if (!sourceFile.isFile()) {
                throw new IllegalArgumentException("Path is not a file: " + sourceFilePath);
            }
            
            if (!sourceFile.canRead()) {
                throw new SecurityException("Cannot read source file: " + sourceFilePath);
            }
            
            // Create secure temporary file
            String uniqueId = UUID.randomUUID().toString();
            tempFilePath = TEMP_DIR + File.separator + "temp_" + uniqueId + ".txt";
            File tempFile = new File(tempFilePath);
            
            // Read from source file
            reader = new BufferedReader(new FileReader(sourceFile));
            writer = new BufferedWriter(new FileWriter(tempFile));
            
            String line;
            while ((line = reader.readLine()) != null) {
                writer.write(line);
                writer.newLine();
            }
            
            return tempFilePath;
            
        } catch (FileNotFoundException e) {
            System.err.println("Error: File not found - " + e.getMessage());
            return null;
        } catch (SecurityException e) {
            System.err.println("Error: Security exception - " + e.getMessage());
            return null;
        } catch (IOException e) {
            System.err.println("Error: I/O exception - " + e.getMessage());
            return null;
        } catch (Exception e) {
            System.err.println("Error: Unexpected exception - " + e.getMessage());
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
        System.out.println("=== File Read/Write Error Handling Test Cases ===\\n");
        
        // Test Case 1: Create a valid test file and read it
        try {
            String testFile1 = TEMP_DIR + File.separator + "test_input_1.txt";
            FileWriter fw = new FileWriter(testFile1);
            fw.write("This is test content line 1\\n");
            fw.write("This is test content line 2\\n");
            fw.close();
            
            System.out.println("Test Case 1: Valid file");
            String result = readAndWriteFile(testFile1);
            System.out.println("Result: " + (result != null ? "Success - " + result : "Failed"));
            System.out.println();
        } catch (Exception e) {
            System.out.println("Test Case 1 setup failed: " + e.getMessage() + "\\n");
        }
        
        // Test Case 2: Non-existent file
        System.out.println("Test Case 2: Non-existent file");
        String result2 = readAndWriteFile("/non/existent/file.txt");
        System.out.println("Result: " + (result2 != null ? "Success" : "Failed as expected"));
        System.out.println();
        
        // Test Case 3: Null file path
        System.out.println("Test Case 3: Null file path");
        try {
            String result3 = readAndWriteFile(null);
            System.out.println("Result: Failed");
        } catch (IllegalArgumentException e) {
            System.out.println("Result: Exception caught as expected - " + e.getMessage());
        }
        System.out.println();
        
        // Test Case 4: Empty file path
        System.out.println("Test Case 4: Empty file path");
        try {
            String result4 = readAndWriteFile("");
            System.out.println("Result: Failed");
        } catch (IllegalArgumentException e) {
            System.out.println("Result: Exception caught as expected - " + e.getMessage());
        }
        System.out.println();
        
        // Test Case 5: Directory instead of file
        System.out.println("Test Case 5: Directory instead of file");
        String result5 = readAndWriteFile(TEMP_DIR);
        System.out.println("Result: " + (result5 != null ? "Success" : "Failed as expected"));
        System.out.println();
    }
}
