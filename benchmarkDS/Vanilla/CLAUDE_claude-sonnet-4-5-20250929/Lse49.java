
import java.io.*;
import java.util.zip.*;

public class Lse49 {
    public static void unzipArchive(String zipFilePath, String destDirectory) {
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
        } catch (IOException e) {
            e.printStackTrace();
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
        // Test case 1
        unzipArchive("archive.zip", "/tmp/unpack");
        
        // Test case 2
        unzipArchive("test1.zip", "/tmp/unpack1");
        
        // Test case 3
        unzipArchive("test2.zip", "/tmp/unpack2");
        
        // Test case 4
        unzipArchive("data.zip", "/tmp/unpack3");
        
        // Test case 5
        unzipArchive("files.zip", "/tmp/unpack4");
        
        System.out.println("Unzip operations completed.");
    }
}
