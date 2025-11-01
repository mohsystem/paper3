
import java.io.*;
import java.util.zip.*;
import org.apache.commons.compress.archivers.tar.*;
import org.apache.commons.compress.compressors.gzip.*;
import org.apache.commons.compress.compressors.bzip2.*;

public class Lse51 {
    
    // Method to extract tar archive to a specified directory
    public static void extractTarArchive(String tarFilePath, String destDirectory) throws IOException {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        try (FileInputStream fis = new FileInputStream(tarFilePath);
             InputStream bis = getCompressedInputStream(fis, tarFilePath);
             TarArchiveInputStream tais = new TarArchiveInputStream(bis)) {
            
            TarArchiveEntry entry;
            while ((entry = tais.getNextTarEntry()) != null) {
                File outputFile = new File(destDir, entry.getName());
                
                // Security check: prevent path traversal
                if (!outputFile.getCanonicalPath().startsWith(destDir.getCanonicalPath())) {
                    throw new IOException("Entry is outside of the target directory");
                }
                
                if (entry.isDirectory()) {
                    outputFile.mkdirs();
                } else {
                    outputFile.getParentFile().mkdirs();
                    try (FileOutputStream fos = new FileOutputStream(outputFile)) {
                        byte[] buffer = new byte[8192];
                        int len;
                        while ((len = tais.read(buffer)) != -1) {
                            fos.write(buffer, 0, len);
                        }
                    }
                }
            }
        }
    }
    
    private static InputStream getCompressedInputStream(InputStream fis, String fileName) throws IOException {
        if (fileName.endsWith(".tar.gz") || fileName.endsWith(".tgz")) {
            return new GzipCompressorInputStream(fis);
        } else if (fileName.endsWith(".tar.bz2")) {
            return new BZip2CompressorInputStream(fis);
        } else {
            return fis;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testFiles = {
            "test1.tar",
            "test2.tar.gz",
            "test3.tar.bz2",
            "test4.tgz",
            "test5.tar"
        };
        
        for (String testFile : testFiles) {
            try {
                extractTarArchive(testFile, "/tmp/unpack");
                System.out.println("Successfully extracted: " + testFile);
            } catch (IOException e) {
                System.err.println("Failed to extract " + testFile + ": " + e.getMessage());
            }
        }
    }
}
