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
     * Unzips a specified zip file to a destination directory.
     *
     * @param zipFilePath   The path to the zip file.
     * @param destDirectory The directory to extract files to.
     * @throws IOException if an I/O error occurs.
     */
    public static void unzip(String zipFilePath, String destDirectory) throws IOException {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        byte[] buffer = new byte[1024];
        try (ZipInputStream zis = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry zipEntry = zis.getNextEntry();
            while (zipEntry != null) {
                File newFile = newFile(destDir, zipEntry);
                if (zipEntry.isDirectory()) {
                    if (!newFile.isDirectory() && !newFile.mkdirs()) {
                        throw new IOException("Failed to create directory " + newFile);
                    }
                } else {
                    // Fix for Windows-created archives
                    File parent = newFile.getParentFile();
                    if (!parent.isDirectory() && !parent.mkdirs()) {
                        throw new IOException("Failed to create directory " + parent);
                    }

                    try (FileOutputStream fos = new FileOutputStream(newFile)) {
                        int len;
                        while ((len = zis.read(buffer)) > 0) {
                            fos.write(buffer, 0, len);
                        }
                    }
                }
                zipEntry = zis.getNextEntry();
            }
            zis.closeEntry();
        }
    }

    /**
     * Helper to prevent path traversal vulnerability.
     */
    private static File newFile(File destinationDir, ZipEntry zipEntry) throws IOException {
        File destFile = new File(destinationDir, zipEntry.getName());
        String destDirPath = destinationDir.getCanonicalPath();
        String destFilePath = destFile.getCanonicalPath();
        if (!destFilePath.startsWith(destDirPath + File.separator)) {
            throw new IOException("Entry is outside of the target dir: " + zipEntry.getName());
        }
        return destFile;
    }

    // --- Test Case Helper Methods ---

    /**
     * Creates a dummy zip file for testing purposes.
     */
    private static void createTestZip(String zipFilePath, int testNum) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(zipFilePath);
             ZipOutputStream zos = new ZipOutputStream(fos)) {

            String fileName = "test_file_" + testNum + ".txt";
            String content = "This is content for test case " + testNum;
            
            ZipEntry entry = new ZipEntry(fileName);
            zos.putNextEntry(entry);
            zos.write(content.getBytes());
            zos.closeEntry();

            String dirFileName = "subdir/test_file_" + testNum + ".txt";
            ZipEntry dirEntry = new ZipEntry(dirFileName);
            zos.putNextEntry(dirEntry);
            zos.write(content.getBytes());
            zos.closeEntry();
        }
    }

    /**
     * Deletes a directory recursively.
     */
    private static void deleteDirectory(Path path) {
        try {
            if (Files.exists(path)) {
                Files.walk(path)
                    .sorted(Comparator.reverseOrder())
                    .map(Path::toFile)
                    .forEach(File::delete);
            }
        } catch (IOException e) {
            System.err.println("Failed to delete directory: " + path + " - " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        String zipFileName = "archive.zip";
        String extractDirName = "/tmp/unpack";

        for (int i = 1; i <= 5; i++) {
            System.out.println("--- Running Test Case " + i + " ---");
            Path zipPath = Paths.get(zipFileName);
            Path extractPath = Paths.get(extractDirName);
            boolean success = true;

            try {
                // 1. Setup
                deleteDirectory(extractPath);
                Files.deleteIfExists(zipPath);
                createTestZip(zipFileName, i);
                System.out.println("Created " + zipFileName + " for test " + i);

                // 2. Execute
                unzip(zipFileName, extractDirName);
                System.out.println("Unzipped " + zipFileName + " to " + extractDirName);

                // 3. Verify
                Path file1 = extractPath.resolve("test_file_" + i + ".txt");
                Path file2 = extractPath.resolve("subdir/test_file_" + i + ".txt");
                if (!Files.exists(file1) || !Files.exists(file2)) {
                    System.err.println("Verification failed: Extracted files not found.");
                    success = false;
                } else {
                    String content = new String(Files.readAllBytes(file1));
                    if (!content.equals("This is content for test case " + i)) {
                         System.err.println("Verification failed: File content mismatch.");
                         success = false;
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
                success = false;
            } finally {
                // 4. Cleanup
                deleteDirectory(extractPath);
                try {
                    Files.deleteIfExists(zipPath);
                } catch (IOException e) { /* ignore */ }
                
                if(success) {
                    System.out.println("Test Case " + i + ": PASS");
                } else {
                    System.out.println("Test Case " + i + ": FAIL");
                }
            }
        }
    }
}