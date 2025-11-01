
import java.io.*;
import java.nio.file.*;
import java.util.zip.*;

public class Lse49 {
    // Maximum file size to prevent zip bomb attacks (100MB)
    private static final long MAX_FILE_SIZE = 100L * 1024 * 1024;
    // Maximum total extracted size (500MB)
    private static final long MAX_TOTAL_SIZE = 500L * 1024 * 1024;
    // Maximum number of files to prevent resource exhaustion
    private static final int MAX_FILE_COUNT = 10000;
    // Base directory for extraction - must be absolute and normalized
    private static final String BASE_DIR = "/tmp/unpack";
    
    public static void unzipArchive(String zipFilePath) throws IOException {
        // Validate input path
        if (zipFilePath == null || zipFilePath.isEmpty()) {
            throw new IllegalArgumentException("Zip file path cannot be null or empty");
        }
        
        // Create and validate base directory
        Path baseDir = Paths.get(BASE_DIR).toAbsolutePath().normalize();
        Files.createDirectories(baseDir);
        
        long totalSize = 0;
        int fileCount = 0;
        
        // Open zip file and extract entries safely
        try (ZipInputStream zis = new ZipInputStream(
                new BufferedInputStream(new FileInputStream(zipFilePath)))) {
            
            ZipEntry entry;
            byte[] buffer = new byte[8192];
            
            while ((entry = zis.getNextEntry()) != null) {
                fileCount++;
                if (fileCount > MAX_FILE_COUNT) {
                    throw new IOException("Too many files in archive - potential zip bomb");
                }
                
                // Get entry name and validate it
                String entryName = entry.getName();
                if (entryName == null || entryName.isEmpty()) {
                    continue;
                }
                
                // Normalize the entry name and resolve against base directory
                Path targetPath = baseDir.resolve(entryName).normalize();
                
                // Security: Ensure the resolved path is within the base directory (path traversal prevention)
                if (!targetPath.startsWith(baseDir)) {
                    throw new IOException("Path traversal attempt detected: " + entryName);
                }
                
                // Handle directories
                if (entry.isDirectory()) {
                    Files.createDirectories(targetPath);
                    continue;
                }
                
                // Create parent directories for files
                Path parentPath = targetPath.getParent();
                if (parentPath != null) {
                    Files.createDirectories(parentPath);
                }
                
                // Extract file with size validation to prevent zip bombs
                long entrySize = 0;
                try (OutputStream fos = Files.newOutputStream(targetPath, 
                        StandardOpenOption.CREATE, 
                        StandardOpenOption.TRUNCATE_EXISTING)) {
                    
                    int bytesRead;
                    while ((bytesRead = zis.read(buffer)) != -1) {
                        entrySize += bytesRead;
                        totalSize += bytesRead;
                        
                        // Check individual file size
                        if (entrySize > MAX_FILE_SIZE) {
                            throw new IOException("File too large - potential zip bomb: " + entryName);
                        }
                        
                        // Check total extracted size
                        if (totalSize > MAX_TOTAL_SIZE) {
                            throw new IOException("Total extracted size exceeds limit - potential zip bomb");
                        }
                        
                        fos.write(buffer, 0, bytesRead);
                    }
                }
                
                zis.closeEntry();
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid zip file extraction
        try {
            System.out.println("Test 1: Extracting valid archive");
            unzipArchive("archive.zip");
            System.out.println("Test 1: SUCCESS");
        } catch (Exception e) {
            System.out.println("Test 1: FAILED - " + e.getMessage());
        }
        
        // Test case 2: Non-existent file
        try {
            System.out.println("\\nTest 2: Non-existent file");
            unzipArchive("nonexistent.zip");
            System.out.println("Test 2: FAILED - Should have thrown exception");
        } catch (Exception e) {
            System.out.println("Test 2: SUCCESS - Caught expected exception: " + e.getMessage());
        }
        
        // Test case 3: Null input
        try {
            System.out.println("\\nTest 3: Null input");
            unzipArchive(null);
            System.out.println("Test 3: FAILED - Should have thrown exception");
        } catch (Exception e) {
            System.out.println("Test 3: SUCCESS - Caught expected exception: " + e.getMessage());
        }
        
        // Test case 4: Empty string input
        try {
            System.out.println("\\nTest 4: Empty string input");
            unzipArchive("");
            System.out.println("Test 4: FAILED - Should have thrown exception");
        } catch (Exception e) {
            System.out.println("Test 4: SUCCESS - Caught expected exception: " + e.getMessage());
        }
        
        // Test case 5: Path with special characters (if file exists)
        try {
            System.out.println("\\nTest 5: Archive with special name");
            unzipArchive("test-archive_v1.0.zip");
            System.out.println("Test 5: SUCCESS");
        } catch (Exception e) {
            System.out.println("Test 5: FAILED - " + e.getMessage());
        }
    }
}
