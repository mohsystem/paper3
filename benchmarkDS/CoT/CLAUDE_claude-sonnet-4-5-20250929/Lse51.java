
import java.io.*;
import java.nio.file.*;
import java.util.zip.*;
import org.apache.commons.compress.archivers.tar.*;
import org.apache.commons.compress.compressors.gzip.*;
import org.apache.commons.compress.compressors.bzip2.*;

public class Lse51 {
    
    private static final String EXTRACT_DIR = "/tmp/unpack";
    
    public static void extractTarArchive(String tarFilePath) throws IOException {
        // Create and validate extraction directory
        Path extractPath = Paths.get(EXTRACT_DIR).toAbsolutePath().normalize();
        Files.createDirectories(extractPath);
        
        try (InputStream fileInputStream = Files.newInputStream(Paths.get(tarFilePath));
             InputStream decompressedStream = getDecompressedStream(fileInputStream, tarFilePath);
             TarArchiveInputStream tarInputStream = new TarArchiveInputStream(decompressedStream)) {
            
            TarArchiveEntry entry;
            while ((entry = tarInputStream.getNextTarEntry()) != null) {
                // Security: Validate entry path to prevent path traversal
                Path entryPath = extractPath.resolve(entry.getName()).normalize();
                
                if (!entryPath.startsWith(extractPath)) {
                    throw new SecurityException("Path traversal attempt detected: " + entry.getName());
                }
                
                if (entry.isDirectory()) {
                    Files.createDirectories(entryPath);
                } else {
                    Files.createDirectories(entryPath.getParent());
                    try (OutputStream outputStream = Files.newOutputStream(entryPath)) {
                        byte[] buffer = new byte[8192];
                        int bytesRead;
                        while ((bytesRead = tarInputStream.read(buffer)) != -1) {
                            outputStream.write(buffer, 0, bytesRead);
                        }
                    }
                }
            }
        }
    }
    
    private static InputStream getDecompressedStream(InputStream input, String filePath) throws IOException {
        if (filePath.endsWith(".tar.gz") || filePath.endsWith(".tgz")) {
            return new GzipCompressorInputStream(new BufferedInputStream(input));
        } else if (filePath.endsWith(".tar.bz2")) {
            return new BZip2CompressorInputStream(new BufferedInputStream(input));
        }
        return new BufferedInputStream(input);
    }
    
    public static void main(String[] args) {
        // Test cases - Note: Requires actual tar files to test
        String[] testCases = {
            "/tmp/test1.tar",
            "/tmp/test2.tar.gz",
            "/tmp/test3.tar.bz2",
            "/tmp/test4.tgz",
            "/tmp/test5.tar"
        };
        
        for (String testFile : testCases) {
            try {
                System.out.println("Extracting: " + testFile);
                extractTarArchive(testFile);
                System.out.println("Successfully extracted to " + EXTRACT_DIR);
            } catch (Exception e) {
                System.err.println("Error extracting " + testFile + ": " + e.getMessage());
            }
        }
    }
}
