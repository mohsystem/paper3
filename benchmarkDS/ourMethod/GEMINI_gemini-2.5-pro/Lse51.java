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

// Note: This code requires the Apache Commons Compress library.
// Add the following dependency to your project (e.g., pom.xml for Maven):
// <dependency>
//     <groupId>org.apache.commons</groupId>
//     <artifactId>commons-compress</artifactId>
//     <version>1.26.2</version>
// </dependency>
import org.apache.commons.compress.archivers.tar.TarArchiveEntry;
import org.apache.commons.compress.archivers.tar.TarArchiveInputStream;
import org.apache.commons.compress.archivers.tar.TarArchiveOutputStream;
import org.apache.commons.compress.compressors.bzip2.BZip2CompressorInputStream;
import org.apache.commons.compress.compressors.bzip2.BZip2CompressorOutputStream;
import org.apache.commons.compress.compressors.gzip.GzipCompressorInputStream;
import org.apache.commons.compress.compressors.gzip.GzipCompressorOutputStream;

public class Lse51 {

    /**
     * Securely extracts a tar archive (including .gz and .bz2) to a destination directory.
     * It prevents path traversal attacks by ensuring all files are extracted inside the destination.
     *
     * @param tarFile The path to the tar file.
     * @param destDir The path to the destination directory.
     * @return true if extraction was successful, false otherwise.
     */
    public static boolean secureExtractTar(Path tarFile, Path destDir) {
        if (!Files.isRegularFile(tarFile)) {
            System.err.println("Error: Tar file is not a regular file: " + tarFile);
            return false;
        }

        try {
            // Ensure destination directory exists and is a directory
            if (!Files.exists(destDir)) {
                Files.createDirectories(destDir);
            } else if (!Files.isDirectory(destDir)) {
                 System.err.println("Error: Destination path exists but is not a directory: " + destDir);
                 return false;
            }

            final Path canonicalDestDir = destDir.toFile().getCanonicalFile().toPath();
            String tarFileName = tarFile.getFileName().toString().toLowerCase();

            try (InputStream fi = Files.newInputStream(tarFile);
                 InputStream bi = getDecompressorStream(tarFileName, fi);
                 TarArchiveInputStream ti = new TarArchiveInputStream(bi)) {

                TarArchiveEntry entry;
                while ((entry = ti.getNextTarEntry()) != null) {
                    Path destPath = canonicalDestDir.resolve(entry.getName());
                    
                    // The .toFile().getCanonicalPath() resolves '..' and symlinks.
                    // This check is the core of the path traversal defense.
                    if (!destPath.toFile().getCanonicalPath().startsWith(canonicalDestDir.toString() + File.separator) &&
                        !destPath.toFile().getCanonicalPath().equals(canonicalDestDir.toString())) {
                        System.err.println("Error: Attempted path traversal attack in tar file: " + entry.getName());
                        return false;
                    }

                    if (entry.isDirectory()) {
                        if (!Files.exists(destPath)) {
                            Files.createDirectories(destPath);
                        }
                    } else if (entry.isFile()) {
                        // Ensure parent directories exist
                        Path parent = destPath.getParent();
                        if (parent != null && !Files.exists(parent)) {
                            Files.createDirectories(parent);
                        }
                        
                        try (OutputStream os = new FileOutputStream(destPath.toFile())) {
                            byte[] buffer = new byte[8192];
                            int len;
                            while ((len = ti.read(buffer)) > 0) {
                                os.write(buffer, 0, len);
                            }
                        }
                    } else {
                        // Ignore symlinks, block devices, etc. for security
                        System.out.println("Skipping non-regular file/directory: " + entry.getName());
                    }
                }
            }
            return true;
        } catch (IOException e) {
            System.err.println("An error occurred during tar extraction: " + e.getMessage());
            return false;
        }
    }

    private static InputStream getDecompressorStream(String filename, InputStream in) throws IOException {
        if (filename.endsWith(".tar.gz") || filename.endsWith(".tgz")) {
            return new GzipCompressorInputStream(in);
        } else if (filename.endsWith(".tar.bz2") || filename.endsWith(".tbz2")) {
            return new BZip2CompressorInputStream(in);
        }
        return in; // Assumes uncompressed .tar
    }
    
    // --- Test Cases ---
    private static Path testArchivesDir;

    private static void createTestArchives() throws IOException {
        testArchivesDir = Files.createTempDirectory("java_tar_tests_");

        // Test 1: simple .tar.gz
        try (FileOutputStream fos = new FileOutputStream(testArchivesDir.resolve("test1.tar.gz").toFile());
             GzipCompressorOutputStream gzos = new GzipCompressorOutputStream(fos);
             TarArchiveOutputStream taos = new TarArchiveOutputStream(gzos)) {
            addEntry(taos, "file1.txt", "content1");
            addEntry(taos, "dir1/file2.txt", "content2");
        }

        // Test 2: simple .tar.bz2
        try (FileOutputStream fos = new FileOutputStream(testArchivesDir.resolve("test2.tar.bz2").toFile());
             BZip2CompressorOutputStream bzos = new BZip2CompressorOutputStream(fos);
             TarArchiveOutputStream taos = new TarArchiveOutputStream(bzos)) {
            addEntry(taos, "file3.txt", "content3");
        }

        // Test 3: path traversal
        try (FileOutputStream fos = new FileOutputStream(testArchivesDir.resolve("test3_traversal.tar.gz").toFile());
             GzipCompressorOutputStream gzos = new GzipCompressorOutputStream(fos);
             TarArchiveOutputStream taos = new TarArchiveOutputStream(gzos)) {
            addEntry(taos, "../../../pwned.txt", "malicious");
        }

        // Test 4: absolute path
        try (FileOutputStream fos = new FileOutputStream(testArchivesDir.resolve("test4_absolute.tar.gz").toFile());
             GzipCompressorOutputStream gzos = new GzipCompressorOutputStream(fos);
             TarArchiveOutputStream taos = new TarArchiveOutputStream(gzos)) {
            addEntry(taos, "/tmp/absolute_pwned.txt", "malicious");
        }
        
        // Test 5: symlink
        try (FileOutputStream fos = new FileOutputStream(testArchivesDir.resolve("test5_symlink.tar.gz").toFile());
             GzipCompressorOutputStream gzos = new GzipCompressorOutputStream(fos);
             TarArchiveOutputStream taos = new TarArchiveOutputStream(gzos)) {
            TarArchiveEntry entry = new TarArchiveEntry("link_to_file1", TarArchiveEntry.LF_SYMLINK);
            entry.setLinkName("file1.txt");
            taos.putArchiveEntry(entry);
            taos.closeArchiveEntry();
        }
    }
    
    private static void addEntry(TarArchiveOutputStream taos, String name, String content) throws IOException {
        byte[] data = content.getBytes("UTF-8");
        TarArchiveEntry entry = new TarArchiveEntry(name);
        entry.setSize(data.length);
        taos.putArchiveEntry(entry);
        taos.write(data);
        taos.closeArchiveEntry();
    }
    
    private static void cleanup(Path dir) throws IOException {
        if (dir != null && Files.exists(dir)) {
            Files.walk(dir)
                .sorted(Comparator.reverseOrder())
                .map(Path::toFile)
                .forEach(File::delete);
        }
    }

    public static void main(String[] args) throws IOException {
        System.out.println("Setting up test archives...");
        createTestArchives();
        System.out.println("Test archives created in: " + testArchivesDir);

        // Test Case 1: Simple gzip tar
        Path tempDir1 = Files.createTempDirectory("unpack1_");
        System.out.println("\n--- Test Case 1: Simple .tar.gz ---");
        boolean success1 = secureExtractTar(testArchivesDir.resolve("test1.tar.gz"), tempDir1);
        System.out.println("Extraction success: " + success1);
        System.out.println("Check file1.txt exists: " + Files.exists(tempDir1.resolve("file1.txt")));
        System.out.println("Check dir1/file2.txt exists: " + Files.exists(tempDir1.resolve("dir1/file2.txt")));
        
        // Test Case 2: Simple bzip2 tar
        Path tempDir2 = Files.createTempDirectory("unpack2_");
        System.out.println("\n--- Test Case 2: Simple .tar.bz2 ---");
        boolean success2 = secureExtractTar(testArchivesDir.resolve("test2.tar.bz2"), tempDir2);
        System.out.println("Extraction success: " + success2);
        System.out.println("Check file3.txt exists: " + Files.exists(tempDir2.resolve("file3.txt")));

        // Test Case 3: Path Traversal
        Path tempDir3 = Files.createTempDirectory("unpack3_");
        System.out.println("\n--- Test Case 3: Path Traversal (should fail) ---");
        boolean success3 = secureExtractTar(testArchivesDir.resolve("test3_traversal.tar.gz"), tempDir3);
        System.out.println("Extraction success: " + success3);
        Path pwnedFile = tempDir3.resolve("../pwned.txt").toFile().getCanonicalFile().toPath();
        System.out.println("Check for traversal file " + pwnedFile + " exists: " + Files.exists(pwnedFile));

        // Test Case 4: Absolute Path
        Path tempDir4 = Files.createTempDirectory("unpack4_");
        System.out.println("\n--- Test Case 4: Absolute Path (should fail) ---");
        boolean success4 = secureExtractTar(testArchivesDir.resolve("test4_absolute.tar.gz"), tempDir4);
        System.out.println("Extraction success: " + success4);
        System.out.println("Check for absolute file /tmp/absolute_pwned.txt exists: " + Files.exists(Paths.get("/tmp/absolute_pwned.txt")));

        // Test Case 5: Symlink
        Path tempDir5 = Files.createTempDirectory("unpack5_");
        System.out.println("\n--- Test Case 5: Symlink (should be skipped) ---");
        boolean success5 = secureExtractTar(testArchivesDir.resolve("test5_symlink.tar.gz"), tempDir5);
        System.out.println("Extraction success: " + success5);
        System.out.println("Check symlink exists: " + Files.isSymbolicLink(tempDir5.resolve("link_to_file1")));

        // Cleanup
        cleanup(tempDir1);
        cleanup(tempDir2);
        cleanup(tempDir3);
        cleanup(tempDir4);
        cleanup(tempDir5);
        if (Files.exists(Paths.get("/tmp/absolute_pwned.txt"))) {
            Files.delete(Paths.get("/tmp/absolute_pwned.txt"));
        }
        cleanup(testArchivesDir);
        System.out.println("\nCleanup complete.");
    }
}