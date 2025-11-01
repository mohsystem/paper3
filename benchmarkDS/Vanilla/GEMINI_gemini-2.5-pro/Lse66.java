import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;

public class Lse66 {

    /**
     * Simulates saving an uploaded PDF file to an 'uploads' directory.
     *
     * @param sourceFilePath The path to the source PDF file.
     * @return true if the file was saved successfully, false otherwise.
     */
    public static boolean savePdf(String sourceFilePath) {
        if (sourceFilePath == null || sourceFilePath.trim().isEmpty()) {
            System.err.println("Error: Source file path is empty.");
            return false;
        }

        Path sourcePath = Paths.get(sourceFilePath);
        File sourceFile = sourcePath.toFile();

        // 1. Check if source file exists and is a regular file
        if (!sourceFile.exists() || !sourceFile.isFile()) {
            System.err.println("Error: Source file does not exist or is not a regular file: " + sourceFilePath);
            return false;
        }

        // 2. Check if the file is a PDF (case-insensitive)
        if (!sourceFilePath.toLowerCase().endsWith(".pdf")) {
            System.err.println("Error: File is not a PDF: " + sourceFilePath);
            return false;
        }

        // 3. Create the 'uploads' directory if it doesn't exist
        String uploadDirName = "uploads";
        Path uploadDirPath = Paths.get(uploadDirName);
        if (!Files.exists(uploadDirPath)) {
            try {
                Files.createDirectories(uploadDirPath);
            } catch (IOException e) {
                System.err.println("Error: Could not create directory '" + uploadDirName + "'. " + e.getMessage());
                return false;
            }
        }

        // 4. Copy the file to the uploads directory
        try {
            Path destinationPath = uploadDirPath.resolve(sourcePath.getFileName());
            Files.copy(sourcePath, destinationPath, StandardCopyOption.REPLACE_EXISTING);
            System.out.println("Successfully saved '" + sourceFilePath + "' to '" + destinationPath + "'");
            return true;
        } catch (IOException e) {
            System.err.println("Error: Could not save file '" + sourceFilePath + "'. " + e.getMessage());
            return false;
        }
    }

    // Helper method to create dummy files for testing
    private static void createDummyFile(String fileName, String content) {
        try (FileWriter writer = new FileWriter(fileName)) {
            writer.write(content);
        } catch (IOException e) {
            System.err.println("Failed to create dummy file: " + fileName);
        }
    }

    public static void main(String[] args) {
        // Setup test environment by creating some dummy files
        System.out.println("--- Setting up test environment ---");
        createDummyFile("test1.pdf", "This is the content of PDF 1.");
        createDummyFile("test2.pdf", "This is the content of PDF 2.");
        createDummyFile("test3.txt", "This is a text file, not a PDF.");
        createDummyFile("test4.pdf", "This is PDF 4, used to test when the uploads dir already exists.");

        String[] testCases = {
            "test1.pdf",          // Case 1: Valid PDF file
            "test2.pdf",          // Case 2: Another valid PDF file
            "test3.txt",          // Case 3: Invalid file type (not a PDF)
            "non_existent.pdf",   // Case 4: File that does not exist
            "test4.pdf"           // Case 5: Valid PDF, where the 'uploads' directory already exists
        };
        
        System.out.println("\n--- Running Test Cases ---");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\nTest Case " + (i + 1) + ": Processing '" + testCases[i] + "'");
            boolean success = savePdf(testCases[i]);
            System.out.println("Result: " + (success ? "SUCCESS" : "FAILURE"));
        }
    }
}