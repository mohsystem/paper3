
import java.io.*;
import java.nio.file.*;
import java.util.zip.*;

public class Lse50 {
    
    public static void secureExtractZip(String zipFilePath, String destDirectory) throws IOException {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        String canonicalDestPath = destDir.getCanonicalPath();
        
        try (ZipInputStream zis = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry zipEntry = zis.getNextEntry();
            
            while (zipEntry != null) {
                String fileName = zipEntry.getName();
                
                // Security: Sanitize file name to prevent path traversal
                File newFile = new File(destDir, fileName);
                String canonicalPath = newFile.getCanonicalPath();
                
                // Security: Ensure extracted file stays within target directory
                if (!canonicalPath.startsWith(canonicalDestPath + File.separator) && 
                    !canonicalPath.equals(canonicalDestPath)) {
                    throw new IOException("Entry is outside of the target dir: " + fileName);
                }
                
                if (zipEntry.isDirectory()) {
                    newFile.mkdirs();
                } else {
                    // Create parent directories if needed
                    new File(newFile.getParent()).mkdirs();
                    
                    try (FileOutputStream fos = new FileOutputStream(newFile)) {
                        byte[] buffer = new byte[4096];
                        int len;
                        while ((len = zis.read(buffer)) > 0) {
                            fos.write(buffer, 0, len);
                        }
                    }
                }
                zipEntry = zis.getNextEntry();
            }
            zis.closeEntry();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal extraction
        try {
            secureExtractZip("archive.zip", "/tmp/unpack");
            System.out.println("Test 1: Successfully extracted archive.zip");
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: With different archive name
        try {
            secureExtractZip("test.zip", "/tmp/unpack");
            System.out.println("Test 2: Successfully extracted test.zip");
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Non-existent file
        try {
            secureExtractZip("nonexistent.zip", "/tmp/unpack");
            System.out.println("Test 3: Completed");
        } catch (Exception e) {
            System.out.println("Test 3 failed (expected): " + e.getMessage());
        }
        
        // Test case 4: Different destination
        try {
            secureExtractZip("archive.zip", "/tmp/unpack2");
            System.out.println("Test 4: Successfully extracted to different location");
        } catch (Exception e) {
            System.out.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test case 5: Create nested directory structure
        try {
            secureExtractZip("archive.zip", "/tmp/unpack/nested/path");
            System.out.println("Test 5: Successfully extracted to nested path");
        } catch (Exception e) {
            System.out.println("Test 5 failed: " + e.getMessage());
        }
    }
}
