import java.io.IOException;
import java.io.OutputStream;
import java.net.URI;
import java.nio.file.*;
import java.util.HashSet;
import java.util.Set;

public class Lse72 {

    // Chain-of-Through process:
    // 1) Problem understanding:
    //    Implement a secure upload_file function that saves data to an 'uploads' folder located in the same directory as the source/class,
    //    ensuring proper sanitization, validation, and safety checks, returning the saved file path.
    //
    // 2) Security requirements:
    //    - Prevent directory traversal by sanitizing filenames.
    //    - Restrict allowed file extensions.
    //    - Enforce size limits.
    //    - Avoid overwriting existing files by using unique names.
    //    - Normalize and validate paths.
    //
    // 3) Secure coding generation:
    //    Implement the logic step-by-step in code with validations and safe operations.
    //
    // 4) Code review:
    //    Checked for potential issues like null checks, path normalization, extension whitelist, and size limits.
    //
    // 5) Secure code output:
    //    Final function below applies all mitigations.

    public static Path upload_file(String originalFilename, byte[] data) throws IOException {
        if (originalFilename == null) {
            throw new IllegalArgumentException("Filename cannot be null");
        }
        if (data == null) {
            throw new IllegalArgumentException("Data cannot be null");
        }

        final long MAX_SIZE = 5L * 1024 * 1024; // 5 MB
        if (data.length > MAX_SIZE) {
            throw new IllegalArgumentException("File too large");
        }

        // Allowed extensions
        Set<String> allowed = new HashSet<>();
        allowed.add("txt");
        allowed.add("pdf");
        allowed.add("png");
        allowed.add("jpg");
        allowed.add("jpeg");
        allowed.add("gif");
        allowed.add("bin");

        // Extract extension
        String name = originalFilename.trim();
        // Remove any path components just in case
        name = name.replace('\\', '/');
        if (name.contains("/")) {
            name = name.substring(name.lastIndexOf('/') + 1);
        }

        int dot = name.lastIndexOf('.');
        String ext = "";
        String base = name;
        if (dot > 0 && dot < name.length() - 1) {
            ext = name.substring(dot + 1).toLowerCase();
            base = name.substring(0, dot);
        }

        if (ext.isEmpty() || !allowed.contains(ext)) {
            throw new IllegalArgumentException("Disallowed or missing file extension");
        }

        // Sanitize base name: keep only safe characters
        StringBuilder sb = new StringBuilder();
        for (char c : base.toCharArray()) {
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '.' || c == '_' || c == '-') {
                sb.append(c);
            } else {
                sb.append('_');
            }
        }
        String safeBase = sb.toString();
        safeBase = safeBase.replaceAll("^\\.+", ""); // no leading dots
        if (safeBase.isEmpty()) {
            safeBase = "file";
        }
        if (safeBase.length() > 200) {
            safeBase = safeBase.substring(0, 200);
        }

        String safeName = safeBase + "." + ext;

        // Determine base directory (same dir as this class/jar if possible)
        Path baseDir;
        try {
            URI uri = Lse72.class.getProtectionDomain().getCodeSource().getLocation().toURI();
            Path codePath = Paths.get(uri);
            baseDir = Files.isDirectory(codePath) ? codePath : codePath.getParent();
            if (baseDir == null) {
                baseDir = Paths.get(System.getProperty("user.dir"));
            }
        } catch (Exception e) {
            baseDir = Paths.get(System.getProperty("user.dir"));
        }

        Path uploadDir = baseDir.resolve("uploads").normalize();
        // Ensure uploads directory within baseDir
        if (!uploadDir.startsWith(baseDir.normalize())) {
            throw new SecurityException("Invalid upload directory path");
        }
        Files.createDirectories(uploadDir);

        // Generate a unique filename and write atomically (CREATE_NEW)
        Path target = uploadDir.resolve(safeName);
        int counter = 1;
        while (true) {
            try (OutputStream os = Files.newOutputStream(target, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
                os.write(data);
                break; // success
            } catch (FileAlreadyExistsException ex) {
                String candidate = safeBase + "_" + counter + "." + ext;
                target = uploadDir.resolve(candidate);
                counter++;
            }
        }

        return target.toAbsolutePath().normalize();
    }

    // 5 test cases in main
    public static void main(String[] args) {
        try {
            // Test 1: valid text file
            Path p1 = upload_file("report.txt", "Hello, World!".getBytes());
            System.out.println("Saved: " + p1);

            // Test 2: directory traversal attempt
            try {
                Path p2 = upload_file("../../secret.txt", "Top secret".getBytes());
                System.out.println("Saved: " + p2);
            } catch (Exception e) {
                System.out.println("Blocked traversal: " + e.getMessage());
            }

            // Test 3: disallowed extension
            try {
                Path p3 = upload_file("script.sh", "#!/bin/sh\necho hi".getBytes());
                System.out.println("Saved: " + p3);
            } catch (Exception e) {
                System.out.println("Blocked extension: " + e.getMessage());
            }

            // Test 4: weird characters in name with allowed extension
            Path p4 = upload_file("inva|id:name?.png", new byte[] {0x1, 0x2, 0x3});
            System.out.println("Saved: " + p4);

            // Test 5: duplicate filename should create unique
            Path p5 = upload_file("report.txt", "Another content".getBytes());
            System.out.println("Saved: " + p5);

        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
}