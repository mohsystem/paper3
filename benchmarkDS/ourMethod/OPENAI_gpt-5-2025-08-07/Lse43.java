import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.channels.SeekableByteChannel;
import java.nio.file.DirectoryStream;
import java.nio.file.LinkOption;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.SecureDirectoryStream;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.BasicFileAttributeView;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.Files;
import java.util.EnumSet;

public class Lse43 {
    private static final Path BASE_DIR = Paths.get("/safe");
    private static final long MAX_FILE_SIZE = 10L * 1024L * 1024L; // 10 MiB

    public static byte[] readFromSafe(String name) throws IOException {
        if (!isValidName(name)) {
            throw new IOException("invalid input");
        }

        if (!Files.isDirectory(BASE_DIR, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("unavailable");
        }

        // Use SecureDirectoryStream to anchor operations to the directory descriptor.
        try (DirectoryStream<Path> ds = Files.newDirectoryStream(BASE_DIR)) {
            if (!(ds instanceof SecureDirectoryStream)) {
                throw new IOException("unavailable");
            }
            SecureDirectoryStream<Path> sds = (SecureDirectoryStream<Path>) ds;

            Path rel = Paths.get(name);
            if (rel.getNameCount() != 1) {
                throw new IOException("invalid input");
            }

            // Check attributes without following symlinks
            BasicFileAttributeView view = sds.getFileAttributeView(rel, BasicFileAttributeView.class, LinkOption.NOFOLLOW_LINKS);
            if (view == null) {
                throw new IOException("not found");
            }
            BasicFileAttributes attrs = view.readAttributes();
            if (!attrs.isRegularFile()) {
                throw new IOException("not a file");
            }
            if (attrs.size() > MAX_FILE_SIZE) {
                throw new IOException("too large");
            }

            // Open and read with a strict size limit
            try (SeekableByteChannel ch = sds.newByteChannel(rel, EnumSet.of(StandardOpenOption.READ))) {
                ByteArrayOutputStream baos = new ByteArrayOutputStream((int)Math.min(attrs.size(), 8192));
                ByteBuffer buf = ByteBuffer.allocate(8192);
                long total = 0;
                while (true) {
                    buf.clear();
                    int r = ch.read(buf);
                    if (r == -1) break;
                    if (r == 0) continue;
                    total += r;
                    if (total > MAX_FILE_SIZE) {
                        throw new IOException("too large");
                    }
                    buf.flip();
                    byte[] tmp = new byte[buf.remaining()];
                    buf.get(tmp);
                    baos.write(tmp);
                }
                return baos.toByteArray();
            }
        }
    }

    private static boolean isValidName(String s) {
        if (s == null) return false;
        int len = s.length();
        if (len < 1 || len > 255) return false;
        if (".".equals(s) || "..".equals(s)) return false;
        for (int i = 0; i < len; i++) {
            char c = s.charAt(i);
            if (c == '/' || c == '\\' || c == 0) return false;
            if (!(c == '.' || c == '_' || c == '-' ||
                  (c >= '0' && c <= '9') ||
                  (c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z'))) {
                return false;
            }
        }
        return true;
    }

    // Minimal tests
    public static void main(String[] args) {
        String[] tests = new String[] {
            "example.txt",
            "nonexistent.txt",
            "../passwd",
            "symlink",
            "nested.txt"
        };
        for (String t : tests) {
            try {
                byte[] data = readFromSafe(t);
                String preview = new String(data, 0, Math.min(data.length, 64), StandardCharsets.UTF_8);
                System.out.println("Read " + t + ": " + data.length + " bytes; preview: " + preview.replace("\n", "\\n"));
            } catch (Exception e) {
                System.out.println("Read " + t + ": error");
            }
        }
    }
}