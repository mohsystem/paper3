import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.*;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;
import java.util.Comparator;

public class Task117 {

    /**
     * Extracts a zip file to a specified destination directory.
     * This method protects against path traversal attacks.
     *
     * @param zipFilePath   The path to the zip file.
     * @param destDirectory The directory where files will be extracted.
     * @return true if extraction was successful, false otherwise.
     */
    public static boolean extractZip(String zipFilePath, String destDirectory) {
        Path destDirPath = Paths.get(destDirectory).toAbsolutePath();

        // Create destination directory if it doesn't exist
        try {
            Files.createDirectories(destDirPath);
        } catch (IOException e) {
            System.err.println("Error creating destination directory: " + e.getMessage());
            return false;
        }

        try (ZipFile zipFile = new ZipFile(zipFilePath)) {
            Enumeration<? extends ZipEntry> entries = zipFile.entries();
            byte[] buffer = new byte[8192];

            while (entries.hasMoreElements()) {
                ZipEntry entry = entries.nextElement();
                Path entryPath = destDirPath.resolve(entry.getName()).normalize();

                // Security Check: Path Traversal
                if (!entryPath.startsWith(destDirPath)) {
                    System.err.println("Path traversal attempt detected. Skipping entry: " + entry.getName());
                    continue; // Skip this entry
                }
                
                if (entry.isDirectory()) {
                    Files.createDirectories(entryPath);
                } else {
                    // Ensure parent directory exists for files
                    Path parentDir = entryPath.getParent();
                    if (parentDir != null && !Files.exists(parentDir)) {
                        Files.createDirectories(parentDir);
                    }
                    
                    try (InputStream in = zipFile.getInputStream(entry);
                         FileOutputStream out = new FileOutputStream(entryPath.toFile())) {
                        int len;
                        while ((len = in.read(buffer)) > 0) {
                            out.write(buffer, 0, len);
                        }
                    }
                }
            }
        } catch (IOException e) {
            System.err.println("An error occurred during zip extraction: " + e.getMessage());
            return false;
        }
        return true;
    }

    // --- Test Cases ---
    
    private static void createTestZip(String zipFileName, boolean includeTraversal) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(zipFileName);
             ZipOutputStream zos = new ZipOutputStream(fos)) {
            
            // Add a normal file
            ZipEntry entry1 = new ZipEntry("file1.txt");
            zos.putNextEntry(entry1);
            zos.write("This is file 1.".getBytes());
            zos.closeEntry();

            // Add a file in a directory
            ZipEntry entry2 = new ZipEntry("dir1/file2.txt");
            zos.putNextEntry(entry2);
            zos.write("This is file 2.".getBytes());
            zos.closeEntry();
            
            // Add an empty directory
            ZipEntry entry3 = new ZipEntry("dir1/emptydir/");
            zos.putNextEntry(entry3);
            zos.closeEntry();

            if (includeTraversal) {
                // Add a path traversal attempt
                ZipEntry maliciousEntry = new ZipEntry("../../evil.txt");
                zos.putNextEntry(maliciousEntry);
                zos.write("malicious content".getBytes());
                zos.closeEntry();
            }
        }
    }
    
    private static void cleanup(Path... paths) throws IOException {
        for (Path path : paths) {
            if (Files.exists(path)) {
                if (Files.isDirectory(path)) {
                    Files.walk(path)
                        .sorted(Comparator.reverseOrder())
                        .map(Path::toFile)
                        .forEach(File::delete);
                } else {
                    Files.delete(path);
                }
            }
        }
    }
    
    public static void main(String[] args) {
        runTests();
    }
    
    private static void runTests() {
        System.out.println("--- Running Java ZIP Extraction Tests ---");
        Path tempDir = Paths.get("java_test_temp");
        
        try {
            // Setup
            Files.createDirectories(tempDir);
            Path testZip = tempDir.resolve("test.zip");
            Path traversalZip = tempDir.resolve("traversal.zip");
            createTestZip(testZip.toString(), false);
            createTestZip(traversalZip.toString(), true);

            // Test Case 1: Normal extraction
            System.out.println("\n[Test 1] Normal Extraction");
            Path out1 = tempDir.resolve("out1");
            boolean success1 = extractZip(testZip.toString(), out1.toString());
            System.out.println("Result: " + (success1 && Files.exists(out1.resolve("dir1/file2.txt")) ? "PASS" : "FAIL"));

            // Test Case 2: Extraction to a non-existent directory
            System.out.println("\n[Test 2] Extract to New Directory");
            Path out2 = tempDir.resolve("out2_new");
            boolean success2 = extractZip(testZip.toString(), out2.toString());
            System.out.println("Result: " + (success2 && Files.exists(out2.resolve("file1.txt")) ? "PASS" : "FAIL"));

            // Test Case 3: Path traversal attack
            System.out.println("\n[Test 3] Path Traversal Attack");
            Path out3 = tempDir.resolve("out3");
            extractZip(traversalZip.toString(), out3.toString());
            Path evilFile = tempDir.resolve("evil.txt");
            System.out.println("Result: " + (!Files.exists(evilFile) ? "PASS" : "FAIL"));

            // Test Case 4: Non-existent archive file
            System.out.println("\n[Test 4] Non-existent Archive");
            Path out4 = tempDir.resolve("out4");
            boolean success4 = extractZip("nonexistent.zip", out4.toString());
            System.out.println("Result: " + (!success4 ? "PASS" : "FAIL"));

            // Test Case 5: Output is a file
            System.out.println("\n[Test 5] Output is a File");
            Path outFile = tempDir.resolve("output_file.txt");
            Files.createFile(outFile);
            boolean success5 = extractZip(testZip.toString(), outFile.toString());
            // The method should fail because it can't create a directory at a path where a file exists
            System.out.println("Result: " + (!success5 ? "PASS" : "FAIL"));

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // Cleanup
            try {
                cleanup(tempDir);
            } catch (IOException e) {
                System.err.println("Cleanup failed: " + e.getMessage());
            }
        }
    }
}