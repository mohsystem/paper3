
import java.io.*;
import java.nio.file.*;
import java.util.*;

public class Task121 {
    private static final String UPLOAD_DIRECTORY = "uploads";
    
    public static boolean uploadFile(String sourceFilePath, String destinationFileName) {
        try {
            // Create upload directory if it doesn't exist\n            File uploadDir = new File(UPLOAD_DIRECTORY);\n            if (!uploadDir.exists()) {\n                uploadDir.mkdirs();\n            }\n            \n            // Read source file\n            File sourceFile = new File(sourceFilePath);\n            if (!sourceFile.exists()) {\n                System.out.println("Source file does not exist: " + sourceFilePath);\n                return false;\n            }\n            \n            // Copy file to upload directory\n            Path source = sourceFile.toPath();\n            Path destination = Paths.get(UPLOAD_DIRECTORY, destinationFileName);\n            Files.copy(source, destination, StandardCopyOption.REPLACE_EXISTING);\n            \n            System.out.println("File uploaded successfully: " + destinationFileName);\n            return true;\n        } catch (IOException e) {\n            System.out.println("Error uploading file: " + e.getMessage());\n            return false;\n        }\n    }\n    \n    public static boolean uploadFileWithContent(String fileName, String content) {\n        try {\n            // Create upload directory if it doesn't exist
            File uploadDir = new File(UPLOAD_DIRECTORY);
            if (!uploadDir.exists()) {
                uploadDir.mkdirs();
            }
            
            // Write content to file
            Path destination = Paths.get(UPLOAD_DIRECTORY, fileName);
            Files.write(destination, content.getBytes(), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            
            System.out.println("File uploaded successfully: " + fileName);
            return true;
        } catch (IOException e) {
            System.out.println("Error uploading file: " + e.getMessage());
            return false;
        }
    }
    
    public static List<String> listUploadedFiles() {
        List<String> files = new ArrayList<>();
        File uploadDir = new File(UPLOAD_DIRECTORY);
        
        if (uploadDir.exists() && uploadDir.isDirectory()) {
            File[] fileList = uploadDir.listFiles();
            if (fileList != null) {
                for (File file : fileList) {
                    if (file.isFile()) {
                        files.add(file.getName());
                    }
                }
            }
        }
        
        return files;
    }
    
    public static void main(String[] args) {
        System.out.println("=== File Upload Program ===\\n");
        
        // Test Case 1: Upload a file with content
        System.out.println("Test Case 1: Upload text file");
        boolean result1 = uploadFileWithContent("test1.txt", "Hello, this is test file 1!");
        System.out.println("Result: " + result1 + "\\n");
        
        // Test Case 2: Upload another file with content
        System.out.println("Test Case 2: Upload document");
        boolean result2 = uploadFileWithContent("document.txt", "This is a sample document with multiple lines.\\nLine 2\\nLine 3");
        System.out.println("Result: " + result2 + "\\n");
        
        // Test Case 3: Upload a file with JSON content
        System.out.println("Test Case 3: Upload JSON file");
        boolean result3 = uploadFileWithContent("data.json", "{\\"name\\":\\"John\\",\\"age\\":30,\\"city\\":\\"New York\\"}");
        System.out.println("Result: " + result3 + "\\n");
        
        // Test Case 4: Upload a file with special characters
        System.out.println("Test Case 4: Upload file with special characters");
        boolean result4 = uploadFileWithContent("special.txt", "Special chars: !@#$%^&*()");
        System.out.println("Result: " + result4 + "\\n");
        
        // Test Case 5: List all uploaded files
        System.out.println("Test Case 5: List all uploaded files");
        List<String> uploadedFiles = listUploadedFiles();
        System.out.println("Uploaded files:");
        for (String file : uploadedFiles) {
            System.out.println("  - " + file);
        }
    }
}
