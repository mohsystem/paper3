
import java.io.*;
import java.util.zip.*;
import java.nio.file.*;

public class Lse50 {
    public static void extractZipFile(String zipFilePath, String destDirectory) throws IOException {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        try (ZipInputStream zipIn = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry entry = zipIn.getNextEntry();
            
            while (entry != null) {
                String filePath = destDirectory + File.separator + entry.getName();
                
                // Security check: prevent path traversal attacks
                File destFile = new File(filePath);
                String destDirPath = destDir.getCanonicalPath();
                String destFilePath = destFile.getCanonicalPath();
                
                if (!destFilePath.startsWith(destDirPath + File.separator)) {
                    throw new IOException("Entry is outside of the target directory: " + entry.getName());
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
        // Test cases
        String[] testCases = {
            "archive.zip",
            "test1.zip",
            "test2.zip",
            "test3.zip",
            "test4.zip"
        };
        
        for (String testCase : testCases) {
            try {
                System.out.println("Extracting: " + testCase);
                extractZipFile(testCase, "/tmp/unpack");
                System.out.println("Successfully extracted: " + testCase);
            } catch (IOException e) {
                System.out.println("Error extracting " + testCase + ": " + e.getMessage());
            }
        }
    }
}
