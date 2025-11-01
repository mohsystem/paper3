
import java.io.*;
import java.util.zip.*;
import java.nio.file.*;

public class Lse49 {
    public static void unzipArchive(String zipFilePath, String destDirectory) throws IOException {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        try (ZipInputStream zipIn = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry entry = zipIn.getNextEntry();
            
            while (entry != null) {
                String filePath = destDirectory + File.separator + entry.getName();
                
                // Security check: prevent path traversal
                File destFile = new File(filePath);
                String destDirPath = destDir.getCanonicalPath();
                String destFilePath = destFile.getCanonicalPath();
                
                if (!destFilePath.startsWith(destDirPath + File.separator)) {
                    throw new IOException("Entry is outside of the target dir: " + entry.getName());
                }
                
                if (!entry.isDirectory()) {
                    extractFile(zipIn, filePath);
                } else {
                    File dir = new File(filePath);
                    dir.mkdirs();
                }
                zipIn.closeEntry();
                entry = zipIn.getNextEntry();
            }
        }
    }
    
    private static void extractFile(ZipInputStream zipIn, String filePath) throws IOException {
        File file = new File(filePath);
        file.getParentFile().mkdirs();
        
        try (BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(filePath))) {
            byte[] bytesIn = new byte[4096];
            int read;
            while ((read = zipIn.read(bytesIn)) != -1) {
                bos.write(bytesIn, 0, read);
            }
        }
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1: Basic unzip
            unzipArchive("archive.zip", "/tmp/unpack");
            System.out.println("Test 1: Successfully unzipped to /tmp/unpack");
            
            // Test case 2: Different destination
            unzipArchive("archive.zip", "/tmp/unpack2");
            System.out.println("Test 2: Successfully unzipped to /tmp/unpack2");
            
            // Test case 3: Unzip to existing directory
            unzipArchive("archive.zip", "/tmp/unpack");
            System.out.println("Test 3: Successfully unzipped to existing directory");
            
            // Test case 4: Create nested directories
            unzipArchive("archive.zip", "/tmp/test/nested/unpack");
            System.out.println("Test 4: Successfully created nested directories");
            
            // Test case 5: Handle non-existent zip file
            try {
                unzipArchive("nonexistent.zip", "/tmp/unpack");
                System.out.println("Test 5: Should have thrown exception");
            } catch (FileNotFoundException e) {
                System.out.println("Test 5: Correctly handled non-existent file");
            }
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
