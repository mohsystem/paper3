import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

public class Lse50 {

    /**
     * Extracts a zip file to a specified directory.
     * The destination directory will be created if it does not exist.
     *
     * @param zipFilePath   Path to the .zip file.
     * @param destDirectory Path to the destination directory.
     * @throws IOException if an I/O error occurs.
     */
    public static void extractZip(String zipFilePath, String destDirectory) throws IOException {
        Path destDirPath = Paths.get(destDirectory);
        if (!Files.exists(destDirPath)) {
            Files.createDirectories(destDirPath);
        }

        try (ZipInputStream zipIn = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry entry = zipIn.getNextEntry();
            while (entry != null) {
                Path filePath = destDirPath.resolve(entry.getName()).normalize();
                
                // Prevent Zip Slip vulnerability
                if (!filePath.startsWith(destDirPath)) {
                    throw new IOException("Zip entry is outside of the target dir: " + entry.getName());
                }

                if (!entry.isDirectory()) {
                    // Create parent directories if they don't exist for files inside subfolders
                    Path parent = filePath.getParent();
                    if (parent != null && !Files.exists(parent)) {
                        Files.createDirectories(parent);
                    }
                    // Extract file
                    Files.copy(zipIn, filePath, StandardCopyOption.REPLACE_EXISTING);
                } else {
                    // Create directory
                    Files.createDirectories(filePath);
                }
                zipIn.closeEntry();
                entry = zipIn.getNextEntry();
            }
        }
    }

    // Helper to create a test zip file
    private static void createTestZip(String zipFileName, String[] fileNames, String[] fileContents) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipFileName))) {
            for (int i = 0; i < fileNames.length; i++) {
                ZipEntry entry = new ZipEntry(fileNames[i]);
                zos.putNextEntry(entry);
                if (fileContents[i] != null) {
                    zos.write(fileContents[i].getBytes(StandardCharsets.UTF_8));
                }
                zos.closeEntry();
            }
        }
    }

    // Helper to recursively delete a directory
    private static void deleteDirectory(Path path) throws IOException {
        if (Files.exists(path)) {
            Files.walkFileTree(path, new SimpleFileVisitor<Path>() {
                @Override
                public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
                    Files.delete(file);
                    return FileVisitResult.CONTINUE;
                }
                @Override
                public FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException {
                    Files.delete(dir);
                    return FileVisitResult.CONTINUE;
                }
            });
        }
    }

    public static void main(String[] args) {
        System.out.println("Running Java Lse50 tests...");
        String destDir = "/tmp/unpack";

        // Test Case 1 & 2: Basic extraction and creating destination directory
        System.out.println("\n--- Test Case 1 & 2: Basic Extraction ---");
        try {
            String zipFile = "archive1.zip";
            Path destPath = Paths.get(destDir);
            deleteDirectory(destPath); // Ensure dir doesn't exist
            createTestZip(zipFile, new String[]{"file1.txt", "dir1/", "dir1/file2.txt"}, new String[]{"hello", null, "world"});
            
            extractZip(zipFile, destDir);

            Path file1 = destPath.resolve("file1.txt");
            Path file2 = destPath.resolve("dir1/file2.txt");
            if (Files.exists(file1) && Files.readString(file1).equals("hello") && Files.exists(file2) && Files.readString(file2).equals("world")) {
                System.out.println("PASS: Files extracted successfully.");
            } else {
                System.out.println("FAIL: File extraction verification failed.");
            }
            // Cleanup
            Files.delete(Paths.get(zipFile));
            deleteDirectory(destPath);
        } catch (IOException e) {
            System.out.println("FAIL: " + e.getMessage());
        }

        // Test Case 3: Non-existent zip file
        System.out.println("\n--- Test Case 3: Non-existent ZIP file ---");
        try {
            extractZip("nonexistent.zip", destDir);
            System.out.println("FAIL: Should have thrown FileNotFoundException.");
        } catch (FileNotFoundException e) {
            System.out.println("PASS: Correctly handled non-existent zip file.");
        } catch (IOException e) {
            System.out.println("FAIL: Unexpected exception: " + e.getMessage());
        }

        // Test Case 4: Extracting an empty zip file
        System.out.println("\n--- Test Case 4: Empty ZIP file ---");
        try {
            String zipFile = "archive_empty.zip";
            Path destPath = Paths.get(destDir);
            deleteDirectory(destPath);
            createTestZip(zipFile, new String[]{}, new String[]{});
            
            extractZip(zipFile, destDir);
            
            if (Files.exists(destPath) && Files.list(destPath).count() == 0) {
                System.out.println("PASS: Empty zip extracted correctly (empty directory created).");
            } else {
                System.out.println("FAIL: Directory should be empty after extraction.");
            }
            // Cleanup
            Files.delete(Paths.get(zipFile));
            deleteDirectory(destPath);
        } catch (IOException e) {
            System.out.println("FAIL: " + e.getMessage());
        }

        // Test Case 5: Overwriting existing files
        System.out.println("\n--- Test Case 5: Overwriting existing files ---");
        try {
            String zipFile = "archive_overwrite.zip";
            Path destPath = Paths.get(destDir);
            deleteDirectory(destPath);
            Files.createDirectories(destPath);
            Path existingFile = destPath.resolve("file.txt");
            Files.writeString(existingFile, "old content");

            createTestZip(zipFile, new String[]{"file.txt"}, new String[]{"new content"});
            extractZip(zipFile, destDir);
            
            if (Files.readString(existingFile).equals("new content")) {
                System.out.println("PASS: File was successfully overwritten.");
            } else {
                System.out.println("FAIL: File content was not overwritten.");
            }
            // Cleanup
            Files.delete(Paths.get(zipFile));
            deleteDirectory(destPath);
        } catch (IOException e) {
            System.out.println("FAIL: " + e.getMessage());
        }
    }
}