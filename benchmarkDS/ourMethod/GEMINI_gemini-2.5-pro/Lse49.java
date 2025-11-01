import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Lse49 {
    private static final Logger LOGGER = Logger.getLogger(Lse49.class.getName());
    private static final int BUFFER_SIZE = 4096;

    /**
     * Unzips a zip file to a specified directory, preventing path traversal attacks.
     *
     * @param zipFilePath   The path to the zip file.
     * @param destDirectory The directory to extract files to.
     * @throws IOException if an I/O error occurs.
     */
    public static void unzipArchive(String zipFilePath, String destDirectory) throws IOException {
        Path destDirPath = Paths.get(destDirectory).toAbsolutePath().normalize();
        if (!Files.exists(destDirPath)) {
            Files.createDirectories(destDirPath);
        }

        File zipFile = new File(zipFilePath);
        if (!zipFile.exists() || !zipFile.isFile() || !zipFile.canRead()){
            throw new IOException("Archive file is not accessible: " + zipFilePath);
        }

        try (InputStream fis = new FileInputStream(zipFile);
             ZipInputStream zis = new ZipInputStream(fis)) {

            ZipEntry zipEntry = zis.getNextEntry();
            while (zipEntry != null) {
                Path destPath = destDirPath.resolve(zipEntry.getName()).toAbsolutePath().normalize();

                // Path traversal check
                if (!destPath.startsWith(destDirPath)) {
                    throw new IOException("Entry is outside of the target directory: " + zipEntry.getName());
                }

                if (zipEntry.isDirectory()) {
                    Files.createDirectories(destPath);
                } else {
                    // Create parent directories if they don't exist
                    if (destPath.getParent() != null) {
                        Files.createDirectories(destPath.getParent());
                    }
                    // Using Files.copy is safer as it handles resource management
                    Files.copy(zis, destPath);
                }
                zis.closeEntry();
                zipEntry = zis.getNextEntry();
            }
        }
    }

    private static void createTestZip(String zipFilePath, String[] fileNames, String[] fileContents, boolean isMalicious) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(zipFilePath);
             ZipOutputStream zos = new ZipOutputStream(fos)) {

            for (int i = 0; i < fileNames.length; i++) {
                String fileName = fileNames[i];
                if (isMalicious && i == fileNames.length -1) {
                    // This creates an entry that tries to traverse up the directory tree
                    fileName = "../" + fileName;
                }
                ZipEntry zipEntry = new ZipEntry(fileName);
                zos.putNextEntry(zipEntry);
                if (fileContents[i] != null) {
                    byte[] bytes = fileContents[i].getBytes();
                    zos.write(bytes, 0, bytes.length);
                }
                zos.closeEntry();
            }
        }
    }
    
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
        String baseTempDir = System.getProperty("java.io.tmpdir");
        
        // Test Case 1: Unzip a valid archive.
        System.out.println("--- Test Case 1: Valid Archive ---");
        try {
            String archivePath = baseTempDir + File.separator + "archive1.zip";
            String unpackDir = baseTempDir + File.separator + "unpack1";
            createTestZip(archivePath, new String[]{"file1.txt", "dir1/", "dir1/file2.txt"}, new String[]{"content1", null, "content2"}, false);
            System.out.println("Unzipping " + archivePath + " to " + unpackDir);
            unzipArchive(archivePath, unpackDir);
            System.out.println("Test Case 1: Success.");
            deleteDirectory(new File(unpackDir));
            new File(archivePath).delete();
        } catch (IOException e) {
            LOGGER.log(Level.SEVERE, "Test Case 1: Failed.", e);
        }

        // Test Case 2: Attempt to unzip an archive with a path traversal entry.
        System.out.println("\n--- Test Case 2: Malicious Archive (Path Traversal) ---");
        try {
            String archivePath = baseTempDir + File.separator + "malicious.zip";
            String unpackDir = baseTempDir + File.separator + "unpack2";
            createTestZip(archivePath, new String[]{"good.txt", "evil.txt"}, new String[]{"good content", "evil content"}, true);
            System.out.println("Unzipping " + archivePath + " to " + unpackDir);
            unzipArchive(archivePath, unpackDir);
            System.out.println("Test Case 2: Failed (should have thrown exception).");
        } catch (IOException e) {
            System.out.println("Test Case 2: Success (Caught expected exception): " + e.getMessage());
        } finally {
            deleteDirectory(new File(baseTempDir + File.separator + "unpack2"));
            new File(baseTempDir + File.separator + "malicious.zip").delete();
        }

        // Test Case 3: Attempt to unzip a non-existent archive.
        System.out.println("\n--- Test Case 3: Non-existent Archive ---");
        try {
            String archivePath = baseTempDir + File.separator + "nonexistent.zip";
            String unpackDir = baseTempDir + File.separator + "unpack3";
            System.out.println("Unzipping " + archivePath + " to " + unpackDir);
            unzipArchive(archivePath, unpackDir);
            System.out.println("Test Case 3: Failed (should have thrown exception).");
        } catch (IOException e) {
            System.out.println("Test Case 3: Success (Caught expected exception): " + e.getMessage());
        }

        // Test Case 4: Unzip to a location with no permissions.
        System.out.println("\n--- Test Case 4: No Permission Directory ---");
        // This test may not fail on all systems (e.g., if run as root)
        String unpackDir = "/root/unpack4";
        if (System.getProperty("os.name").toLowerCase().contains("win")) {
            unpackDir = "C:\\Windows\\System32\\config\\unpack4";
        }
        try {
            String archivePath = baseTempDir + File.separator + "archive4.zip";
            createTestZip(archivePath, new String[]{"file1.txt"}, new String[]{"content"}, false);
            System.out.println("Unzipping " + archivePath + " to " + unpackDir);
            unzipArchive(archivePath, unpackDir);
            System.out.println("Test Case 4: Failed or was skipped (should have thrown exception).");
            new File(archivePath).delete();
        } catch (IOException e) {
            System.out.println("Test Case 4: Success (Caught expected exception): " + e.getMessage());
        } finally {
            new File(baseTempDir + File.separator + "archive4.zip").delete();
        }

        // Test Case 5: Unzip an empty archive.
        System.out.println("\n--- Test Case 5: Empty Archive ---");
        try {
            String archivePath = baseTempDir + File.separator + "empty.zip";
            String unpackDir5 = baseTempDir + File.separator + "unpack5";
            createTestZip(archivePath, new String[]{}, new String[]{}, false);
            System.out.println("Unzipping " + archivePath + " to " + unpackDir5);
            unzipArchive(archivePath, unpackDir5);
            System.out.println("Test Case 5: Success.");
            deleteDirectory(new File(unpackDir5));
            new File(archivePath).delete();
        } catch (IOException e) {
            LOGGER.log(Level.SEVERE, "Test Case 5: Failed.", e);
        }
    }
}