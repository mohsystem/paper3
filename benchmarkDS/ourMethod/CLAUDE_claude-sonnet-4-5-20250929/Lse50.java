
import java.io.*;
import java.util.zip.*;
import java.nio.file.*;

public class Lse50 {
    // Maximum allowed file size: 100 MB to prevent zip bombs
    private static final long MAX_FILE_SIZE = 100 * 1024 * 1024;
    // Maximum number of files to prevent excessive resource usage
    private static final int MAX_FILES = 10000;
    // Base directory for extraction - must be absolute and validated
    private static final String BASE_DIR = "/tmp/unpack";
    
    public static void extractZipArchive(String zipFilePath) throws IOException {
        // Validate input path is not null or empty
        if (zipFilePath == null || zipFilePath.trim().isEmpty()) {
            throw new IllegalArgumentException("Zip file path cannot be null or empty");
        }
        
        // Normalize and validate the base directory
        Path baseDir = Paths.get(BASE_DIR).toAbsolutePath().normalize();
        Files.createDirectories(baseDir);
        
        int fileCount = 0;
        long totalSize = 0;
        
        // Open zip file with try-with-resources for automatic cleanup
        try (ZipInputStream zis = new ZipInputStream(
                new BufferedInputStream(new FileInputStream(zipFilePath)))) {
            
            ZipEntry entry;
            byte[] buffer = new byte[8192];
            
            while ((entry = zis.getNextEntry()) != null) {
                fileCount++;
                
                // Prevent zip bomb attacks by limiting file count
                if (fileCount > MAX_FILES) {
                    throw new IOException("Too many files in archive (limit: " + MAX_FILES + ")");
                }
                
                // Get the entry name and validate it
                String entryName = entry.getName();
                if (entryName == null || entryName.isEmpty()) {
                    continue;
                }
                
                // Normalize the entry path and resolve against base directory
                Path targetPath = baseDir.resolve(entryName).normalize();
                
                // Security check: ensure path stays within base directory (prevent directory traversal)
                if (!targetPath.startsWith(baseDir)) {
                    throw new IOException("Entry is outside of the target directory: " + entryName);
                }
                
                // Handle directories
                if (entry.isDirectory()) {
                    Files.createDirectories(targetPath);
                } else {
                    // Ensure parent directory exists
                    Path parentDir = targetPath.getParent();
                    if (parentDir != null) {
                        Files.createDirectories(parentDir);
                    }
                    
                    // Extract file with size validation to prevent zip bombs
                    try (OutputStream fos = new BufferedOutputStream(
                            Files.newOutputStream(targetPath, 
                                StandardOpenOption.CREATE, 
                                StandardOpenOption.TRUNCATE_EXISTING))) {
                        
                        int bytesRead;
                        long fileSize = 0;
                        
                        while ((bytesRead = zis.read(buffer)) != -1) {
                            fileSize += bytesRead;
                            totalSize += bytesRead;
                            
                            // Prevent zip bomb by checking individual file size
                            if (fileSize > MAX_FILE_SIZE) {
                                throw new IOException("File too large: " + entryName);
                            }
                            
                            // Prevent zip bomb by checking total extracted size
                            if (totalSize > MAX_FILE_SIZE * 10) {
                                throw new IOException("Total extracted size exceeds limit");
                            }
                            
                            fos.write(buffer, 0, bytesRead);
                        }
                        
                        // Ensure data is written to disk
                        fos.flush();
                    }
                    
                    // Set restrictive permissions (owner read/write only)
                    Files.setPosixFilePermissions(targetPath, 
                        java.nio.file.attribute.PosixFilePermissions.fromString("rw-------"));
                }
                
                zis.closeEntry();
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid zip file
        try {
            System.out.println("Test 1: Extracting valid archive");
            extractZipArchive("archive.zip");
            System.out.println("Test 1 passed");
        } catch (Exception e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Non-existent file
        try {
            System.out.println("\\nTest 2: Non-existent file");
            extractZipArchive("nonexistent.zip");
            System.out.println("Test 2 failed: should have thrown exception");
        } catch (Exception e) {
            System.out.println("Test 2 passed: " + e.getMessage());
        }
        
        // Test case 3: Null input
        try {
            System.out.println("\\nTest 3: Null input");
            extractZipArchive(null);
            System.out.println("Test 3 failed: should have thrown exception");
        } catch (Exception e) {
            System.out.println("Test 3 passed: " + e.getMessage());
        }
        
        // Test case 4: Empty string input
        try {
            System.out.println("\\nTest 4: Empty string input");
            extractZipArchive("");
            System.out.println("Test 4 failed: should have thrown exception");
        } catch (Exception e) {
            System.out.println("Test 4 passed: " + e.getMessage());
        }
        
        // Test case 5: Path with directory traversal attempt
        try {
            System.out.println("\\nTest 5: Malicious path (if archive contains traversal)");
            extractZipArchive("test_traversal.zip");
            System.out.println("Test 5: extraction completed with validation");
        } catch (Exception e) {
            System.out.println("Test 5: " + e.getMessage());
        }
    }
}
