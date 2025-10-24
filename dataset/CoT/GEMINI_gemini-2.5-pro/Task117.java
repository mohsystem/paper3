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
     * Extracts a ZIP archive to a specified destination directory, preventing path traversal attacks.
     * Note: This implementation is for ZIP files. TAR file extraction requires an external library
     * like Apache Commons Compress.
     *
     * @param zipFilePath   The path to the ZIP file.
     * @param destDirectory The directory where files will be extracted.
     * @return true if extraction was successful, false otherwise.
     */
    public static boolean extractArchive(String zipFilePath, String destDirectory) {
        File destDir = new File(destDirectory);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }

        String destDirCanonicalPath;
        try {
            destDirCanonicalPath = destDir.getCanonicalPath();
        } catch (IOException e) {
            System.err.println("Error getting canonical path for destination directory: " + e.getMessage());
            return false;
        }

        byte[] buffer = new byte[1024];
        try (ZipInputStream zis = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry zipEntry = zis.getNextEntry();
            while (zipEntry != null) {
                File newFile = new File(destDirectory, zipEntry.getName());

                // Security check for Path Traversal (Zip Slip)
                String destFileCanonicalPath = newFile.getCanonicalPath();
                if (!destFileCanonicalPath.startsWith(destDirCanonicalPath + File.separator)) {
                    System.err.println("Path Traversal attempt detected. Skipping entry: " + zipEntry.getName());
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
            System.err.println("Error during extraction: " + e.getMessage());
            return false;
        }
        return true;
    }

    // Helper methods for testing
    private static void createTestZip(String zipFilePath, boolean malicious) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipFilePath))) {
            // Add a directory
            ZipEntry dirEntry = new ZipEntry("dir1/");
            zos.putNextEntry(dirEntry);
            zos.closeEntry();
            
            // Add a normal file
            ZipEntry fileEntry1 = new ZipEntry("dir1/file1.txt");
            zos.putNextEntry(fileEntry1);
            zos.write("This is file 1.".getBytes());
            zos.closeEntry();

            // Add a top-level file
            ZipEntry fileEntry2 = new ZipEntry("file2.txt");
            zos.putNextEntry(fileEntry2);
            zos.write("This is file 2.".getBytes());
            zos.closeEntry();

            if (malicious) {
                // Add a malicious entry for path traversal
                ZipEntry maliciousEntry = new ZipEntry("../../malicious.txt");
                zos.putNextEntry(maliciousEntry);
                zos.write("This file should not be created.".getBytes());
                zos.closeEntry();
            }
        }
    }

    private static void deleteDirectory(File directory) throws IOException {
        if (directory.exists()) {
            Files.walk(directory.toPath())
                .sorted(Comparator.reverseOrder())
                .map(Path::toFile)
                .forEach(File::delete);
        }
    }

    public static void main(String[] args) throws IOException {
        String baseTestDir = "java_test_area";
        File baseDir = new File(baseTestDir);
        if (baseDir.exists()) {
            deleteDirectory(baseDir);
        }
        baseDir.mkdirs();

        try {
            // Test Case 1: Normal ZIP extraction
            System.out.println("--- Test Case 1: Normal ZIP Extraction ---");
            String normalZip = baseTestDir + File.separator + "normal.zip";
            String normalExtractDir = baseTestDir + File.separator + "normal_extract";
            createTestZip(normalZip, false);
            boolean success1 = extractArchive(normalZip, normalExtractDir);
            System.out.println("Extraction successful: " + success1);
            System.out.println("Exists 'normal_extract/dir1/file1.txt': " + new File(normalExtractDir, "dir1/file1.txt").exists());
            System.out.println();

            // Test Case 2: Malicious ZIP extraction (Path Traversal)
            System.out.println("--- Test Case 2: Malicious ZIP Extraction ---");
            String maliciousZip = baseTestDir + File.separator + "malicious.zip";
            String maliciousExtractDir = baseTestDir + File.separator + "malicious_extract";
            createTestZip(maliciousZip, true);
            boolean success2 = extractArchive(maliciousZip, maliciousExtractDir);
            System.out.println("Extraction successful (operation completes): " + success2);
            System.out.println("Malicious file created outside 'java_test_area': " + new File("malicious.txt").exists());
            System.out.println();

            // Test Case 3: Extracting to a non-existent directory
            System.out.println("--- Test Case 3: Extract to Non-Existent Directory ---");
            String newDirExtract = baseTestDir + File.separator + "new_dir_extract";
            boolean success3 = extractArchive(normalZip, newDirExtract);
            System.out.println("Extraction successful: " + success3);
            System.out.println("Exists 'new_dir_extract/file2.txt': " + new File(newDirExtract, "file2.txt").exists());
            System.out.println();
            
            // Test Case 4: Extracting an empty archive
            System.out.println("--- Test Case 4: Extract Empty Archive ---");
            String emptyZip = baseTestDir + File.separator + "empty.zip";
            String emptyExtractDir = baseTestDir + File.separator + "empty_extract";
            new ZipOutputStream(new FileOutputStream(emptyZip)).close();
            boolean success4 = extractArchive(emptyZip, emptyExtractDir);
            System.out.println("Extraction successful: " + success4);
            System.out.println("Directory created: " + new File(emptyExtractDir).exists());
            System.out.println();

            // Test Case 5: Archive path does not exist
            System.out.println("--- Test Case 5: Archive Path Not Found ---");
            String nonExistentZip = baseTestDir + File.separator + "notfound.zip";
            String nonExistentExtractDir = baseTestDir + File.separator + "non_existent_extract";
            boolean success5 = extractArchive(nonExistentZip, nonExistentExtractDir);
            System.out.println("Extraction successful: " + success5);
            System.out.println();

        } finally {
             // Cleanup
             System.out.println("Cleaning up test files and directories...");
             deleteDirectory(baseDir);
             // Clean up potential malicious file
             new File("malicious.txt").delete();
             System.out.println("Cleanup complete.");
        }
    }
}