
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HexFormat;

public class Task127 {
    private static final int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    private static final int BUFFER_SIZE = 8192;

    public static String copyFileToTemp(String baseDir, String relativeFilePath) throws IOException {
        if (baseDir == null || baseDir.trim().isEmpty()) {
            throw new IllegalArgumentException("Base directory cannot be null or empty");
        }
        if (relativeFilePath == null || relativeFilePath.trim().isEmpty()) {
            throw new IllegalArgumentException("Relative file path cannot be null or empty");
        }

        Path basePath = Paths.get(baseDir).toAbsolutePath().normalize();
        if (!Files.exists(basePath) || !Files.isDirectory(basePath)) {
            throw new IllegalArgumentException("Base directory does not exist or is not a directory");
        }

        Path targetPath = basePath.resolve(relativeFilePath).normalize();
        if (!targetPath.startsWith(basePath)) {
            throw new SecurityException("Path traversal attempt detected");
        }

        if (!Files.exists(targetPath)) {
            throw new FileNotFoundException("Source file does not exist: " + targetPath);
        }
        if (!Files.isRegularFile(targetPath, LinkOption.NOFOLLOW_LINKS)) {
            throw new IllegalArgumentException("Source path is not a regular file");
        }
        if (!Files.isReadable(targetPath)) {
            throw new IOException("Source file is not readable");
        }

        long fileSize = Files.size(targetPath);
        if (fileSize > MAX_FILE_SIZE) {
            throw new IOException("File size exceeds maximum allowed size");
        }

        Path tempFile = null;
        try {
            tempFile = Files.createTempFile("secure_copy_", ".tmp");
            Files.setPosixFilePermissions(tempFile, 
                java.util.EnumSet.of(
                    java.nio.file.attribute.PosixFilePermission.OWNER_READ,
                    java.nio.file.attribute.PosixFilePermission.OWNER_WRITE
                ));
        } catch (UnsupportedOperationException e) {
            // Non-POSIX system, use default permissions
        }

        try (InputStream in = Files.newInputStream(targetPath, StandardOpenOption.READ);
             OutputStream out = Files.newOutputStream(tempFile, 
                 StandardOpenOption.WRITE, 
                 StandardOpenOption.CREATE, 
                 StandardOpenOption.TRUNCATE_EXISTING)) {
            
            byte[] buffer = new byte[BUFFER_SIZE];
            int bytesRead;
            long totalBytesRead = 0;

            while ((bytesRead = in.read(buffer)) != -1) {
                totalBytesRead += bytesRead;
                if (totalBytesRead > MAX_FILE_SIZE) {
                    throw new IOException("File size check failed during copy");
                }
                out.write(buffer, 0, bytesRead);
            }
            
            out.flush();
            if (out instanceof FileOutputStream) {
                ((FileOutputStream) out).getFD().sync();
            }
        } catch (IOException e) {
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException deleteEx) {
                    // Log deletion failure but don't mask original exception
                }
            }
            throw new IOException("Failed to copy file: " + e.getMessage(), e);
        }

        return tempFile.toString();
    }

    public static void main(String[] args) {
        String testBaseDir = System.getProperty("java.io.tmpdir");
        
        // Test Case 1: Valid file copy
        try {
            Path testFile1 = Files.createTempFile(Paths.get(testBaseDir), "test1_", ".txt");
            Files.write(testFile1, "Test content 1".getBytes(StandardCharsets.UTF_8));
            String result1 = copyFileToTemp(testBaseDir, testFile1.getFileName().toString());
            System.out.println("Test 1 PASS: " + result1);
            Files.deleteIfExists(testFile1);
            Files.deleteIfExists(Paths.get(result1));
        } catch (Exception e) {
            System.out.println("Test 1 FAIL: " + e.getMessage());
        }

        // Test Case 2: Null base directory
        try {
            copyFileToTemp(null, "test.txt");
            System.out.println("Test 2 FAIL: Should throw exception");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 2 PASS: " + e.getMessage());
        } catch (Exception e) {
            System.out.println("Test 2 FAIL: Wrong exception type");
        }

        // Test Case 3: Path traversal attempt
        try {
            copyFileToTemp(testBaseDir, "../etc/passwd");
            System.out.println("Test 3 FAIL: Should throw exception");
        } catch (SecurityException e) {
            System.out.println("Test 3 PASS: " + e.getMessage());
        } catch (Exception e) {
            System.out.println("Test 3 PASS: Path traversal blocked - " + e.getMessage());
        }

        // Test Case 4: Non-existent file
        try {
            copyFileToTemp(testBaseDir, "nonexistent_file_12345.txt");
            System.out.println("Test 4 FAIL: Should throw exception");
        } catch (FileNotFoundException e) {
            System.out.println("Test 4 PASS: " + e.getMessage());
        } catch (Exception e) {
            System.out.println("Test 4 FAIL: Wrong exception type");
        }

        // Test Case 5: Empty file path
        try {
            copyFileToTemp(testBaseDir, "");
            System.out.println("Test 5 FAIL: Should throw exception");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 5 PASS: " + e.getMessage());
        } catch (Exception e) {
            System.out.println("Test 5 FAIL: Wrong exception type");
        }
    }
}
