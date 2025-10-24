import java.io.File;
import java.io.IOException;
import java.nio.file.*;
import java.util.Arrays;
import java.util.List;

public class Task121 {

    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
    private static final List<String> ALLOWED_EXTENSIONS = Arrays.asList(".txt", ".jpg", ".png", ".pdf");

    /**
     * Securely uploads a file by copying it from a source path to a destination directory
     * with several security checks.
     *
     * @param sourcePathStr The path to the source file.
     * @param destDirStr    The path to the destination directory.
     * @return true if the upload was successful, false otherwise.
     */
    public static boolean secureFileUpload(String sourcePathStr, String destDirStr) {
        Path sourcePath;
        Path destDirPath;
        try {
            sourcePath = Paths.get(sourcePathStr);
            destDirPath = Paths.get(destDirStr);
        } catch (InvalidPathException e) {
            System.err.println("Error: Invalid path provided. " + e.getMessage());
            return false;
        }

        // 1. Source File Checks
        if (!Files.exists(sourcePath) || !Files.isRegularFile(sourcePath) || !Files.isReadable(sourcePath)) {
            System.err.println("Error: Source file is not a valid, readable file.");
            return false;
        }

        // 2. Destination Directory Checks
        if (!Files.exists(destDirPath) || !Files.isDirectory(destDirPath) || !Files.isWritable(destDirPath)) {
            System.err.println("Error: Destination is not a valid, writable directory.");
            return false;
        }

        // 3. File Size Check
        try {
            long fileSize = Files.size(sourcePath);
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: File size exceeds the maximum limit of " + (MAX_FILE_SIZE / (1024 * 1024)) + " MB.");
                return false;
            }
        } catch (IOException e) {
            System.err.println("Error: Could not determine file size. " + e.getMessage());
            return false;
        }

        // 4. Filename and Extension Validation
        Path fileNamePath = sourcePath.getFileName();
        if (fileNamePath == null) {
            System.err.println("Error: Could not extract filename from source path.");
            return false;
        }
        String fileName = fileNamePath.toString();
        
        // Sanitize filename to prevent including path characters
        String sanitizedFileName = fileName.replaceAll("[\\\\/]", "");
        if (!fileName.equals(sanitizedFileName)) {
            System.err.println("Error: Filename contains invalid path characters.");
            return false;
        }

        boolean extensionAllowed = ALLOWED_EXTENSIONS.stream().anyMatch(sanitizedFileName.toLowerCase()::endsWith);
        if (!extensionAllowed) {
            System.err.println("Error: File extension is not allowed.");
            return false;
        }
        
        // 5. Path Traversal Check
        Path destFilePath = destDirPath.resolve(sanitizedFileName);
        
        try {
            Path canonicalDestDir = destDirPath.toRealPath(); // Canonicalize the intended directory
            Path finalDestPath = destFilePath.toAbsolutePath().normalize();

            // The parent of the final destination path must be the canonical destination directory.
            // This is a crucial check to prevent path traversal attacks like 'uploads/../'.
            if (!finalDestPath.getParent().equals(canonicalDestDir)) {
                System.err.println("Error: Path traversal attempt detected.");
                return false;
            }
        } catch (IOException e) {
            System.err.println("Error: Could not resolve destination path. " + e.getMessage());
            return false;
        }

        // 6. Check for existing file (to prevent overwrite)
        if (Files.exists(destFilePath)) {
            System.err.println("Error: File with the same name already exists in the destination.");
            return false;
        }

        // 7. Perform the copy
        try {
            Files.copy(sourcePath, destFilePath, StandardCopyOption.COPY_ATTRIBUTES);
            System.out.println("Success: File '" + sanitizedFileName + "' uploaded successfully.");
            return true;
        } catch (IOException e) {
            System.err.println("Error: File copy failed. " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) throws IOException {
        // Setup for tests
        String uploadDir = "uploads_java";
        Files.createDirectories(Paths.get(uploadDir));
        
        // Create test files
        Path goodFile = Paths.get("test_good.txt");
        Files.write(goodFile, "This is a good file.".getBytes());

        Path largeFile = Paths.get("test_large.bin");
        // Create a file slightly larger than the limit
        Files.write(largeFile, new byte[(int)MAX_FILE_SIZE + 1]);

        Path badExtFile = Paths.get("test_bad.exe");
        Files.write(badExtFile, "dummy executable".getBytes());

        Path traversalFile = Paths.get("traversal_test.txt");
        Files.write(traversalFile, "This file should not escape the upload dir".getBytes());


        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Successful upload
        System.out.println("\n[Test 1] Successful Upload:");
        secureFileUpload(goodFile.toString(), uploadDir);

        // Test Case 2: File already exists (Overwrite prevention)
        System.out.println("\n[Test 2] File Exists:");
        secureFileUpload(goodFile.toString(), uploadDir); 

        // Test Case 3: File too large
        System.out.println("\n[Test 3] File Too Large:");
        secureFileUpload(largeFile.toString(), uploadDir);

        // Test Case 4: Disallowed extension
        System.out.println("\n[Test 4] Disallowed Extension:");
        secureFileUpload(badExtFile.toString(), uploadDir);

        // Test Case 5: Path Traversal Attempt
        System.out.println("\n[Test 5] Path Traversal Attempt:");
        String maliciousDestDir = uploadDir + File.separator + ".." + File.separator;
        secureFileUpload(traversalFile.toString(), maliciousDestDir);
        
        // Cleanup test files
        Files.deleteIfExists(goodFile);
        Files.deleteIfExists(largeFile);
        Files.deleteIfExists(badExtFile);
        Files.deleteIfExists(traversalFile);
    }
}