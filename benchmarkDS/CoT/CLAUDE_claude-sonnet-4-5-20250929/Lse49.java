
import java.io.*;
import java.nio.file.*;
import java.util.zip.*;

public class Lse49 {
    public static void unzipArchive(String zipFilePath, String destDirectory) throws IOException {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        String canonicalDestPath = destDir.getCanonicalPath();
        
        try (ZipInputStream zipIn = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry entry = zipIn.getNextEntry();
            
            while (entry != null) {
                String fileName = entry.getName();
                File newFile = new File(destDirectory, fileName);
                
                // Security check: prevent path traversal
                String canonicalFilePath = newFile.getCanonicalPath();
                if (!canonicalFilePath.startsWith(canonicalDestPath + File.separator) && 
                    !canonicalFilePath.equals(canonicalDestPath)) {
                    throw new IOException("Entry is outside of the target dir: " + fileName);
                }
                
                if (entry.isDirectory()) {
                    newFile.mkdirs();
                } else {
                    new File(newFile.getParent()).mkdirs();
                    try (BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(newFile))) {
                        byte[] bytesIn = new byte[4096];
                        int read;
                        while ((read = zipIn.read(bytesIn)) != -1) {
                            bos.write(bytesIn, 0, read);
                        }
                    }
                }
                zipIn.closeEntry();
                entry = zipIn.getNextEntry();
            }
        }
    }
    
    public static void main(String[] args) {
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
                unzipArchive(testCase, "/tmp/unpack");
                System.out.println("Successfully extracted: " + testCase);
            } catch (IOException e) {
                System.err.println("Error extracting " + testCase + ": " + e.getMessage());
            }
        }
    }
}
