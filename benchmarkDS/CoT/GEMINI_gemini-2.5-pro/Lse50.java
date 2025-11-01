import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Comparator;

// Requires Apache Commons Compress library: org.apache.commons:commons-compress
import org.apache.commons.compress.archivers.tar.TarArchiveEntry;
import org.apache.commons.compress.archivers.tar.TarArchiveInputStream;
import org.apache.commons.compress.archivers.tar.TarArchiveOutputStream;
import org.apache.commons.compress.utils.IOUtils;

public class Lse50 {

    /**
     * Extracts a TAR archive to a specified directory, preventing path traversal attacks.
     *
     * @param tarFilePath The path to the .tar file.
     * @param destDirPath The path to the destination directory.
     * @throws IOException If an I/O error occurs or a path traversal attack is detected.
     */
    public static void extractTar(String tarFilePath, String destDirPath) throws IOException {
        File destDir = new File(destDirPath);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }

        // Use canonical path for security check
        String canonicalDestDirPath = destDir.getCanonicalPath();

        try (FileInputStream fis = new FileInputStream(tarFilePath);
             TarArchiveInputStream tis = new TarArchiveInputStream(fis)) {
            
            TarArchiveEntry entry;
            while ((entry = tis.getNextTarEntry()) != null) {
                File destFile = new File(destDir, entry.getName());
                
                // Security Check: Path Traversal (Zip Slip)
                String canonicalDestFilePath = destFile.getCanonicalPath();
                if (!canonicalDestFilePath.startsWith(canonicalDestDirPath + File.separator)) {
                    throw new IOException("Attempted Path Traversal in TAR file: " + entry.getName());
                }

                if (entry.isDirectory()) {
                    if (!destFile.exists()) {
                        destFile.mkdirs();
                    }
                } else {
                    // Ensure parent directory exists
                    File parent = destFile.getParentFile();
                    if (!parent.exists()) {
                        parent.mkdirs();
                    }
                    try (FileOutputStream fos = new FileOutputStream(destFile)) {
                        IOUtils.copy(tis, fos);
                    }
                }
            }
        }
    }

    /**
     * Creates a TAR archive from a list of files.
     *
     * @param tarFilePath The path of the TAR file to create.
     * @param filesToAdd  The paths of the files to add to the archive.
     * @throws IOException If an I/O error occurs.
     */
    private static void createTarFile(String tarFilePath, String[] filesToAdd, String[] entryNames) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(tarFilePath);
             TarArchiveOutputStream taos = new TarArchiveOutputStream(fos)) {
            taos.setLongFileMode(TarArchiveOutputStream.LONGFILE_GNU);
            for (int i = 0; i < filesToAdd.length; i++) {
                File file = new File(filesToAdd[i]);
                TarArchiveEntry entry = new TarArchiveEntry(file, entryNames[i]);
                taos.putArchiveEntry(entry);
                try (FileInputStream fis = new FileInputStream(file)) {
                    IOUtils.copy(fis, taos);
                }
                taos.closeArchiveEntry();
            }
        }
    }
    
    // Helper to delete a directory recursively
    private static void deleteDirectory(File directory) throws IOException {
        if (directory.exists()) {
            Files.walk(directory.toPath())
                 .sorted(Comparator.reverseOrder())
                 .map(Path::toFile)
                 .forEach(File::delete);
        }
    }


    public static void main(String[] args) {
        // --- Test Case Setup ---
        try {
            // Create dummy files for archiving
            Files.createDirectories(Paths.get("test_data"));
            Files.write(Paths.get("test_data/file1.txt"), "hello".getBytes());
            Files.createDirectories(Paths.get("test_data/subdir"));
            Files.write(Paths.get("test_data/subdir/file2.txt"), "world".getBytes());
        } catch (IOException e) {
            System.err.println("Failed to create test setup files: " + e.getMessage());
            return;
        }

        // --- Test Cases ---
        String[] testCases = {
            "test_case_1_normal",
            "test_case_2_empty_tar",
            "test_case_3_nested_dirs",
            "test_case_4_malicious_path",
            "test_case_5_dot_path"
        };

        for (int i = 0; i < testCases.length; i++) {
            String testName = testCases[i];
            String tarFile = testName + ".tar";
            String unpackDir = "/tmp/unpack_java_" + testName;
            System.out.println("--- Running " + testName + " ---");
            
            try {
                // 1. Create the TAR file for the test case
                switch (i) {
                    case 0: // Normal
                        createTarFile(tarFile, 
                            new String[]{"test_data/file1.txt", "test_data/subdir/file2.txt"},
                            new String[]{"file1.txt", "subdir/file2.txt"});
                        break;
                    case 1: // Empty
                        createTarFile(tarFile, new String[]{}, new String[]{});
                        break;
                    case 2: // Nested Dirs
                        createTarFile(tarFile, 
                            new String[]{"test_data/file1.txt"},
                            new String[]{"a/b/c/file1.txt"});
                        break;
                    case 3: // Malicious Path (Path Traversal)
                        createTarFile(tarFile, 
                            new String[]{"test_data/file1.txt"},
                            new String[]{"../../evil.txt"});
                        break;
                    case 4: // Dot Path
                        createTarFile(tarFile, 
                            new String[]{"test_data/file1.txt"},
                            new String[]{"./safe_file.txt"});
                        break;
                }

                // 2. Run the extraction
                System.out.println("Extracting " + tarFile + " to " + unpackDir);
                extractTar(tarFile, unpackDir);
                System.out.println("Extraction successful.");

            } catch (IOException e) {
                System.err.println("Caught expected exception for malicious input or I/O error: " + e.getMessage());
            } finally {
                // 3. Cleanup
                try {
                    Files.deleteIfExists(Paths.get(tarFile));
                    deleteDirectory(new File(unpackDir));
                    System.out.println("Cleaned up " + tarFile + " and " + unpackDir);
                } catch (IOException e) {
                    System.err.println("Cleanup failed: " + e.getMessage());
                }
                System.out.println();
            }
        }

        // --- Global Test Cleanup ---
        try {
            deleteDirectory(new File("test_data"));
        } catch (IOException e) {
            System.err.println("Global cleanup of test_data failed: " + e.getMessage());
        }
    }
}