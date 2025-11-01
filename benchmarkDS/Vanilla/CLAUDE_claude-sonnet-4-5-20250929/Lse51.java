
import java.io.*;
import java.util.zip.*;
import org.apache.commons.compress.archivers.tar.*;
import org.apache.commons.compress.compressors.gzip.*;
import org.apache.commons.compress.compressors.bzip2.*;
import org.apache.commons.compress.utils.IOUtils;

public class Lse51 {
    
    public static void unzipTarArchive(String tarFilePath, String destDirectory) throws IOException {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        try (FileInputStream fis = new FileInputStream(tarFilePath);
             InputStream bis = getInputStream(tarFilePath, fis);
             TarArchiveInputStream tais = new TarArchiveInputStream(bis)) {
            
            TarArchiveEntry entry;
            while ((entry = tais.getNextTarEntry()) != null) {
                File outputFile = new File(destDirectory, entry.getName());
                
                if (entry.isDirectory()) {
                    outputFile.mkdirs();
                } else {
                    outputFile.getParentFile().mkdirs();
                    try (FileOutputStream fos = new FileOutputStream(outputFile)) {
                        IOUtils.copy(tais, fos);
                    }
                }
            }
        }
    }
    
    private static InputStream getInputStream(String filePath, FileInputStream fis) throws IOException {
        if (filePath.endsWith(".tar.gz") || filePath.endsWith(".tgz")) {
            return new GzipCompressorInputStream(fis);
        } else if (filePath.endsWith(".tar.bz2")) {
            return new BZip2CompressorInputStream(fis);
        } else {
            return fis;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {
            "test1.tar.gz",
            "test2.tar.bz2",
            "test3.tar",
            "test4.tgz",
            "test5.tar.gz"
        };
        
        for (String testCase : testCases) {
            try {
                System.out.println("Extracting: " + testCase);
                unzipTarArchive(testCase, "/tmp/unpack");
                System.out.println("Successfully extracted: " + testCase);
            } catch (IOException e) {
                System.out.println("Error extracting " + testCase + ": " + e.getMessage());
            }
        }
    }
}
