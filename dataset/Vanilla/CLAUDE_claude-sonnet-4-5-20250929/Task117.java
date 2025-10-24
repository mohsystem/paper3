
import java.io.*;
import java.util.zip.*;
import java.util.Enumeration;
import org.apache.commons.compress.archivers.tar.*;
import org.apache.commons.compress.compressors.gzip.*;
import org.apache.commons.compress.utils.IOUtils;

public class Task117 {
    
    public static boolean extractZip(String zipFilePath, String destDirectory) {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        try (ZipInputStream zipIn = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry entry = zipIn.getNextEntry();
            while (entry != null) {
                String filePath = destDirectory + File.separator + entry.getName();
                if (!entry.isDirectory()) {
                    extractFile(zipIn, filePath);
                } else {
                    File dir = new File(filePath);
                    dir.mkdirs();
                }
                zipIn.closeEntry();
                entry = zipIn.getNextEntry();
            }
            return true;
        } catch (IOException e) {
            System.err.println("Error extracting ZIP: " + e.getMessage());
            return false;
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
    
    public static boolean extractTarGz(String tarGzFilePath, String destDirectory) {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        try (FileInputStream fis = new FileInputStream(tarGzFilePath);
             GzipCompressorInputStream gzipIn = new GzipCompressorInputStream(fis);
             TarArchiveInputStream tarIn = new TarArchiveInputStream(gzipIn)) {
            
            TarArchiveEntry entry;
            while ((entry = tarIn.getNextTarEntry()) != null) {
                String filePath = destDirectory + File.separator + entry.getName();
                File outputFile = new File(filePath);
                
                if (entry.isDirectory()) {
                    outputFile.mkdirs();
                } else {
                    outputFile.getParentFile().mkdirs();
                    try (FileOutputStream fos = new FileOutputStream(outputFile)) {
                        IOUtils.copy(tarIn, fos);
                    }
                }
            }
            return true;
        } catch (IOException e) {
            System.err.println("Error extracting TAR.GZ: " + e.getMessage());
            return false;
        }
    }
    
    public static boolean extractTar(String tarFilePath, String destDirectory) {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        try (FileInputStream fis = new FileInputStream(tarFilePath);
             TarArchiveInputStream tarIn = new TarArchiveInputStream(fis)) {
            
            TarArchiveEntry entry;
            while ((entry = tarIn.getNextTarEntry()) != null) {
                String filePath = destDirectory + File.separator + entry.getName();
                File outputFile = new File(filePath);
                
                if (entry.isDirectory()) {
                    outputFile.mkdirs();
                } else {
                    outputFile.getParentFile().mkdirs();
                    try (FileOutputStream fos = new FileOutputStream(outputFile)) {
                        IOUtils.copy(tarIn, fos);
                    }
                }
            }
            return true;
        } catch (IOException e) {
            System.err.println("Error extracting TAR: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Create and extract a simple ZIP file
        System.out.println("Test 1: ZIP extraction");
        try {
            String zipPath = "test1.zip";
            String extractPath = "extracted_zip";
            
            // Create a test ZIP file
            try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipPath))) {
                zos.putNextEntry(new ZipEntry("test.txt"));
                zos.write("Hello from ZIP!".getBytes());
                zos.closeEntry();
            }
            
            boolean result = extractZip(zipPath, extractPath);
            System.out.println("ZIP extraction result: " + result);
            new File(zipPath).delete();
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Extract ZIP with directory structure
        System.out.println("\\nTest 2: ZIP with directories");
        try {
            String zipPath = "test2.zip";
            String extractPath = "extracted_zip2";
            
            try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipPath))) {
                zos.putNextEntry(new ZipEntry("folder/"));
                zos.closeEntry();
                zos.putNextEntry(new ZipEntry("folder/file.txt"));
                zos.write("Nested file content".getBytes());
                zos.closeEntry();
            }
            
            boolean result = extractZip(zipPath, extractPath);
            System.out.println("ZIP with directories extraction result: " + result);
            new File(zipPath).delete();
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Non-existent file
        System.out.println("\\nTest 3: Non-existent file");
        boolean result3 = extractZip("nonexistent.zip", "output3");
        System.out.println("Non-existent file result: " + result3);
        
        // Test case 4: Empty ZIP
        System.out.println("\\nTest 4: Empty ZIP");
        try {
            String zipPath = "empty.zip";
            String extractPath = "extracted_empty";
            
            try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipPath))) {
                // Empty ZIP
            }
            
            boolean result = extractZip(zipPath, extractPath);
            System.out.println("Empty ZIP extraction result: " + result);
            new File(zipPath).delete();
        } catch (Exception e) {
            System.out.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test case 5: Multiple files in ZIP
        System.out.println("\\nTest 5: Multiple files in ZIP");
        try {
            String zipPath = "test5.zip";
            String extractPath = "extracted_zip5";
            
            try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipPath))) {
                for (int i = 1; i <= 3; i++) {
                    zos.putNextEntry(new ZipEntry("file" + i + ".txt"));
                    zos.write(("Content of file " + i).getBytes());
                    zos.closeEntry();
                }
            }
            
            boolean result = extractZip(zipPath, extractPath);
            System.out.println("Multiple files ZIP extraction result: " + result);
            new File(zipPath).delete();
        } catch (Exception e) {
            System.out.println("Test 5 failed: " + e.getMessage());
        }
    }
}
