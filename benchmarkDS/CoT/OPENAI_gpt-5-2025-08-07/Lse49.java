import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.DirectoryNotEmptyException;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.FileVisitResult;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;

/*
Chain-of-Through process in code generation:
1) Problem understanding:
   - Implement secure extraction of archive.zip into /tmp/unpack.
   - Provide a function that accepts input parameters (archive path, destination directory) and returns a list of extracted file paths.
2) Security requirements:
   - Prevent Zip Slip path traversal vulnerabilities.
   - Do not follow symlinks when writing.
   - Avoid deleting arbitrary paths; strictly manage /tmp/unpack.
   - Use streaming to avoid excessive memory usage.
3) Secure coding generation:
   - Normalize and validate all paths before writing.
   - Create directories safely.
   - Skip entries that attempt to escape destination directory.
4) Code review:
   - Each write operation validates destination is within base.
   - Directory deletion restricted to expected base path.
   - Try-with-resources for stream safety.
5) Secure code output:
   - All identified risks addressed; only safe entries are extracted.
*/

public class Lse49 {

    public static List<Path> unzipSecure(Path zipPath, Path destDir) throws IOException {
        List<Path> extracted = new ArrayList<>();
        if (zipPath == null || destDir == null) {
            throw new IllegalArgumentException("zipPath and destDir must not be null");
        }
        if (!Files.exists(zipPath) || !Files.isRegularFile(zipPath, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Archive does not exist or is not a regular file: " + zipPath);
        }
        Path base = destDir.toAbsolutePath().normalize();
        Files.createDirectories(base);

        try (ZipFile zf = new ZipFile(zipPath.toFile())) {
            zf.stream().forEach(entry -> {
                try {
                    handleEntrySecure(zf, entry, base, extracted);
                } catch (IOException e) {
                    // Skip problematic entry but continue with others
                }
            });
        }
        return extracted;
    }

    private static void handleEntrySecure(ZipFile zf, ZipEntry entry, Path base, List<Path> extracted) throws IOException {
        // Use entry name as POSIX-like path; normalize against base
        String name = entry.getName();

        // Skip entries with null/empty names
        if (name == null || name.trim().isEmpty()) return;

        Path target = base.resolve(name).normalize();

        // Ensure target is within base to prevent Zip Slip
        if (!target.startsWith(base)) {
            return; // skip dangerous entry
        }

        if (entry.isDirectory() || name.endsWith("/")) {
            Files.createDirectories(target);
            return;
        }

        // Ensure parent directory exists
        Path parent = target.getParent();
        if (parent != null) {
            Files.createDirectories(parent);
        }

        // Do not follow symlinks on target path
        if (Files.exists(target, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(target)) {
            return; // skip writing over symlinks
        }

        try (InputStream is = new BufferedInputStream(zf.getInputStream(entry));
             OutputStream os = new BufferedOutputStream(Files.newOutputStream(
                     target,
                     StandardOpenOption.CREATE,
                     StandardOpenOption.TRUNCATE_EXISTING,
                     StandardOpenOption.WRITE))) {
            byte[] buffer = new byte[8192];
            int read;
            while ((read = is.read(buffer)) != -1) {
                os.write(buffer, 0, read);
            }
        }
        extracted.add(target);
    }

    // Helper to safely delete contents of /tmp/unpack between tests
    private static void safeDeleteRecursively(Path dir) throws IOException {
        if (dir == null) return;
        Path target = dir.toAbsolutePath().normalize();
        Path allowed = Paths.get("/tmp/unpack").toAbsolutePath().normalize();
        if (!target.equals(allowed)) {
            // To be extra safe, only allow deletion of exactly /tmp/unpack
            return;
        }
        if (!Files.exists(target)) return;

        Files.walkFileTree(target, EnumSet.noneOf(FileVisitOption.class), Integer.MAX_VALUE, new SimpleFileVisitor<Path>() {
            @Override public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
                try {
                    Files.deleteIfExists(file);
                } catch (IOException ignored) {}
                return FileVisitResult.CONTINUE;
            }
            @Override public FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException {
                try {
                    Files.deleteIfExists(dir);
                } catch (DirectoryNotEmptyException | FileAlreadyExistsException ignored) {}
                return FileVisitResult.CONTINUE;
            }
        });
    }

    // Helper to create zip files for tests
    private static void createZip(Path zipPath, Map<String, byte[]> entries) throws IOException {
        if (zipPath.getParent() != null) Files.createDirectories(zipPath.getParent());
        try (ZipOutputStream zos = new ZipOutputStream(Files.newOutputStream(zipPath,
                StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE))) {
            for (Map.Entry<String, byte[]> e : entries.entrySet()) {
                String name = e.getKey();
                if (name == null || name.isEmpty()) continue;
                // Normalize to use forward slashes
                name = name.replace("\\", "/");
                boolean isDir = name.endsWith("/");
                ZipEntry ze = new ZipEntry(name);
                zos.putNextEntry(ze);
                if (!isDir) {
                    byte[] data = (e.getValue() != null) ? e.getValue() : new byte[0];
                    zos.write(data);
                }
                zos.closeEntry();
            }
        }
    }

    private static void printResult(String testName, List<Path> extracted) {
        System.out.println("Test: " + testName);
        for (Path p : extracted) {
            System.out.println("  Extracted: " + p.toString());
        }
        if (extracted.isEmpty()) {
            System.out.println("  No files extracted.");
        }
    }

    public static void main(String[] args) throws Exception {
        Path dest = Paths.get("/tmp/unpack");
        Path zip = Paths.get("archive.zip");

        // Test 1: Basic files and directory
        safeDeleteRecursively(dest);
        Map<String, byte[]> entries1 = new LinkedHashMap<>();
        entries1.put("a.txt", "hello".getBytes());
        entries1.put("dir/", null);
        entries1.put("dir/b.txt", "world".getBytes());
        createZip(zip, entries1);
        List<Path> out1 = unzipSecure(zip, dest);
        printResult("Basic files and directory", out1);

        // Test 2: Nested directories and files
        safeDeleteRecursively(dest);
        Map<String, byte[]> entries2 = new LinkedHashMap<>();
        entries2.put("nested/", null);
        entries2.put("nested/level1/", null);
        entries2.put("nested/level1/level2/", null);
        entries2.put("nested/level1/level2/c.txt", "deep".getBytes());
        createZip(zip, entries2);
        List<Path> out2 = unzipSecure(zip, dest);
        printResult("Nested directories", out2);

        // Test 3: Path traversal attempts
        safeDeleteRecursively(dest);
        Map<String, byte[]> entries3 = new LinkedHashMap<>();
        entries3.put("../evil.txt", "should not extract".getBytes());
        entries3.put("safe/d.txt", "ok".getBytes());
        entries3.put("x/../../evil2.txt", "should not extract".getBytes());
        createZip(zip, entries3);
        List<Path> out3 = unzipSecure(zip, dest);
        printResult("Path traversal prevention", out3);

        // Test 4: Absolute path entry
        safeDeleteRecursively(dest);
        Map<String, byte[]> entries4 = new LinkedHashMap<>();
        entries4.put("/abs.txt", "abs".getBytes()); // absolute-like path should be skipped
        entries4.put("regular/e.txt", "fine".getBytes());
        createZip(zip, entries4);
        List<Path> out4 = unzipSecure(zip, dest);
        printResult("Absolute path entry handling", out4);

        // Test 5: Empty archive
        safeDeleteRecursively(dest);
        Map<String, byte[]> entries5 = new LinkedHashMap<>();
        createZip(zip, entries5);
        List<Path> out5 = unzipSecure(zip, dest);
        printResult("Empty archive", out5);
    }
}