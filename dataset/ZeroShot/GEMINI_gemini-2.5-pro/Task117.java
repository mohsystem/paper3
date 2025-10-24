import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Comparator;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

public class Task117 {

    /**
     * Extracts a ZIP file to a specified destination directory with security checks.
     *
     * @param zipFilePath   Path to the ZIP file.
     * @param destDirPath   Path to the destination directory.
     * @throws IOException if an I/O error occurs or a security violation is detected.
     */
    public static void extractZip(String zipFilePath, String destDirPath) throws IOException {
        Path destDir = Paths.get(destDirPath).toAbsolutePath();
        if (!Files.exists(destDir)) {
            Files.createDirectories(destDir);
        }

        try (ZipInputStream zis = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry zipEntry = zis.getNextEntry();
            byte[] buffer = new byte[1024];

            while (zipEntry != null) {
                Path newFilePath = destDir.resolve(zipEntry.getName()).normalize();

                // Security Check: Path Traversal (Zip Slip)
                if (!newFilePath.startsWith(destDir)) {
                    throw new IOException("Zip Slip vulnerability detected: Entry is outside of the target dir: " + zipEntry.getName());
                }

                if (zipEntry.isDirectory()) {
                    if (!Files.exists(newFilePath)) {
                        Files.createDirectories(newFilePath);
                    }
                } else {
                    // Create parent directories if they don't exist
                    Path parent = newFilePath.getParent();
                    if (parent != null && !Files.exists(parent)) {
                        Files.createDirectories(parent);
                    }

                    // Write file content
                    try (FileOutputStream fos = new FileOutputStream(newFilePath.toFile())) {
                        int len;
                        while ((len = zis.read(buffer)) > 0) {
                            fos.write(buffer, 0, len);
                        }
                    }
                }
                zis.closeEntry();
                zipEntry = zis.getNextEntry();
            }
        }
    }

    /**
     * Helper method to create a test zip file.
     */
    private static void createTestZip(String zipFilePath) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipFilePath))) {
            // Add a normal file
            zos.putNextEntry(new ZipEntry("test_file1.txt"));
            zos.write("This is file 1.".getBytes());
            zos.closeEntry();

            // Add a file in a subdirectory
            zos.putNextEntry(new ZipEntry("subdir/test_file2.txt"));
            zos.write("This is file 2 in a subdir.".getBytes());
            zos.closeEntry();

            // Add an empty directory
            zos.putNextEntry(new ZipEntry("empty_dir/"));
            zos.closeEntry();
            
            // Add a file with a path traversal attempt (for testing security)
            zos.putNextEntry(new ZipEntry("../malicious.txt"));
            zos.write("This should not be extracted.".getBytes());
            zos.closeEntry();
        }
    }

    /**
     * Helper method to delete a directory recursively.
     */
    private static void deleteDirectory(Path path) throws IOException {
        if (Files.exists(path)) {
            Files.walk(path)
                 .sorted(Comparator.reverseOrder())
                 .map(Path::toFile)
                 .forEach(File::delete);
        }
    }

    public static void main(String[] args) {
        String testDir = "java_test_area";
        try {
            // Setup test environment
            Path testDirPath = Paths.get(testDir);
            deleteDirectory(testDirPath); // Clean up previous runs
            Files.createDirectories(testDirPath);

            String validZip = testDirPath.resolve("test.zip").toString();
            String maliciousZip = testDirPath.resolve("malicious.zip").toString(); // Conceptually, this would be the same file for the test
            createTestZip(validZip);

            // Test Case 1: Standard successful extraction
            System.out.println("--- Test Case 1: Standard Extraction ---");
            String dest1 = testDirPath.resolve("output1").toString();
            try {
                extractZip(validZip, dest1);
                System.out.println("Successfully extracted to: " + dest1);
                if(Files.exists(Paths.get(dest1, "test_file1.txt")) && Files.exists(Paths.get(dest1, "subdir", "test_file2.txt"))){
                    System.out.println("Verification: PASSED");
                } else {
                    System.out.println("Verification: FAILED");
                }
            } catch (IOException e) {
                System.err.println("Extraction failed: " + e.getMessage());
            }

            // Test Case 2: Extracting a non-existent ZIP file
            System.out.println("\n--- Test Case 2: Non-existent ZIP File ---");
            String dest2 = testDirPath.resolve("output2").toString();
            try {
                extractZip("non_existent.zip", dest2);
                System.out.println("Extraction successful (this should not happen).");
            } catch (IOException e) {
                System.out.println("Caught expected exception: " + e.getMessage());
            }

            // Test Case 3: Path Traversal (Zip Slip) security test
            System.out.println("\n--- Test Case 3: Path Traversal (Zip Slip) Test ---");
            String dest3 = testDirPath.resolve("output3").toString();
            try {
                extractZip(validZip, dest3); // The same zip contains a malicious entry
                System.out.println("Extraction completed.");
                 if(Files.exists(Paths.get(testDirPath.toString(), "malicious.txt"))){
                    System.out.println("Verification: FAILED - Malicious file was created.");
                } else {
                    System.out.println("Verification: PASSED - Malicious file was blocked.");
                }
            } catch (IOException e) {
                System.out.println("Caught expected security exception: " + e.getMessage());
            }

            // Test Case 4: Destination is an existing file (should fail gracefully)
             System.out.println("\n--- Test Case 4: Destination is a File ---");
            Path dest4File = testDirPath.resolve("output4");
            Files.createFile(dest4File);
            try {
                extractZip(validZip, dest4File.toString());
                System.out.println("Extraction successful (this should not happen).");
            } catch (IOException e) {
                System.out.println("Caught expected exception: " + e.getMessage());
            }

            // Test Case 5: Empty source zip file path
            System.out.println("\n--- Test Case 5: Empty Source Path ---");
             String dest5 = testDirPath.resolve("output5").toString();
            try {
                extractZip("", dest5);
                System.out.println("Extraction successful (this should not happen).");
            } catch (Exception e) {
                System.out.println("Caught expected exception: " + e.getMessage());
            }

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // Clean up
            try {
                deleteDirectory(Paths.get(testDir));
                System.out.println("\nCleaned up test directory.");
            } catch (IOException e) {
                System.err.println("Failed to clean up test directory: " + e.getMessage());
            }
        }
    }
}