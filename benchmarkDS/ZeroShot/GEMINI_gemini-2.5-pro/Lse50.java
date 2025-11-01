import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Comparator;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

public class Lse50 {

    /**
     * Securely extracts a zip file to a specified directory, preventing path traversal attacks.
     * The prompt mentions opening "archive.zip as a tarfile", which is technically incorrect.
     * This implementation interprets the request as extracting a standard ZIP archive.
     *
     * @param zipFilePath   Path to the .zip file.
     * @param destDirectory Path to the destination directory.
     * @return true if extraction was successful, false otherwise.
     */
    public static boolean extractZipSecure(String zipFilePath, String destDirectory) {
        File destDir = new File(destDirectory);
        // Create destination directory if it doesn't exist
        if (!destDir.exists() && !destDir.mkdirs()) {
            System.err.println("Error: Could not create destination directory: " + destDirectory);
            return false;
        }

        // Ensure the destination is a directory
        if (!destDir.isDirectory()) {
            System.err.println("Error: Destination path is not a directory: " + destDirectory);
            return false;
        }

        byte[] buffer = new byte[4096];
        try (ZipInputStream zis = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry zipEntry = zis.getNextEntry();
            String destDirPath = destDir.getCanonicalPath();

            while (zipEntry != null) {
                File newFile = new File(destDirectory, zipEntry.getName());
                
                // Security check for Path Traversal (Zip Slip)
                String destFilePath = newFile.getCanonicalPath();
                if (!destFilePath.startsWith(destDirPath + File.separator)) {
                    System.err.println("Security Error: Entry is outside of the target dir: " + zipEntry.getName());
                    zipEntry = zis.getNextEntry();
                    continue;
                }

                if (zipEntry.isDirectory()) {
                    if (!newFile.isDirectory() && !newFile.mkdirs()) {
                        throw new IOException("Failed to create directory " + newFile);
                    }
                } else {
                    // Create parent directories
                    File parent = newFile.getParentFile();
                    if (!parent.isDirectory() && !parent.mkdirs()) {
                        throw new IOException("Failed to create directory " + parent);
                    }
                    
                    // Write file content
                    try (FileOutputStream fos = new FileOutputStream(newFile)) {
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
            System.err.println("Error during zip extraction: " + e.getMessage());
            return false;
        }
        return true;
    }

    // Helper method to create a zip file for testing purposes
    private static void createTestZip(String zipFilePath, String[] fileEntries) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipFilePath))) {
            for (String entry : fileEntries) {
                ZipEntry zipEntry = new ZipEntry(entry);
                zos.putNextEntry(zipEntry);
                if (!entry.endsWith("/")) { // It's a file, not a directory
                    byte[] data = ("content of " + entry).getBytes();
                    zos.write(data, 0, data.length);
                }
                zos.closeEntry();
            }
        }
    }
    
    // Helper method to delete a directory recursively
    private static void deleteDirectory(File directory) throws IOException {
        if (directory.exists()) {
            Files.walk(directory.toPath())
                .sorted(Comparator.reverseOrder())
                .map(Path::toFile)
                .forEach(File::delete);
        }
    }

    public static void main(String[] args) throws IOException {
        String tempDir = System.getProperty("java.io.tmpdir");
        Path testRoot = Paths.get(tempDir, "lse50_java_test");
        
        // Clean up previous runs
        deleteDirectory(testRoot.toFile());
        Files.createDirectories(testRoot);

        try {
            // Test Case 1: Normal, safe extraction
            System.out.println("--- Test Case 1: Normal Extraction ---");
            String safeZip = testRoot.resolve("archive_safe.zip").toString();
            String safeDest = testRoot.resolve("unpack_safe").toString();
            createTestZip(safeZip, new String[]{"file1.txt", "dir1/", "dir1/file2.txt"});
            boolean success1 = extractZipSecure(safeZip, safeDest);
            System.out.println("Extraction successful: " + success1);
            System.out.println("Exists 'unpack_safe/dir1/file2.txt': " + Files.exists(Paths.get(safeDest, "dir1", "file2.txt")));
            System.out.println();

            // Test Case 2: Path Traversal attack
            System.out.println("--- Test Case 2: Path Traversal Attack ---");
            String evilZip = testRoot.resolve("archive_evil.zip").toString();
            String evilDest = testRoot.resolve("unpack_evil").toString();
            // This path attempts to write a file outside the target directory
            createTestZip(evilZip, new String[]{"../../evil.txt"});
            boolean success2 = extractZipSecure(evilZip, evilDest);
            System.out.println("Extraction reported success (should handle gracefully): " + success2);
            System.out.println("Malicious file created: " + Files.exists(testRoot.resolve("../evil.txt")));
            System.out.println();
            
            // Test Case 3: Absolute Path attack
            System.out.println("--- Test Case 3: Absolute Path Attack ---");
            String absZip = testRoot.resolve("archive_abs.zip").toString();
            String absDest = testRoot.resolve("unpack_abs").toString();
            String absolutePathEntry = new File(tempDir, "absolute.txt").getAbsolutePath();
            createTestZip(absZip, new String[]{absolutePathEntry});
            boolean success3 = extractZipSecure(absZip, absDest);
            System.out.println("Extraction reported success (should handle gracefully): " + success3);
            System.out.println("Malicious absolute file created: " + Files.exists(Paths.get(absolutePathEntry)));
            System.out.println();

            // Test Case 4: Zip file not found
            System.out.println("--- Test Case 4: Zip File Not Found ---");
            String nonExistentZip = testRoot.resolve("nonexistent.zip").toString();
            String dest4 = testRoot.resolve("unpack_4").toString();
            boolean success4 = extractZipSecure(nonExistentZip, dest4);
            System.out.println("Extraction successful: " + success4);
            System.out.println();

            // Test Case 5: Destination is a file, not a directory
            System.out.println("--- Test Case 5: Destination is a File ---");
            String dest5 = testRoot.resolve("destination_is_file.txt").toString();
            Files.createFile(Paths.get(dest5));
            boolean success5 = extractZipSecure(safeZip, dest5);
            System.out.println("Extraction successful: " + success5);
            System.out.println();

        } finally {
             // Cleanup
            System.out.println("--- Cleaning up test files ---");
            deleteDirectory(testRoot.toFile());
            System.out.println("Cleanup complete.");
        }
    }
}