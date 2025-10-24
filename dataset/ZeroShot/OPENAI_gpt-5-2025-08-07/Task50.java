import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.SecureRandom;
import java.util.*;

public class Task50 {
    private static final long MAX_SIZE = 5L * 1024L * 1024L; // 5 MB
    private static final Set<String> ALLOWED_EXT = new HashSet<>(Arrays.asList(
            "txt", "pdf", "png", "jpg", "jpeg", "gif", "bin"
    ));
    private static final SecureRandom RNG = new SecureRandom();

    public static String uploadFile(Path storageDir, String originalFilename, byte[] content) throws IOException {
        Objects.requireNonNull(storageDir, "storageDir");
        Objects.requireNonNull(originalFilename, "originalFilename");
        Objects.requireNonNull(content, "content");

        if (content.length == 0) {
            throw new IllegalArgumentException("File content is empty.");
        }
        if (content.length > MAX_SIZE) {
            throw new IllegalArgumentException("File too large. Limit is " + MAX_SIZE + " bytes.");
        }

        // Create storage directory securely
        createDirectoriesSecure(storageDir);

        String safeName = sanitizeFilename(originalFilename);
        String ext = getAllowedExtensionOrDefault(safeName, "bin");
        String uniqueName = generateUniqueName(ext);

        Path finalPath = storageDir.resolve(uniqueName).normalize();

        // Write to temp file then move atomically
        Path temp = Files.createTempFile(storageDir, "upload-", ".tmp");
        try {
            Files.write(temp, content, StandardOpenOption.TRUNCATE_EXISTING);
            setOwnerOnlyPermissions(temp, true);
            try {
                Files.move(temp, finalPath, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(temp, finalPath, StandardCopyOption.REPLACE_EXISTING);
            }
            setOwnerOnlyPermissions(finalPath, false);
        } finally {
            // Best-effort cleanup if something failed before move
            try {
                Files.deleteIfExists(temp);
            } catch (Exception ignored) {}
        }

        return "Upload successful. Stored as " + finalPath.getFileName().toString() + " (" + content.length + " bytes).";
    }

    private static void createDirectoriesSecure(Path dir) throws IOException {
        Path normalized = dir.toAbsolutePath().normalize();
        Files.createDirectories(normalized);
        setDirOwnerOnlyPermissions(normalized);
    }

    private static void setDirOwnerOnlyPermissions(Path dir) {
        try {
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rwx------");
            Files.setPosixFilePermissions(dir, perms);
        } catch (UnsupportedOperationException | IOException ignored) {
        }
    }

    private static void setOwnerOnlyPermissions(Path path, boolean isTemp) {
        try {
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            Files.setPosixFilePermissions(path, perms);
        } catch (UnsupportedOperationException | IOException ignored) {
        }
    }

    private static String sanitizeFilename(String name) {
        // Keep only [A-Za-z0-9._-], limit length
        StringBuilder sb = new StringBuilder();
        for (char c : name.toCharArray()) {
            if (Character.isLetterOrDigit(c) || c == '.' || c == '_' || c == '-') {
                sb.append(c);
            } else {
                sb.append('_');
            }
        }
        String cleaned = sb.toString();
        if (cleaned.length() > 255) {
            cleaned = cleaned.substring(0, 255);
        }
        // Remove leading dots to avoid hidden files
        while (cleaned.startsWith(".")) {
            cleaned = cleaned.substring(1);
        }
        if (cleaned.isEmpty()) {
            cleaned = "file";
        }
        return cleaned;
    }

    private static String getAllowedExtensionOrDefault(String filename, String defExt) {
        String ext = "";
        int dot = filename.lastIndexOf('.');
        if (dot >= 0 && dot < filename.length() - 1) {
            ext = filename.substring(dot + 1).toLowerCase(Locale.ROOT);
        }
        if (!ALLOWED_EXT.contains(ext)) {
            return defExt;
        }
        return ext;
    }

    private static String generateUniqueName(String ext) {
        byte[] buf = new byte[16];
        RNG.nextBytes(buf);
        StringBuilder hex = new StringBuilder();
        for (byte b : buf) {
            hex.append(String.format("%02x", b));
        }
        long ts = System.currentTimeMillis();
        return "f-" + ts + "-" + hex + "." + ext;
    }

    public static void main(String[] args) {
        Path storage = Paths.get("uploads_java");

        // Test cases
        List<String> tests = new ArrayList<>();
        tests.add("hello.txt");
        tests.add("../../etc/passwd");
        tests.add("image.PNG");
        tests.add("script.exe");
        tests.add("..hidden..file..name..pdf");

        List<byte[]> contents = new ArrayList<>();
        contents.add("Hello World".getBytes());
        contents.add("Not a passwd".getBytes());
        contents.add(new byte[]{1,2,3,4,5});
        contents.add("binary data".getBytes());
        // Create an over-limit content to test rejection
        byte[] big = new byte[(int)Math.min(MAX_SIZE + 1024, Integer.MAX_VALUE)];
        new SecureRandom().nextBytes(big);
        contents.add(big);

        for (int i = 0; i < tests.size(); i++) {
            try {
                String msg = uploadFile(storage, tests.get(i), contents.get(i));
                System.out.println((i+1) + ": " + msg);
            } catch (Exception e) {
                System.out.println((i+1) + ": Upload failed: " + e.getMessage());
            }
        }
    }
}