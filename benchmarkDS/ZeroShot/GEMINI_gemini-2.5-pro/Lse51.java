import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;

// Required external library: Apache Commons Compress
// Add to your build configuration (e.g., pom.xml for Maven):
// <dependency>
//     <groupId>org.apache.commons</groupId>
//     <artifactId>commons-compress</artifactId>
//     <version>1.26.1</version>
// </dependency>
import org.apache.commons.compress.archivers.tar.TarArchiveEntry;
import org.apache.commons.compress.archivers.tar.TarArchiveInputStream;
import org.apache.commons.compress.archivers.tar.TarArchiveOutputStream;
import org.apache.commons.compress.utils.IOUtils;


public class Lse51 {

    /**
     * Securely extracts a tar archive to a specified directory.
     * Prevents path traversal attacks by ensuring that all extracted files are within the destination directory.
     *
     * @param tarPath     The path to the .tar file.
     * @param extractPath The destination directory for extraction.
     * @return true if extraction was successful, false otherwise.
     */
    public static boolean secureUntar(String tarPath, String extractPath) {
        Path destination = Paths.get(extractPath).toAbsolutePath();
        Path tarFile = Paths.get(tarPath);

        if (!Files.exists(tarFile)) {
            System.err.println("Error: TAR file not found at " + tarPath);
            return false;
        }

        // The 'try-with-resources' statement is the Java equivalent of Python's 'with'
        try (InputStream is = new FileInputStream(tarFile.toFile());
             TarArchiveInputStream tarIn = new TarArchiveInputStream(is)) {
            
            TarArchiveEntry entry;
            while ((entry = tarIn.getNextTarEntry()) != null) {
                Path entryPath = destination.resolve(entry.getName()).normalize();

                // Security check: ensure the resolved path is still inside the destination directory
                if (!entryPath.startsWith(destination)) {
                    System.err.println("Security Error: Malicious entry detected (Path Traversal): " + entry.getName());
                    return false;
                }

                if (entry.isDirectory()) {
                    Files.createDirectories(entryPath);
                } else {
                    // Create parent directories if they don't exist
                    Path parent = entryPath.getParent();
                    if (parent != null) {
                        Files.createDirectories(parent);
                    }
                    // The inner 'try-with-resources' ensures the output stream is closed for each file
                    try (OutputStream out = new FileOutputStream(entryPath.toFile())) {
                        IOUtils.copy(tarIn, out);
                    }
                }
            }
            return true;
        } catch (IOException e) {
            System.err.println("Error during TAR extraction: " + e.getMessage());
            return false;
        }
    }

    // --- Test Case Setup ---

    private static final String TEST_DIR = "/tmp/unpack_java_test";
    
    private static void setupTestEnvironment() throws IOException {
        cleanupTestEnvironment(); // Clean up from previous runs
        Files.createDirectories(Paths.get(TEST_DIR));
        
        // 1. Create a valid tar file
        Map<String, String> goodFiles = new HashMap<>();
        goodFiles.put("file1.txt", "This is file 1.");
        goodFiles.put("dir1/file2.txt", "This is file 2 in a directory.");
        createTestTar("good.tar", goodFiles);

        // 2. Create a tar file with a path traversal attempt
        Map<String, String> traversalFiles = new HashMap<>();
        traversalFiles.put("../../tmp/evil.txt", "malicious content");
        createTestTar("bad_traversal.tar", traversalFiles);
        
        // 3. Create a tar file with an absolute path attempt
        Map<String, String> absoluteFiles = new HashMap<>();
        absoluteFiles.put("/tmp/evil_absolute.txt", "malicious absolute content");
        createTestTar("bad_absolute.tar", absoluteFiles);

        // 5. Create an empty tar file
        createTestTar("empty.tar", new HashMap<>());
    }
    
    private static void createTestTar(String archiveName, Map<String, String> filesToAdd) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(archiveName);
             TarArchiveOutputStream taos = new TarArchiveOutputStream(fos)) {
            taos.setLongFileMode(TarArchiveOutputStream.LONGFILE_GNU);
            for (Map.Entry<String, String> entry : filesToAdd.entrySet()) {
                TarArchiveEntry tarEntry = new TarArchiveEntry(entry.getKey());
                byte[] data = entry.getValue().getBytes("UTF-8");
                tarEntry.setSize(data.length);
                taos.putArchiveEntry(tarEntry);
                taos.write(data);
                taos.closeArchiveEntry();
            }
        }
    }
    
    private static void cleanupTestEnvironment() {
        try {
            Path dirPath = Paths.get(TEST_DIR);
            if (Files.exists(dirPath)) {
                Files.walk(dirPath)
                    .sorted(Comparator.reverseOrder())
                    .map(Path::toFile)
                    .forEach(File::delete);
            }
            Files.deleteIfExists(Paths.get("good.tar"));
            Files.deleteIfExists(Paths.get("bad_traversal.tar"));
            Files.deleteIfExists(Paths.get("bad_absolute.tar"));
            Files.deleteIfExists(Paths.get("empty.tar"));
        } catch (IOException e) {
            // Suppress cleanup errors
        }
    }
    
    public static void main(String[] args) {
        try {
            setupTestEnvironment();
            System.out.println("--- Running Java Test Cases ---");

            // Test Case 1: Valid tar file
            System.out.println("\n[Test 1] Extracting a valid TAR file...");
            boolean result1 = secureUntar("good.tar", TEST_DIR);
            System.out.println("Result: " + (result1 ? "SUCCESS" : "FAILURE"));
            assert result1;
            assert Files.exists(Paths.get(TEST_DIR, "file1.txt"));

            // Test Case 2: Path traversal attack
            System.out.println("\n[Test 2] Extracting a TAR with path traversal...");
            boolean result2 = secureUntar("bad_traversal.tar", TEST_DIR);
            System.out.println("Result: " + (!result2 ? "SUCCESS (Attack Blocked)" : "FAILURE (Vulnerable)"));
            assert !result2;
            assert !Files.exists(Paths.get("/tmp/evil.txt"));

            // Test Case 3: Absolute path attack
            System.out.println("\n[Test 3] Extracting a TAR with an absolute path...");
            boolean result3 = secureUntar("bad_absolute.tar", TEST_DIR);
            System.out.println("Result: " + (!result3 ? "SUCCESS (Attack Blocked)" : "FAILURE (Vulnerable)"));
            assert !result3;
            assert !Files.exists(Paths.get("/tmp/evil_absolute.txt"));

            // Test Case 4: Non-existent file
            System.out.println("\n[Test 4] Attempting to extract a non-existent TAR file...");
            boolean result4 = secureUntar("non_existent.tar", TEST_DIR);
            System.out.println("Result: " + (!result4 ? "SUCCESS (Handled gracefully)" : "FAILURE"));
            assert !result4;

            // Test Case 5: Empty tar file
            System.out.println("\n[Test 5] Extracting an empty TAR file...");
            boolean result5 = secureUntar("empty.tar", TEST_DIR);
            System.out.println("Result: " + (result5 ? "SUCCESS" : "FAILURE"));
            assert result5;

        } catch (IOException e) {
            System.err.println("An error occurred during test setup: " + e.getMessage());
        } finally {
            cleanupTestEnvironment();
            System.out.println("\n--- Java Tests Complete, Cleanup Done ---");
        }
    }
}