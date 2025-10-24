
import java.io.*;
import java.nio.file.*;
import java.util.zip.*;
import java.util.Enumeration;
import org.apache.commons.compress.archivers.tar.*;
import org.apache.commons.compress.compressors.gzip.*;

public class Task117 {
    
    // Secure path validation to prevent path traversal attacks
    private static boolean isValidPath(String destDir, String filePath) throws IOException {
        File destDirFile = new File(destDir).getCanonicalFile();
        File destFile = new File(destDirFile, filePath).getCanonicalFile();
        return destFile.getCanonicalPath().startsWith(destDirFile.getCanonicalPath() + File.separator);
    }
    
    // Extract ZIP files
    public static void extractZip(String zipFilePath, String destDir) throws IOException {
        File destDirectory = new File(destDir);
        if (!destDirectory.exists()) {
            destDirectory.mkdirs();
        }
        
        try (ZipFile zipFile = new ZipFile(zipFilePath)) {
            Enumeration<? extends ZipEntry> entries = zipFile.entries();
            
            while (entries.hasMoreElements()) {
                ZipEntry entry = entries.nextElement();
                String entryName = entry.getName();
                
                // Validate path to prevent path traversal
                if (!isValidPath(destDir, entryName)) {
                    throw new IOException("Invalid entry path detected: " + entryName);
                }
                
                File destFile = new File(destDir, entryName);
                
                if (entry.isDirectory()) {
                    destFile.mkdirs();
                } else {
                    // Create parent directories if needed
                    destFile.getParentFile().mkdirs();
                    
                    try (InputStream is = zipFile.getInputStream(entry);
                         FileOutputStream fos = new FileOutputStream(destFile)) {
                        byte[] buffer = new byte[8192];
                        int len;
                        while ((len = is.read(buffer)) != -1) {
                            fos.write(buffer, 0, len);
                        }
                    }
                }
            }
        }
        System.out.println("ZIP extraction completed: " + zipFilePath);
    }
    
    // Extract TAR files (with optional GZIP compression)
    public static void extractTar(String tarFilePath, String destDir) throws IOException {
        File destDirectory = new File(destDir);
        if (!destDirectory.exists()) {
            destDirectory.mkdirs();
        }
        
        InputStream fileInputStream = new FileInputStream(tarFilePath);
        InputStream tarInputStream = fileInputStream;
        
        // Check if it's a GZIP compressed TAR
        if (tarFilePath.endsWith(".tar.gz") || tarFilePath.endsWith(".tgz")) {
            tarInputStream = new GzipCompressorInputStream(fileInputStream);
        }
        
        try (TarArchiveInputStream tarInput = new TarArchiveInputStream(tarInputStream)) {
            TarArchiveEntry entry;
            
            while ((entry = tarInput.getNextTarEntry()) != null) {
                String entryName = entry.getName();
                
                // Validate path to prevent path traversal
                if (!isValidPath(destDir, entryName)) {
                    throw new IOException("Invalid entry path detected: " + entryName);
                }
                
                File destFile = new File(destDir, entryName);
                
                if (entry.isDirectory()) {
                    destFile.mkdirs();
                } else {
                    destFile.getParentFile().mkdirs();
                    
                    try (FileOutputStream fos = new FileOutputStream(destFile)) {
                        byte[] buffer = new byte[8192];
                        int len;
                        while ((len = tarInput.read(buffer)) != -1) {
                            fos.write(buffer, 0, len);
                        }
                    }
                }
            }
        }
        System.out.println("TAR extraction completed: " + tarFilePath);
    }
    
    public static void main(String[] args) {
        System.out.println("Archive Extraction Program - Test Cases\\n");
        
        // Test Case 1: Extract a simple ZIP file
        try {
            String testZip1 = "test1.zip";
            createTestZip(testZip1, "test_output1");
            extractZip(testZip1, "extracted_zip1");
            System.out.println("Test 1 passed: Simple ZIP extraction\\n");
        } catch (Exception e) {
            System.out.println("Test 1 error: " + e.getMessage() + "\\n");
        }
        
        // Test Case 2: Extract ZIP with subdirectories
        try {
            String testZip2 = "test2.zip";
            createTestZipWithSubdirs(testZip2);
            extractZip(testZip2, "extracted_zip2");
            System.out.println("Test 2 passed: ZIP with subdirectories\\n");
        } catch (Exception e) {
            System.out.println("Test 2 error: " + e.getMessage() + "\\n");
        }
        
        // Test Case 3: Extract TAR file
        try {
            String testTar = "test.tar";
            createTestTar(testTar);
            extractTar(testTar, "extracted_tar");
            System.out.println("Test 3 passed: TAR extraction\\n");
        } catch (Exception e) {
            System.out.println("Test 3 error: " + e.getMessage() + "\\n");
        }
        
        // Test Case 4: Test path traversal prevention
        try {
            String maliciousZip = "malicious.zip";
            createMaliciousZip(maliciousZip);
            extractZip(maliciousZip, "safe_output");
            System.out.println("Test 4 failed: Should have prevented path traversal\\n");
        } catch (IOException e) {
            System.out.println("Test 4 passed: Path traversal prevented\\n");
        } catch (Exception e) {
            System.out.println("Test 4 error: " + e.getMessage() + "\\n");
        }
        
        // Test Case 5: Empty archive handling
        try {
            String emptyZip = "empty.zip";
            createEmptyZip(emptyZip);
            extractZip(emptyZip, "extracted_empty");
            System.out.println("Test 5 passed: Empty ZIP handled\\n");
        } catch (Exception e) {
            System.out.println("Test 5 error: " + e.getMessage() + "\\n");
        }
    }
    
    // Helper methods to create test archives
    private static void createTestZip(String zipPath, String contentDir) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipPath))) {
            ZipEntry entry = new ZipEntry("test.txt");
            zos.putNextEntry(entry);
            zos.write("Test content".getBytes());
            zos.closeEntry();
        }
    }
    
    private static void createTestZipWithSubdirs(String zipPath) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipPath))) {
            zos.putNextEntry(new ZipEntry("subdir/"));
            zos.closeEntry();
            ZipEntry entry = new ZipEntry("subdir/file.txt");
            zos.putNextEntry(entry);
            zos.write("Subdirectory content".getBytes());
            zos.closeEntry();
        }
    }
    
    private static void createTestTar(String tarPath) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(tarPath);
             TarArchiveOutputStream taos = new TarArchiveOutputStream(fos)) {
            TarArchiveEntry entry = new TarArchiveEntry("test.txt");
            byte[] content = "TAR test content".getBytes();
            entry.setSize(content.length);
            taos.putArchiveEntry(entry);
            taos.write(content);
            taos.closeArchiveEntry();
        }
    }
    
    private static void createMaliciousZip(String zipPath) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipPath))) {
            ZipEntry entry = new ZipEntry("../../etc/passwd");
            zos.putNextEntry(entry);
            zos.write("malicious content".getBytes());
            zos.closeEntry();
        }
    }
    
    private static void createEmptyZip(String zipPath) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipPath))) {
            // Empty archive
        }
    }
}
