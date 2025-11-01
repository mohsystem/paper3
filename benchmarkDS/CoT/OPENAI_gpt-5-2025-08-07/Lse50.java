import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class Lse50 {
    // Chain-of-Through condensed steps in secure code (high-level):
    // 1) Understand task: extract archive.zip into /tmp/unpack.
    // 2) Security: prevent path traversal, avoid writing outside target, limit file sizes.
    // 3) Implement safe extraction logic with validation.
    // 4) Review: ensure checks before writes; use try-with-resources.
    // 5) Output: final secured implementation.

    // Securely extract a ZIP archive to a destination directory.
    // Returns the number of files (not directories) extracted.
    public static int extractArchiveZip(String archivePath, String destDir) throws IOException {
        final long MAX_FILE_SIZE = 1024L * 1024L * 1024L; // 1 GiB per file limit to mitigate zip-bombs
        Path archive = Paths.get(archivePath);
        Path dest = Paths.get(destDir);

        if (!Files.exists(archive) || !Files.isRegularFile(archive)) {
            throw new IOException("Archive not found or not a regular file: " + archivePath);
        }

        Files.createDirectories(dest);
        Path destCanonical = dest.toAbsolutePath().normalize();

        int filesExtracted = 0;
        try (InputStream fis = new BufferedInputStream(Files.newInputStream(archive));
             ZipInputStream zis = new ZipInputStream(fis)) {

            ZipEntry entry;
            byte[] buffer = new byte[8192];

            while ((entry = zis.getNextEntry()) != null) {
                String name = entry.getName();

                // Basic name validation
                if (name == null || name.isEmpty()) {
                    zis.closeEntry();
                    continue;
                }
                if (name.contains("..") || name.startsWith("/") || name.startsWith("\\") || name.contains(":")) {
                    // Skip suspicious paths
                    zis.closeEntry();
                    continue;
                }

                Path target;
                try {
                    target = destCanonical.resolve(name).normalize();
                } catch (InvalidPathException ipe) {
                    zis.closeEntry();
                    continue;
                }

                // Ensure target is within destination directory
                if (!target.startsWith(destCanonical)) {
                    zis.closeEntry();
                    continue;
                }

                if (entry.isDirectory()) {
                    Files.createDirectories(target);
                } else {
                    // Avoid creating outside directories
                    Path parent = target.getParent();
                    if (parent != null) {
                        Files.createDirectories(parent);
                    }

                    // Write file with size limit
                    long written = 0L;
                    try (OutputStream out = Files.newOutputStream(
                            target,
                            StandardOpenOption.CREATE,
                            StandardOpenOption.TRUNCATE_EXISTING,
                            StandardOpenOption.WRITE)) {

                        int len;
                        while ((len = zis.read(buffer)) > 0) {
                            written += len;
                            if (written > MAX_FILE_SIZE) {
                                throw new IOException("File too large (possible zip-bomb): " + name);
                            }
                            out.write(buffer, 0, len);
                        }
                    }
                    filesExtracted++;
                }
                zis.closeEntry();
            }
        }
        return filesExtracted;
    }

    public static void main(String[] args) {
        // 5 test cases (may fail if files don't exist on the system where this runs)
        String[] archives = {
            "archive.zip",
            "archive.zip",
            "nonexistent.zip",
            "archive.zip",
            "archive.zip"
        };
        String[] dests = {
            "/tmp/unpack",          // expected per prompt
            "/tmp/unpack",          // repeat extraction
            "/tmp/unpack",          // nonexistent archive
            "/tmp/unpack/subdir",   // nested dest
            "/tmp/unpack"           // same dest again
        };

        for (int i = 0; i < archives.length; i++) {
            try {
                int count = extractArchiveZip(archives[i], dests[i]);
                System.out.println("Test " + (i + 1) + ": Extracted files = " + count);
            } catch (Exception e) {
                System.out.println("Test " + (i + 1) + ": Failed with error: " + e.getMessage());
            }
        }
    }
}