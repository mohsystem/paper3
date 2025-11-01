import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;
import java.util.Comparator;

public class Lse50 {

    /**
     * Extracts all files from a ZIP archive to a destination directory.
     * This method protects against path traversal attacks (Zip Slip).
     *
     * @param zipFilePath The path to the ZIP file.
     * @param destDirPath The path to the directory where files will be extracted.
     * @return true if extraction was successful, false otherwise.
     */
    public static boolean extractArchive(String zipFilePath, String destDirPath) {
        Path destDir = Paths.get(destDirPath).toAbsolutePath().normalize();
        Path zipFile = Paths.get(zipFilePath);

        if (!Files.exists(zipFile)) {
            System.err.println("Error: ZIP file not found at " + zipFilePath);
            return false;
        }

        try {
            Files.createDirectories(destDir);
        } catch (IOException e) {
            System.err.println("Error: Could not create destination directory " + destDirPath);
            e.printStackTrace();
            return false;
        }

        byte[] buffer = new byte[4096];
        try (InputStream fis = Files.newInputStream(zipFile);
             ZipInputStream zis = new ZipInputStream(fis)) {

            ZipEntry zipEntry = zis.getNextEntry();
            while (zipEntry != null) {
                Path destFile = destDir.resolve(zipEntry.getName()).toAbsolutePath().normalize();

                // Path traversal vulnerability check
                if (!destFile.startsWith(destDir)) {
                    System.err.println("Path traversal attempt detected for entry: " + zipEntry.getName());
                    // Skip this entry and continue with the next one.
                    // Or you could throw an exception to stop the whole process.
                    // For this example, we will just skip.
                    zipEntry = zis.getNextEntry();
                    continue;
                }

                if (zipEntry.isDirectory()) {
                    Files.createDirectories(destFile);
                } else {
                    // Ensure parent directory exists
                    if (destFile.getParent() != null) {
                        Files.createDirectories(destFile.getParent());
                    }
                    
                    try (FileOutputStream fos = new FileOutputStream(destFile.toFile())) {
                        int len;
                        while ((len = zis.read(buffer)) > 0) {
                            fos.write(buffer, 0, len);
                        }
                    }
                }
                zis.closeEntry();
                zipEntry = zis.getNextEntry();
            }
        } catch (IOException e) {
            System.err.println("Error during ZIP extraction.");
            e.printStackTrace();
            return false;
        }
        return true;
    }

    /**
     * Creates a test zip file with various entry types for testing.
     * @param zipFilePath The path where the test zip will be created.
     */
    private static void createTestZip(String zipFilePath) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(zipFilePath);
             ZipOutputStream zos = new ZipOutputStream(fos)) {

            // 1. A normal file
            ZipEntry entry1 = new ZipEntry("test1.txt");
            zos.putNextEntry(entry1);
            zos.write("This is a test file.".getBytes(StandardCharsets.UTF_8));
            zos.closeEntry();

            // 2. A file in a subdirectory
            ZipEntry entry2 = new ZipEntry("dir1/test2.txt");
            zos.putNextEntry(entry2);
            zos.write("This is a nested file.".getBytes(StandardCharsets.UTF_8));
            zos.closeEntry();

            // 3. A path traversal attempt
            ZipEntry entry3 = new ZipEntry("../evil.txt");
            zos.putNextEntry(entry3);
            zos.write("This should not be extracted.".getBytes(StandardCharsets.UTF_8));
            zos.closeEntry();
            
            // 4. An absolute path attempt
            ZipEntry entry4 = new ZipEntry("/tmp/absolute.txt");
            zos.putNextEntry(entry4);
            zos.write("This should also not be extracted.".getBytes(StandardCharsets.UTF_8));
            zos.closeEntry();

            // 5. Another safe file
            ZipEntry entry5 = new ZipEntry("test3.txt");
            zos.putNextEntry(entry5);
            zos.write("This is another safe file.".getBytes(StandardCharsets.UTF_8));
            zos.closeEntry();
        }
    }

    /**
     * Deletes a directory and all its contents.
     * @param directoryPath Path to the directory to delete.
     */
    private static void deleteDirectory(Path directoryPath) {
        if (!Files.exists(directoryPath)) {
            return;
        }
        try {
            Files.walk(directoryPath)
                 .sorted(Comparator.reverseOrder())
                 .map(Path::toFile)
                 .forEach(File::delete);
        } catch (IOException e) {
            System.err.println("Failed to delete directory: " + directoryPath);
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        String zipFileName = "archive.zip";
        String unpackDir = "/tmp/unpack_java";
        Path zipFilePath = Paths.get(zipFileName);
        Path unpackPath = Paths.get(unpackDir);

        // --- Test Case 1: Standard Extraction ---
        System.out.println("--- Test Case 1: Standard Extraction ---");
        try {
            createTestZip(zipFileName);
            System.out.println("Created test zip: " + zipFileName);

            boolean success = extractArchive(zipFileName, unpackDir);
            System.out.println("Extraction successful: " + success);

            // Verify results
            System.out.println("Exists '" + unpackDir + "/test1.txt': " + Files.exists(Paths.get(unpackDir, "test1.txt")));
            System.out.println("Exists '" + unpackDir + "/dir1/test2.txt': " + Files.exists(Paths.get(unpackDir, "dir1", "test2.txt")));
            System.out.println("Exists '" + unpackDir + "/test3.txt': " + Files.exists(Paths.get(unpackDir, "test3.txt")));
            System.out.println("Exists '../evil.txt' relative to unpack dir: " + Files.exists(Paths.get(unpackDir, "../evil.txt")));
            System.out.println("Exists '/tmp/absolute.txt': " + Files.exists(Paths.get("/tmp/absolute.txt")));
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // Cleanup
            deleteDirectory(unpackPath);
            try { Files.deleteIfExists(zipFilePath); } catch (IOException e) { /* ignore */ }
            try { Files.deleteIfExists(Paths.get(unpackDir, "../evil.txt")); } catch (IOException e) { /* ignore */ }
            try { Files.deleteIfExists(Paths.get("/tmp/absolute.txt")); } catch (IOException e) { /* ignore */ }
            System.out.println("Cleaned up test files and directories.");
            System.out.println();
        }

        // --- Test Case 2: Non-existent ZIP file ---
        System.out.println("--- Test Case 2: Non-existent ZIP file ---");
        boolean success2 = extractArchive("nonexistent.zip", unpackDir);
        System.out.println("Extraction successful: " + success2);
        System.out.println();

        // --- Test Case 3: Destination is a file (should fail) ---
        System.out.println("--- Test Case 3: Destination is a file ---");
        Path dummyFile = Paths.get("dummy_file.txt");
        try {
            Files.createFile(dummyFile);
            createTestZip(zipFileName);
            boolean success3 = extractArchive(zipFileName, dummyFile.toString());
            System.out.println("Extraction successful: " + success3);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try { Files.deleteIfExists(dummyFile); } catch (IOException e) { /* ignore */ }
            try { Files.deleteIfExists(zipFilePath); } catch (IOException e) { /* ignore */ }
            System.out.println("Cleaned up.");
            System.out.println();
        }

        // Test cases 4 and 5 are implicitly handled by the malicious zip created in Test 1.
        System.out.println("--- Test Cases 4 & 5: Path Traversal and Absolute Paths are handled in Test 1 ---");
    }
}