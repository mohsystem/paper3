import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.*;

public class Task121 {
    private static final long MAX_SIZE_BYTES = 5L * 1024 * 1024; // 5 MiB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(Arrays.asList(
            "txt", "pdf", "png", "jpg", "jpeg", "gif"
    ));
    private static final String UPLOAD_DIR = "uploads";

    public static String uploadFile(String originalName, byte[] content) throws IOException {
        if (originalName == null || content == null) {
            throw new IllegalArgumentException("Filename and content must not be null.");
        }
        if (content.length == 0) {
            throw new IllegalArgumentException("Empty content not allowed.");
        }
        if (content.length > MAX_SIZE_BYTES) {
            throw new IllegalArgumentException("File exceeds maximum allowed size.");
        }
        if (originalName.contains("..") || originalName.contains("/") || originalName.contains("\\")) {
            throw new IllegalArgumentException("Invalid filename (path traversal detected).");
        }

        String base = Paths.get(originalName).getFileName().toString();
        String sanitized = sanitizeFileName(base);
        if (sanitized.isEmpty()) {
            throw new IllegalArgumentException("Invalid sanitized filename.");
        }

        String ext = getExtension(sanitized);
        if (!ALLOWED_EXTENSIONS.contains(ext)) {
            throw new IllegalArgumentException("Disallowed file extension.");
        }

        ensureUploadDir();

        String uniqueName = UUID.randomUUID().toString().replace("-", "") + "." + ext;
        Path uploadDir = Paths.get(UPLOAD_DIR).toAbsolutePath().normalize();
        Path dest = uploadDir.resolve(uniqueName).normalize();
        if (!dest.startsWith(uploadDir)) {
            throw new SecurityException("Resolved path escapes upload directory.");
        }

        try {
            Files.write(dest, content, StandardOpenOption.CREATE_NEW);
        } catch (FileAlreadyExistsException e) {
            throw new IOException("Name collision, try again.", e);
        }

        // Try to harden permissions where supported
        try {
            if (Files.getFileStore(dest).supportsFileAttributeView("posix")) {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                Files.setPosixFilePermissions(dest, perms);
            } else {
                dest.toFile().setReadable(true, true);
                dest.toFile().setWritable(true, true);
                dest.toFile().setExecutable(false, false);
            }
        } catch (Exception ignored) { }

        return dest.toString();
    }

    private static void ensureUploadDir() throws IOException {
        Path p = Paths.get(UPLOAD_DIR);
        if (!Files.exists(p)) {
            Files.createDirectories(p);
        }
        try {
            if (Files.getFileStore(p).supportsFileAttributeView("posix")) {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rwx------");
                Files.setPosixFilePermissions(p, perms);
            } else {
                p.toFile().setReadable(true, true);
                p.toFile().setWritable(true, true);
                p.toFile().setExecutable(true, true);
            }
        } catch (Exception ignored) { }
    }

    private static String sanitizeFileName(String name) {
        StringBuilder sb = new StringBuilder();
        for (char c : name.toCharArray()) {
            if ((c >= 'a' && c <= 'z')
                    || (c >= 'A' && c <= 'Z')
                    || (c >= '0' && c <= '9')
                    || c == '.' || c == '_' || c == '-') {
                sb.append(c);
            }
        }
        String s = sb.toString();
        if (s.startsWith(".")) {
            s = s.replaceFirst("^\\.+", "");
        }
        // prevent dotless names
        if (!s.contains(".")) return "";
        // limit length
        if (s.length() > 255) s = s.substring(s.length() - 255);
        return s;
    }

    private static String getExtension(String name) {
        int idx = name.lastIndexOf('.');
        if (idx <= 0 || idx == name.length() - 1) return "";
        String ext = name.substring(idx + 1).toLowerCase(Locale.ROOT);
        if (ext.length() > 10) return "";
        return ext;
    }

    public static void main(String[] args) {
        // 5 test cases
        List<String> names = Arrays.asList(
                "hello.txt",
                "image.jpg",
                "report.pdf",
                "evil/../secret.txt",
                "malware.exe"
        );

        List<byte[]> contents = new ArrayList<>();
        contents.add("Hello, World!".getBytes(java.nio.charset.StandardCharsets.UTF_8));
        contents.add(new byte[]{(byte)0xFF, (byte)0xD8, (byte)0xFF, (byte)0xD9}); // JPEG SOI+EOI
        contents.add("%PDF-1.4\n%\u00E2\u00E3\u00CF\u00D3\n".getBytes(java.nio.charset.StandardCharsets.UTF_8));
        contents.add("top secret".getBytes(java.nio.charset.StandardCharsets.UTF_8));
        contents.add(new byte[]{'M','Z',0,0}); // EXE signature

        for (int i = 0; i < names.size(); i++) {
            try {
                String saved = uploadFile(names.get(i), contents.get(i));
                System.out.println("Saved: " + saved);
            } catch (Exception e) {
                System.out.println("Failed: " + names.get(i) + " -> " + e.getMessage());
            }
        }
    }
}