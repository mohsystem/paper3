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

public class Lse49 {

    /**
     * Unzips a zip file to a specified destination directory, preventing path traversal attacks.
     *
     * @param zipFilePath The path to the zip file.
     * @param destDirPath The path to the destination directory.
     * @throws IOException if an I/O error occurs.
     */
    public static void unzip(final String zipFilePath, final String destDirPath) throws IOException {
        File destDir = new File(destDirPath);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        
        String destDirCanonicalPath = destDir.getCanonicalPath();

        byte[] buffer = new byte[1024];
        try (ZipInputStream zis = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry zipEntry = zis.getNextEntry();
            while (zipEntry != null) {
                File destFile = new File(destDir, zipEntry.getName());
                
                // Security check for Path Traversal (Zip Slip)
                String destFileCanonicalPath = destFile.getCanonicalPath();
                if (!destFileCanonicalPath.startsWith(destDirCanonicalPath + File.separator)) {
                    throw new IOException("Attempted Path Traversal attack: " + zipEntry.getName());
                }

                if (zipEntry.isDirectory()) {
                    if (!destFile.isDirectory() && !destFile.mkdirs()) {
                        throw new IOException("Failed to create directory " + destFile);
                    }
                } else {
                    // Create parent directories if they don't exist
                    File parent = destFile.getParentFile();
                    if (!parent.isDirectory() && !parent.mkdirs()) {
                        throw new IOException("Failed to create directory " + parent);
                    }

                    // Write file content
                    try (FileOutputStream fos = new FileOutputStream(destFile)) {
                        int len;
                        while ((len = zis.read(buffer)) > 0) {
                            fos.write(buffer, 0, len);
                        }
                    }
                }
                zipEntry = zis.getNextEntry();
            }
        }
    }

    // Main method with test cases
    public static void main(String[] args) {
        String zipFileName = "archive.zip";
        String unpackDir = "/tmp/unpack";

        // Test cases
        for (int i = 1; i <= 5; i++) {
            System.out.println("--- Running Test Case " + i + " ---");
            try {
                // Setup: Create a test zip file
                createTestZip(zipFileName, i);
                System.out.println("Created " + zipFileName);

                // Execute the unzip function
                unzip(zipFileName, unpackDir);
                System.out.println("Successfully unzipped to " + unpackDir);

                // Verify (simple check)
                Path p1 = Paths.get(unpackDir, "file1.txt");
                Path p2 = Paths.get(unpackDir, "dir1", "file2.txt");
                Path p3 = Paths.get("/tmp/evil.txt"); // Path for traversal attempt

                if(Files.exists(p1)) System.out.println("Verified: " + p1 + " exists.");
                if(Files.exists(p2)) System.out.println("Verified: " + p2 + " exists.");
                if(Files.exists(p3)) {
                     System.err.println("SECURITY VULNERABILITY: Traversal file created at " + p3);
                } else {
                     System.out.println("Security check passed: Traversal file was not created.");
                }

            } catch (IOException e) {
                System.err.println("Caught expected exception: " + e.getMessage());
            } finally {
                // Teardown: Clean up created files and directories
                System.out.println("Cleaning up...");
                cleanup(zipFileName, unpackDir);
                System.out.println("---------------------------\n");
            }
        }
    }
    
    // Helper to create a test zip file
    private static void createTestZip(String zipFileName, int testCase) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(zipFileName);
             ZipOutputStream zos = new ZipOutputStream(fos)) {
            
            // Case 1: Normal files
            addFileToZip(zos, "file1.txt", "content of file1");
            addFileToZip(zos, "dir1/file2.txt", "content of file2");

            // Case 2-5: Add a malicious entry for some tests
            if (testCase > 1) {
                 addFileToZip(zos, "../../evil.txt", "malicious content");
            }
        }
    }
    
    // Helper to add a file entry to a zip stream
    private static void addFileToZip(ZipOutputStream zos, String path, String content) throws IOException {
        ZipEntry entry = new ZipEntry(path);
        zos.putNextEntry(entry);
        zos.write(content.getBytes());
        zos.closeEntry();
    }

    // Helper to clean up test files and directories
    private static void cleanup(String zipFileName, String unpackDir) {
        try {
            Files.deleteIfExists(Paths.get(zipFileName));
            Path unpackPath = Paths.get(unpackDir);
            if (Files.exists(unpackPath)) {
                Files.walk(unpackPath)
                    .sorted(Comparator.reverseOrder())
                    .map(Path::toFile)
                    .forEach(File::delete);
            }
            Files.deleteIfExists(Paths.get("/tmp/evil.txt")); // Clean up potential traversal file
        } catch (IOException e) {
            System.err.println("Cleanup failed: " + e.getMessage());
        }
    }
}