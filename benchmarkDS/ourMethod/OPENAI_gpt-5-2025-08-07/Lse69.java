import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

public class Lse69 {

    private static final long MAX_SIZE = 8L * 1024 * 1024; // 8 MiB
    private static final Pattern SAFE_NAME = Pattern.compile("^[A-Za-z0-9._-]{1,255}$");

    private static boolean isSafeFilename(String name) {
        if (name == null) return false;
        if (name.length() == 0 || name.length() > 255) return false;
        if (name.equals(".") || name.equals("..")) return false;
        if (name.contains("/") || name.contains("\\")) return false;
        return SAFE_NAME.matcher(name).matches();
    }

    public static String upload_image(Path baseDir, String inputName, String outputName) {
        // Fail-closed on invalid inputs
        if (baseDir == null || !isSafeFilename(inputName) || !isSafeFilename(outputName)) {
            return null;
        }
        try {
            // Create base directory if missing, with restrictive permissions where possible
            if (Files.notExists(baseDir)) {
                try {
                    Files.createDirectories(baseDir);
                    try {
                        Files.setPosixFilePermissions(baseDir, PosixFilePermissions.fromString("rwx------"));
                    } catch (UnsupportedOperationException ignored) {
                        // Non-POSIX FS, ignore
                    }
                } catch (IOException e) {
                    return null;
                }
            }

            Path inputPath = baseDir.resolve(inputName).normalize();
            // Ensure we remain inside baseDir
            if (!inputPath.getParent().equals(baseDir.toAbsolutePath().normalize())) {
                return null;
            }

            BasicFileAttributes attrs;
            try {
                attrs = Files.readAttributes(inputPath, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            } catch (IOException e) {
                return null;
            }
            if (!attrs.isRegularFile()) {
                return null;
            }
            long size = attrs.size();
            if (size < 0 || size > MAX_SIZE) {
                return null;
            }

            byte[] data = new byte[(int) size];
            int readTotal = 0;
            try (InputStream is = Files.newInputStream(inputPath, StandardOpenOption.READ)) {
                while (readTotal < data.length) {
                    int r = is.read(data, readTotal, data.length - readTotal);
                    if (r < 0) break;
                    readTotal += r;
                }
            } catch (IOException e) {
                return null;
            }
            if (readTotal != data.length) {
                return null;
            }

            byte[] b64 = Base64.getEncoder().encode(data);
            String encoded = new String(b64, StandardCharsets.UTF_8);

            // Prepare output temp file within baseDir
            Path tmpFile;
            try {
                Set<StandardOpenOption> options = new HashSet<>();
                options.add(StandardOpenOption.CREATE);
                options.add(StandardOpenOption.TRUNCATE_EXISTING);
                options.add(StandardOpenOption.WRITE);
                tmpFile = Files.createTempFile(baseDir, "enc-", ".tmp");
                try {
                    Files.setPosixFilePermissions(tmpFile, PosixFilePermissions.fromString("rw-------"));
                } catch (UnsupportedOperationException ignored) {
                    // non-POSIX
                }
                try (BufferedWriter writer = Files.newBufferedWriter(tmpFile, StandardCharsets.UTF_8, StandardOpenOption.WRITE)) {
                    writer.write(encoded);
                    writer.flush();
                }
                try (FileChannel ch = FileChannel.open(tmpFile, StandardOpenOption.WRITE)) {
                    ch.force(true);
                }
            } catch (IOException e) {
                return null;
            }

            Path outputPath = baseDir.resolve(outputName).normalize();
            if (!outputPath.getParent().equals(baseDir.toAbsolutePath().normalize())) {
                try {
                    Files.deleteIfExists(tmpFile);
                } catch (IOException ignored) {}
                return null;
            }

            try {
                // Try atomic move without overwrite
                Files.move(tmpFile, outputPath, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException e) {
                // Fallback to non-atomic move only if destination does not exist
                if (Files.exists(outputPath, LinkOption.NOFOLLOW_LINKS)) {
                    try {
                        Files.deleteIfExists(tmpFile);
                    } catch (IOException ignored) {}
                    return null;
                }
                try {
                    Files.move(tmpFile, outputPath);
                } catch (IOException ex) {
                    try {
                        Files.deleteIfExists(tmpFile);
                    } catch (IOException ignored) {}
                    return null;
                }
            } catch (IOException e) {
                try {
                    Files.deleteIfExists(tmpFile);
                } catch (IOException ignored) {}
                return null;
            }

            try {
                Files.setPosixFilePermissions(outputPath, PosixFilePermissions.fromString("rw-------"));
            } catch (UnsupportedOperationException ignored) {
                // Non-POSIX
            } catch (IOException ignored) {
                // Keep file anyway
            }
            return outputPath.toAbsolutePath().toString();
        } catch (Exception e) {
            return null;
        }
    }

    private static void writeSampleFile(Path baseDir, String name, byte[] content) throws IOException {
        Path p = baseDir.resolve(name).normalize();
        if (!p.getParent().equals(baseDir.toAbsolutePath().normalize())) {
            throw new IOException("invalid path");
        }
        Path tmp = Files.createTempFile(baseDir, "in-", ".tmp");
        try (FileChannel ch = FileChannel.open(tmp, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {
            ch.write(ByteBuffer.wrap(content));
            ch.force(true);
        }
        try {
            Files.move(tmp, p, StandardCopyOption.ATOMIC_MOVE);
        } catch (AtomicMoveNotSupportedException e) {
            Files.move(tmp, p);
        }
        try {
            Files.setPosixFilePermissions(p, PosixFilePermissions.fromString("rw-------"));
        } catch (UnsupportedOperationException ignored) {}
    }

    public static void main(String[] args) {
        Path base = Paths.get("base_java").toAbsolutePath().normalize();
        try {
            Files.createDirectories(base);
        } catch (IOException e) {
            System.out.println("Setup failed");
            return;
        }
        // Create 5 sample "image" files (binary content)
        SecureRandom rng = new SecureRandom();
        try {
            byte[] c1 = new byte[32]; rng.nextBytes(c1); writeSampleFile(base, "img1.bin", c1);
            byte[] c2 = new byte[64]; rng.nextBytes(c2); writeSampleFile(base, "img2.jpg", c2);
            byte[] c3 = new byte[128]; rng.nextBytes(c3); writeSampleFile(base, "img3.png", c3);
            byte[] c4 = new byte[16]; rng.nextBytes(c4); writeSampleFile(base, "icon4.gif", c4);
            byte[] c5 = new byte[256]; rng.nextBytes(c5); writeSampleFile(base, "photo5.tif", c5);
        } catch (IOException e) {
            System.out.println("Setup files failed");
            return;
        }

        String[] inputs = {"img1.bin", "img2.jpg", "img3.png", "icon4.gif", "photo5.tif"};
        String[] outputs = {"img1.b64", "img2.b64", "img3.b64", "icon4.b64", "photo5.b64"};

        for (int i = 0; i < inputs.length; i++) {
            String result = upload_image(base, inputs[i], outputs[i]);
            if (result != null) {
                System.out.println("OK: " + result);
            } else {
                System.out.println("FAIL");
            }
        }
    }
}