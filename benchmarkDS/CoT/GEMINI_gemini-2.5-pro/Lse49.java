import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

/**
 * The prompt requests to "Open the archive.zip file as a tarfile".
 * This is technically incorrect as .zip and .tar are different archive formats.
 * This code correctly processes a .zip file using Java's standard zip libraries.
 */
public class Lse49 {

    /**
     * Unzips a .zip file to a specified destination directory securely.
     *
     * @param zipFilePath   Path to the .zip file.
     * @param destDirectory Path to the directory where files will be extracted.
     * @throws IOException if an I/O error occurs or a path traversal attempt is detected.
     */
    public static void unzip(String zipFilePath, String destDirectory) throws IOException {
        Path destDirPath = Paths.get(destDirectory);
        // Ensure destination directory exists
        Files.createDirectories(destDirPath);

        // Security: Get the canonical path of the destination directory to prevent path traversal
        Path normalizedDestDir = destDirPath.toAbsolutePath().normalize();

        try (ZipInputStream zis = new ZipInputStream(new FileInputStream(zipFilePath))) {
            ZipEntry zipEntry = zis.getNextEntry();
            while (zipEntry != null) {
                Path newFilePath = normalizedDestDir.resolve(zipEntry.getName());

                // SECURITY CHECK: Prevent Path Traversal (Zip Slip)
                // Ensure the resolved path of the entry is within the destination directory
                Path normalizedNewFilePath = newFilePath.toAbsolutePath().normalize();
                if (!normalizedNewFilePath.startsWith(normalizedDestDir)) {
                    throw new IOException("Bad zip entry: " + zipEntry.getName() + " (path traversal attempt)");
                }

                if (zipEntry.isDirectory()) {
                    Files.createDirectories(newFilePath);
                } else {
                    // Create parent directories if they don't exist
                    Path parent = newFilePath.getParent();
                    if (parent != null) {
                        Files.createDirectories(parent);
                    }
                    // Write file content
                    try (OutputStream fos = Files.newOutputStream(newFilePath)) {
                        byte[] buffer = new byte[4096];
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

    // Helper to create a zip file for testing purposes
    private static void createTestZip(String zipFilePath, String... fileEntries) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipFilePath))) {
            for (String entry : fileEntries) {
                if (entry.endsWith("/")) { // Directory entry
                    zos.putNextEntry(new ZipEntry(entry));
                    zos.closeEntry();
                } else { // File entry
                    ZipEntry zipEntry = new ZipEntry(entry);
                    zos.putNextEntry(zipEntry);
                    byte[] data = ("Content of " + entry).getBytes();
                    zos.write(data, 0, data.length);
                    zos.closeEntry();
                }
            }
        }
    }

    // Helper to recursively delete a directory for cleanup
    private static void deleteDirectory(File directory) {
        File[] allContents = directory.listFiles();
        if (allContents != null) {
            for (File file : allContents) {
                deleteDirectory(file);
            }
        }
        directory.delete();
    }

    public static void main(String[] args) {
        System.out.println("Running Java Unzip Test Cases...");

        String tempDir = System.getProperty("java.io.tmpdir");
        Path baseTestPath = Paths.get(tempDir, "java_unzip_tests_lse49");

        try {
            // Clean up previous runs if they exist
            if (Files.exists(baseTestPath)) {
                deleteDirectory(baseTestPath.toFile());
            }
            Files.createDirectories(baseTestPath);
            System.out.println("Using temporary directory: " + baseTestPath);


            // Test Case 1: Simple zip with one file
            System.out.println("\n--- Test Case 1: Simple zip ---");
            String zip1 = baseTestPath.resolve("archive1.zip").toString();
            String unpack1 = baseTestPath.resolve("unpack1").toString();
            createTestZip(zip1, "file1.txt");
            unzip(zip1, unpack1);
            if (Files.exists(Paths.get(unpack1, "file1.txt"))) {
                System.out.println("SUCCESS: file1.txt found.");
            } else {
                System.out.println("FAILURE: file1.txt not found.");
            }

            // Test Case 2: Zip with a subdirectory
            System.out.println("\n--- Test Case 2: Zip with subdirectory ---");
            String zip2 = baseTestPath.resolve("archive2.zip").toString();
            String unpack2 = baseTestPath.resolve("unpack2").toString();
            createTestZip(zip2, "dir1/", "dir1/file2.txt");
            unzip(zip2, unpack2);
            if (Files.exists(Paths.get(unpack2, "dir1", "file2.txt"))) {
                System.out.println("SUCCESS: dir1/file2.txt found.");
            } else {
                System.out.println("FAILURE: dir1/file2.txt not found.");
            }

            // Test Case 3: Zip with an empty directory
            System.out.println("\n--- Test Case 3: Zip with empty directory ---");
            String zip3 = baseTestPath.resolve("archive3.zip").toString();
            String unpack3 = baseTestPath.resolve("unpack3").toString();
            createTestZip(zip3, "emptydir/");
            unzip(zip3, unpack3);
            if (Files.isDirectory(Paths.get(unpack3, "emptydir"))) {
                System.out.println("SUCCESS: emptydir/ found.");
            } else {
                System.out.println("FAILURE: emptydir/ not found.");
            }

            // Test Case 4: Unzipping to a non-existent directory
            System.out.println("\n--- Test Case 4: Unzip to non-existent directory ---");
            String zip4 = baseTestPath.resolve("archive4.zip").toString();
            String unpack4 = baseTestPath.resolve("unpack4").toString();
            createTestZip(zip4, "test.txt");
            unzip(zip4, unpack4); // unpack4 does not exist yet
            if (Files.exists(Paths.get(unpack4, "test.txt"))) {
                System.out.println("SUCCESS: test.txt found in newly created directory.");
            } else {
                System.out.println("FAILURE: test.txt not found.");
            }

            // Test Case 5: Unzipping an empty zip file
            System.out.println("\n--- Test Case 5: Unzip empty zip file ---");
            String zip5 = baseTestPath.resolve("archive5.zip").toString();
            String unpack5 = baseTestPath.resolve("unpack5").toString();
            createTestZip(zip5); // No entries
            unzip(zip5, unpack5);
            if (Files.isDirectory(Paths.get(unpack5)) && new File(unpack5).list().length == 0) {
                System.out.println("SUCCESS: Unpack directory is empty as expected.");
            } else {
                System.out.println("FAILURE: Unpack directory is not empty.");
            }

            System.out.println("\nAll Java tests completed.");

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // Final cleanup
            if (Files.exists(baseTestPath)) {
                deleteDirectory(baseTestPath.toFile());
            }
        }
    }
}