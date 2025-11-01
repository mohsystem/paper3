import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.zip.*;

public class Lse49 {

    // Securely extract a ZIP archive to destDir. Returns list of extracted file paths.
    public static List<String> extractArchive(String archivePath, String destDir) throws IOException {
        Path archive = Paths.get(archivePath);
        Path dest = Paths.get(destDir);
        Files.createDirectories(dest);
        String destCanonical = dest.toFile().getCanonicalPath();

        List<String> extracted = new ArrayList<>();
        long maxEntrySize = 1024L * 1024L * 1024L; // 1 GiB per entry limit
        long maxTotalSize = 2L * 1024L * 1024L * 1024L; // 2 GiB total limit
        long totalWritten = 0L;

        try (ZipFile zip = new ZipFile(archive.toFile())) {
            Enumeration<? extends ZipEntry> entries = zip.entries();
            while (entries.hasMoreElements()) {
                ZipEntry e = entries.nextElement();

                String rawName = e.getName();
                if (rawName == null || rawName.trim().isEmpty()) continue;

                // Normalize separators to prevent Windows backslash confusion
                String normalizedName = rawName.replace("\\", "/");

                // Disallow absolute paths and drive letters
                if (normalizedName.startsWith("/") || normalizedName.startsWith("\\") ||
                    normalizedName.matches("^[A-Za-z]:/.*")) {
                    continue;
                }

                // Resolve target path and ensure it's within destination (prevent zip-slip)
                Path target = dest.resolve(normalizedName).normalize();
                String targetCanonical = target.toFile().getCanonicalPath();
                if (!(targetCanonical.equals(destCanonical) ||
                      targetCanonical.startsWith(destCanonical + File.separator))) {
                    continue;
                }

                if (e.isDirectory() || normalizedName.endsWith("/")) {
                    Files.createDirectories(target);
                    continue;
                }

                // Ensure parent directories
                Path parent = target.getParent();
                if (parent != null) Files.createDirectories(parent);

                long declaredSize = e.getSize();
                if (declaredSize > maxEntrySize) {
                    throw new IOException("Entry too large: " + normalizedName);
                }

                try (InputStream is = new BufferedInputStream(zip.getInputStream(e));
                     OutputStream os = new BufferedOutputStream(Files.newOutputStream(
                         target,
                         StandardOpenOption.CREATE,
                         StandardOpenOption.TRUNCATE_EXISTING,
                         StandardOpenOption.WRITE))) {

                    byte[] buffer = new byte[8192];
                    long written = 0L;
                    int read;
                    while ((read = is.read(buffer)) != -1) {
                        written += read;
                        totalWritten += read;
                        if (written > maxEntrySize) {
                            throw new IOException("Entry exceeded size limit: " + normalizedName);
                        }
                        if (totalWritten > maxTotalSize) {
                            throw new IOException("Archive exceeded total extraction size limit");
                        }
                        os.write(buffer, 0, read);
                    }
                }

                // Set restrictive permissions if supported (owner read/write)
                try {
                    if (Files.getFileStore(dest).supportsFileAttributeView("posix")) {
                        Set<PosixFilePermission> perms = EnumSet.of(
                            PosixFilePermission.OWNER_READ,
                            PosixFilePermission.OWNER_WRITE
                        );
                        Files.setPosixFilePermissions(target, perms);
                    }
                } catch (Exception ignore) {
                    // Ignore if filesystem doesn't support posix or setting perms fails
                }

                extracted.add(target.toString());
            }
        }

        return extracted;
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String defaultArchive = "archive.zip";
        String[] dests = new String[] {
            "/tmp/unpack",
            "/tmp/unpack_case2",
            "/tmp/unpack_case3",
            "/tmp/unpack_case4",
            "/tmp/unpack_case5"
        };

        for (int i = 0; i < dests.length; i++) {
            try {
                List<String> files = extractArchive(defaultArchive, dests[i]);
                System.out.println("Test " + (i + 1) + ": Extracted " + files.size() + " entries to " + dests[i]);
            } catch (Exception ex) {
                System.out.println("Test " + (i + 1) + ": Failed to extract to " + dests[i] + " - " + ex.getMessage());
            }
        }
    }
}