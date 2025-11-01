import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Comparator;

// Required external library: Apache Commons Compress
// Add to your build system (e.g., Maven/Gradle).
// <dependency>
//     <groupId>org.apache.commons</groupId>
//     <artifactId>commons-compress</artifactId>
//     <version>1.26.1</version>
// </dependency>
import org.apache.commons.compress.archivers.tar.TarArchiveEntry;
import org.apache.commons.compress.archivers.tar.TarArchiveInputStream;
import org.apache.commons.compress.compressors.bzip2.BZip2CompressorInputStream;
import org.apache.commons.compress.compressors.gzip.GzipCompressorInputStream;


public class Lse51 {

    /**
     * Safely extracts a tar archive, preventing path traversal attacks.
     * Supports .tar, .tar.gz (tgz), and .tar.bz2 formats.
     *
     * @param tarFile The path to the tar archive.
     * @param destDir The destination directory for extraction.
     * @return true if extraction was successful, false otherwise.
     */
    public static boolean extractTar(String tarFile, String destDir) {
        File archive = new File(tarFile);
        File destination = new File(destDir);

        if (!archive.exists()) {
            System.err.println("Error: Archive file not found: " + tarFile);
            return false;
        }

        if (!destination.exists()) {
            destination.mkdirs();
        }

        try {
            String canonicalDestPath = destination.getCanonicalPath();
            System.out.println("Extracting " + tarFile + " to " + canonicalDestPath);

            InputStream fi = new FileInputStream(archive);
            InputStream decompressor;

            if (tarFile.endsWith(".gz") || tarFile.endsWith(".tgz")) {
                decompressor = new GzipCompressorInputStream(fi);
            } else if (tarFile.endsWith(".bz2")) {
                decompressor = new BZip2CompressorInputStream(fi);
            } else {
                decompressor = fi; // Plain .tar
            }

            try (TarArchiveInputStream tarInput = new TarArchiveInputStream(decompressor)) {
                TarArchiveEntry entry;
                while ((entry = tarInput.getNextTarEntry()) != null) {
                    File destFile = new File(destination, entry.getName());
                    String canonicalDestFile = destFile.getCanonicalPath();

                    // Security Check: Path Traversal (Zip Slip)
                    if (!canonicalDestFile.startsWith(canonicalDestPath + File.separator)) {
                        System.err.println("SECURITY: Skipping potentially malicious entry: " + entry.getName());
                        continue;
                    }
                    
                    // Security Check: Absolute paths
                    if (new File(entry.getName()).isAbsolute()) {
                        System.err.println("SECURITY: Skipping absolute path entry: " + entry.getName());
                        continue;
                    }

                    System.out.println("  -> " + entry.getName());

                    if (entry.isDirectory()) {
                        destFile.mkdirs();
                    } else {
                        // Ensure parent directories exist
                        destFile.getParentFile().mkdirs();
                        
                        try (OutputStream out = new FileOutputStream(destFile)) {
                            byte[] buffer = new byte[8192];
                            int len;
                            while ((len = tarInput.read(buffer)) != -1) {
                                out.write(buffer, 0, len);
                            }
                        }
                    }
                }
            }
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    private static void cleanDirectory(String path) throws IOException {
        File dir = new File(path);
        if (dir.exists() && dir.isDirectory()) {
            Files.walk(dir.toPath())
                 .sorted(Comparator.reverseOrder())
                 .map(Path::toFile)
                 .forEach(File::delete);
        }
    }

    public static void main(String[] args) {
        // --- Test Setup ---
        // These test archives must be created beforehand.
        // You can generate them using the provided Python script.
        String testDataBaseDir = "/tmp/lse51_test_data/";
        String[] testArchives = {
            testDataBaseDir + "test.tar",
            testDataBaseDir + "test.tar.gz",
            testDataBaseDir + "test.tar.bz2",
            testDataBaseDir + "malicious_traversal.tar",
            testDataBaseDir + "malicious_absolute.tar"
        };
        String unpackDir = "/tmp/unpack_java";

        System.out.println("--- Java Tar Extraction Tests ---");
        System.out.println("NOTE: Test archives are expected in " + testDataBaseDir);
        System.out.println("You can generate them by running the Python code in this file.\n");

        for (int i = 0; i < testArchives.length; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            try {
                cleanDirectory(unpackDir);
                extractTar(testArchives[i], unpackDir);
            } catch (IOException e) {
                System.err.println("Failed to clean directory: " + unpackDir);
                e.printStackTrace();
            }
        }
        
        System.out.println("\n--- Java Tests Complete ---");
    }
}