
import java.io.*;
import java.nio.file.*;
import java.util.zip.*;
import org.apache.commons.compress.archivers.tar.*;
import org.apache.commons.compress.compressors.gzip.*;
import org.apache.commons.compress.compressors.bzip2.*;

public class Lse51 {
    private static final Path BASE_DIR = Paths.get("/tmp/unpack").toAbsolutePath().normalize();
    private static final long MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB max per file
    private static final long MAX_TOTAL_SIZE = 500 * 1024 * 1024; // 500MB max total
    private static final int MAX_FILES = 10000;
    
    // Secure tar extraction preventing path traversal and resource exhaustion
    public static void extractTarArchive(String tarFilePath, String compressionType) throws IOException {
        // Validate input parameters - reject null or empty strings
        if (tarFilePath == null || tarFilePath.isEmpty()) {
            throw new IllegalArgumentException("Invalid tar file path");
        }
        if (compressionType == null) {
            compressionType = "none";
        }
        
        // Normalize and validate the tar file path exists within expected bounds
        Path tarPath = Paths.get(tarFilePath).toAbsolutePath().normalize();
        if (!Files.exists(tarPath) || !Files.isRegularFile(tarPath)) {
            throw new IOException("Tar file does not exist or is not a regular file");
        }
        
        // Create base directory with restrictive permissions if it doesn't exist\n        if (!Files.exists(BASE_DIR)) {\n            Files.createDirectories(BASE_DIR);\n            // Set restrictive permissions (owner read/write/execute only)\n            try {\n                Files.setPosixFilePermissions(BASE_DIR, \n                    java.nio.file.attribute.PosixFilePermissions.fromString("rwx------"));\n            } catch (UnsupportedOperationException e) {\n                // Windows doesn't support POSIX permissions, continue
            }
        }
        
        long totalBytesExtracted = 0;
        int fileCount = 0;
        
        // Use try-with-resources to ensure proper resource cleanup
        try (InputStream fis = Files.newInputStream(tarPath);
             InputStream bis = new BufferedInputStream(fis);
             InputStream compressedStream = getCompressedStream(bis, compressionType);
             TarArchiveInputStream tarInput = new TarArchiveInputStream(compressedStream, StandardCharsets.UTF_8.name())) {
            
            TarArchiveEntry entry;
            
            while ((entry = tarInput.getNextTarEntry()) != null) {
                // Limit number of files to prevent zip bomb attacks
                if (++fileCount > MAX_FILES) {
                    throw new IOException("Archive contains too many files");
                }
                
                // Skip non-regular files (directories, symlinks, devices)
                if (!entry.isFile()) {
                    continue;
                }
                
                // Validate entry size to prevent resource exhaustion
                long entrySize = entry.getSize();
                if (entrySize < 0 || entrySize > MAX_FILE_SIZE) {
                    throw new IOException("Entry size exceeds maximum allowed");
                }
                
                // Check total extraction size
                if (totalBytesExtracted + entrySize > MAX_TOTAL_SIZE) {
                    throw new IOException("Total extraction size exceeds maximum allowed");
                }
                
                // Sanitize entry name and prevent path traversal
                String entryName = entry.getName();
                if (entryName == null || entryName.isEmpty()) {
                    continue;
                }
                
                // Remove leading slashes and normalize path
                entryName = entryName.replaceAll("^\\\\.+[\\\\\\\\/]", "").replaceAll("^[\\\\\\\\/]+", "");
                Path targetPath = BASE_DIR.resolve(entryName).normalize();
                
                // Ensure resolved path is still within BASE_DIR (prevent path traversal)
                if (!targetPath.startsWith(BASE_DIR)) {
                    throw new IOException("Entry attempts path traversal: " + entryName);
                }
                
                // Create parent directories with restrictive permissions
                Path parentDir = targetPath.getParent();
                if (parentDir != null && !Files.exists(parentDir)) {
                    Files.createDirectories(parentDir);
                }
                
                // Extract file securely with size validation
                long bytesWritten = 0;
                byte[] buffer = new byte[8192];
                
                // Use try-with-resources for file output stream
                try (OutputStream fos = Files.newOutputStream(targetPath, 
                        StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
                     BufferedOutputStream bos = new BufferedOutputStream(fos)) {
                    
                    int bytesRead;
                    while ((bytesRead = tarInput.read(buffer)) != -1) {
                        // Validate we don't exceed declared entry size\n                        if (bytesWritten + bytesRead > entrySize) {\n                            throw new IOException("Entry size mismatch");\n                        }\n                        bos.write(buffer, 0, bytesRead);\n                        bytesWritten += bytesRead;\n                    }\n                    \n                    // Ensure data is written to disk\n                    bos.flush();\n                    fos.getFD().sync();\n                }\n                \n                // Set restrictive permissions on extracted file\n                try {\n                    Files.setPosixFilePermissions(targetPath,\n                        java.nio.file.attribute.PosixFilePermissions.fromString("rw-------"));\n                } catch (UnsupportedOperationException e) {\n                    // Windows doesn't support POSIX permissions
                }
                
                totalBytesExtracted += bytesWritten;
            }
        }
    }
    
    // Helper to wrap stream with appropriate decompressor
    private static InputStream getCompressedStream(InputStream input, String type) throws IOException {
        switch (type.toLowerCase()) {
            case "gzip":
            case "gz":
                return new GzipCompressorInputStream(input);
            case "bzip2":
            case "bz2":
                return new BZip2CompressorInputStream(input);
            case "none":
            default:
                return input;
        }
    }
    
    // Secure file reading with validation
    public static String readFile(String filePath) throws IOException {
        // Validate input
        if (filePath == null || filePath.isEmpty()) {
            throw new IllegalArgumentException("Invalid file path");
        }
        
        // Normalize and validate path is within BASE_DIR
        Path path = BASE_DIR.resolve(filePath).normalize();
        if (!path.startsWith(BASE_DIR)) {
            throw new IOException("Path traversal attempt detected");
        }
        
        // Open and validate the file handle before reading
        if (!Files.exists(path) || !Files.isRegularFile(path)) {
            throw new IOException("File does not exist or is not a regular file");
        }
        
        long fileSize = Files.size(path);
        if (fileSize > MAX_FILE_SIZE) {
            throw new IOException("File size exceeds maximum");
        }
        
        // Use try-with-resources for automatic resource management
        try (BufferedReader reader = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
            StringBuilder content = new StringBuilder();
            String line;
            long bytesRead = 0;
            
            while ((line = reader.readLine()) != null) {
                bytesRead += line.length();
                if (bytesRead > MAX_FILE_SIZE) {
                    throw new IOException("File content exceeds maximum size");
                }
                content.append(line).append(System.lineSeparator());
            }
            
            return content.toString();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Extract uncompressed tar (would need actual tar file)
        try {
            System.out.println("Test 1: Extract uncompressed tar");
            // extractTarArchive("/path/to/archive.tar", "none");
            System.out.println("Note: Requires actual tar file to test");
        } catch (Exception e) {
            System.err.println("Test 1 error: " + e.getMessage());
        }
        
        // Test case 2: Extract gzip compressed tar
        try {
            System.out.println("\\nTest 2: Extract gzip tar");
            // extractTarArchive("/path/to/archive.tar.gz", "gzip");
            System.out.println("Note: Requires actual tar.gz file to test");
        } catch (Exception e) {
            System.err.println("Test 2 error: " + e.getMessage());
        }
        
        // Test case 3: Extract bzip2 compressed tar
        try {
            System.out.println("\\nTest 3: Extract bzip2 tar");
            // extractTarArchive("/path/to/archive.tar.bz2", "bzip2");
            System.out.println("Note: Requires actual tar.bz2 file to test");
        } catch (Exception e) {
            System.err.println("Test 3 error: " + e.getMessage());
        }
        
        // Test case 4: Read extracted file
        try {
            System.out.println("\\nTest 4: Read file");
            // String content = readFile("example.txt");
            // System.out.println("Content: " + content);
            System.out.println("Note: Requires extracted file to test");
        } catch (Exception e) {
            System.err.println("Test 4 error: " + e.getMessage());
        }
        
        // Test case 5: Invalid path (path traversal attempt)
        try {
            System.out.println("\\nTest 5: Path traversal prevention");
            readFile("../../etc/passwd");
            System.out.println("FAIL: Should have thrown exception");
        } catch (IOException e) {
            System.out.println("PASS: Path traversal blocked - " + e.getMessage());
        }
    }
}
