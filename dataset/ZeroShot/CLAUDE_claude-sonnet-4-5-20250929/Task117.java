
import java.io.*;
import java.nio.file.*;
import java.util.zip.*;
import java.util.Enumeration;
import org.apache.commons.compress.archivers.tar.*;
import org.apache.commons.compress.compressors.gzip.*;

public class Task117 {
    
    public static void extractZip(String zipFilePath, String destDirectory) throws IOException {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        try (ZipFile zipFile = new ZipFile(zipFilePath)) {
            Enumeration<? extends ZipEntry> entries = zipFile.entries();
            
            while (entries.hasMoreElements()) {
                ZipEntry entry = entries.nextElement();
                
                // Security: Prevent path traversal attacks
                String entryName = entry.getName();
                File destFile = new File(destDir, entryName);
                String destDirCanonical = destDir.getCanonicalPath();
                String destFileCanonical = destFile.getCanonicalPath();
                
                if (!destFileCanonical.startsWith(destDirCanonical + File.separator)) {
                    throw new IOException("Entry is outside of the target directory: " + entryName);
                }
                
                if (entry.isDirectory()) {
                    destFile.mkdirs();
                } else {
                    destFile.getParentFile().mkdirs();
                    try (InputStream in = zipFile.getInputStream(entry);
                         FileOutputStream out = new FileOutputStream(destFile)) {
                        byte[] buffer = new byte[8192];
                        int bytesRead;
                        while ((bytesRead = in.read(buffer)) != -1) {
                            out.write(buffer, 0, bytesRead);
                        }
                    }
                }
            }
        }
    }
    
    public static void extractTar(String tarFilePath, String destDirectory) throws IOException {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        try (FileInputStream fis = new FileInputStream(tarFilePath);
             TarArchiveInputStream tarIn = new TarArchiveInputStream(fis)) {
            
            TarArchiveEntry entry;
            while ((entry = tarIn.getNextTarEntry()) != null) {
                // Security: Prevent path traversal attacks
                String entryName = entry.getName();
                File destFile = new File(destDir, entryName);
                String destDirCanonical = destDir.getCanonicalPath();
                String destFileCanonical = destFile.getCanonicalPath();
                
                if (!destFileCanonical.startsWith(destDirCanonical + File.separator)) {
                    throw new IOException("Entry is outside of the target directory: " + entryName);
                }
                
                if (entry.isDirectory()) {
                    destFile.mkdirs();
                } else {
                    destFile.getParentFile().mkdirs();
                    try (FileOutputStream out = new FileOutputStream(destFile)) {
                        byte[] buffer = new byte[8192];
                        int bytesRead;
                        while ((bytesRead = tarIn.read(buffer)) != -1) {
                            out.write(buffer, 0, bytesRead);
                        }
                    }
                }
            }
        }
    }
    
    public static void extractTarGz(String tarGzFilePath, String destDirectory) throws IOException {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        try (FileInputStream fis = new FileInputStream(tarGzFilePath);
             GzipCompressorInputStream gzIn = new GzipCompressorInputStream(fis);
             TarArchiveInputStream tarIn = new TarArchiveInputStream(gzIn)) {
            
            TarArchiveEntry entry;
            while ((entry = tarIn.getNextTarEntry()) != null) {
                // Security: Prevent path traversal attacks
                String entryName = entry.getName();
                File destFile = new File(destDir, entryName);
                String destDirCanonical = destDir.getCanonicalPath();
                String destFileCanonical = destFile.getCanonicalPath();
                
                if (!destFileCanonical.startsWith(destDirCanonical + File.separator)) {
                    throw new IOException("Entry is outside of the target directory: " + entryName);
                }
                
                if (entry.isDirectory()) {
                    destFile.mkdirs();
                } else {
                    destFile.getParentFile().mkdirs();
                    try (FileOutputStream out = new FileOutputStream(destFile)) {
                        byte[] buffer = new byte[8192];
                        int bytesRead;
                        while ((bytesRead = tarIn.read(buffer)) != -1) {
                            out.write(buffer, 0, bytesRead);
                        }
                    }
                }
            }
        }
    }
    
    public static void main(String[] args) {
        try {
            // Test Case 1: Create and extract a simple ZIP file
            System.out.println("Test Case 1: Creating and extracting ZIP file");
            String zipPath = "test1.zip";
            String extractDir1 = "extract1";
            createTestZip(zipPath);
            extractZip(zipPath, extractDir1);
            System.out.println("ZIP extraction completed successfully");
            
            // Test Case 2: Extract ZIP with nested directories
            System.out.println("\\nTest Case 2: ZIP with nested directories");
            String zipPath2 = "test2.zip";
            String extractDir2 = "extract2";
            createTestZipWithDirs(zipPath2);
            extractZip(zipPath2, extractDir2);
            System.out.println("Nested ZIP extraction completed successfully");
            
            // Test Case 3: Create and extract TAR file
            System.out.println("\\nTest Case 3: Creating and extracting TAR file");
            String tarPath = "test3.tar";
            String extractDir3 = "extract3";
            createTestTar(tarPath);
            extractTar(tarPath, extractDir3);
            System.out.println("TAR extraction completed successfully");
            
            // Test Case 4: Extract TAR.GZ file
            System.out.println("\\nTest Case 4: Creating and extracting TAR.GZ file");
            String tarGzPath = "test4.tar.gz";
            String extractDir4 = "extract4";
            createTestTarGz(tarGzPath);
            extractTarGz(tarGzPath, extractDir4);
            System.out.println("TAR.GZ extraction completed successfully");
            
            // Test Case 5: Test security - path traversal prevention
            System.out.println("\\nTest Case 5: Testing path traversal prevention");
            try {
                String maliciousZip = "malicious.zip";
                String extractDir5 = "extract5";
                createMaliciousZip(maliciousZip);
                extractZip(maliciousZip, extractDir5);
                System.out.println("WARNING: Malicious file was extracted");
            } catch (IOException e) {
                System.out.println("Security check passed: " + e.getMessage());
            }
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    private static void createTestZip(String zipPath) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipPath))) {
            ZipEntry entry = new ZipEntry("test.txt");
            zos.putNextEntry(entry);
            zos.write("Hello, World!".getBytes());
            zos.closeEntry();
        }
    }
    
    private static void createTestZipWithDirs(String zipPath) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipPath))) {
            zos.putNextEntry(new ZipEntry("folder1/"));
            zos.closeEntry();
            zos.putNextEntry(new ZipEntry("folder1/file1.txt"));
            zos.write("File in folder".getBytes());
            zos.closeEntry();
        }
    }
    
    private static void createTestTar(String tarPath) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(tarPath);
             TarArchiveOutputStream taos = new TarArchiveOutputStream(fos)) {
            TarArchiveEntry entry = new TarArchiveEntry("test.txt");
            byte[] content = "Hello from TAR!".getBytes();
            entry.setSize(content.length);
            taos.putArchiveEntry(entry);
            taos.write(content);
            taos.closeArchiveEntry();
        }
    }
    
    private static void createTestTarGz(String tarGzPath) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(tarGzPath);
             GzipCompressorOutputStream gzos = new GzipCompressorOutputStream(fos);
             TarArchiveOutputStream taos = new TarArchiveOutputStream(gzos)) {
            TarArchiveEntry entry = new TarArchiveEntry("test.txt");
            byte[] content = "Hello from TAR.GZ!".getBytes();
            entry.setSize(content.length);
            taos.putArchiveEntry(entry);
            taos.write(content);
            taos.closeArchiveEntry();
        }
    }
    
    private static void createMaliciousZip(String zipPath) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipPath))) {
            ZipEntry entry = new ZipEntry("../../../etc/malicious.txt");
            zos.putNextEntry(entry);
            zos.write("Malicious content".getBytes());
            zos.closeEntry();
        }
    }
}
