import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.regex.Pattern;

public class Lse44 {
    private static final Path BASE_DIR = Paths.get("/safe");
    private static final long MAX_SIZE = 4L * 1024L * 1024L; // 4 MiB
    private static final Pattern NAME_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,255}$");

    public static byte[] readSafeFile(String name) throws IOException {
        if (name == null || !NAME_PATTERN.matcher(name).matches()) {
            throw new IOException("invalid input");
        }

        Path target = BASE_DIR.resolve(name).normalize();
        if (!target.startsWith(BASE_DIR)) {
            throw new IOException("invalid path");
        }

        try (SeekableByteChannel ch = Files.newByteChannel(target,
                StandardOpenOption.READ,
                LinkOption.NOFOLLOW_LINKS)) {

            long size = ch.size();
            if (size < 0 || size > MAX_SIZE || size > Integer.MAX_VALUE) {
                throw new IOException("file too large");
            }

            ByteBuffer buf = ByteBuffer.allocate((int) size);
            while (buf.hasRemaining()) {
                int r = ch.read(buf);
                if (r < 0) break;
            }
            return buf.array();
        } catch (NoSuchFileException e) {
            throw new IOException("not found");
        } catch (FileSystemException e) {
            // Covers attempts to read directories or symlink issues with NOFOLLOW_LINKS
            throw new IOException("access denied");
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
                "example.txt",
                "does_not_exist.txt",
                "../passwd",
                "valid-file_123",
                "bad\\name"
        };
        for (String t : tests) {
            try {
                byte[] data = readSafeFile(t);
                System.out.println("OK " + t + " size=" + data.length + " sha1=" + sha1Hex(data));
            } catch (Exception ex) {
                System.out.println("ERR " + t);
            }
        }
    }

    // Simple SHA-1 for test output uniqueness (not for security)
    private static String sha1Hex(byte[] data) {
        try {
            java.security.MessageDigest md = java.security.MessageDigest.getInstance("SHA-1");
            byte[] d = md.digest(data);
            StringBuilder sb = new StringBuilder(d.length * 2);
            for (byte b : d) sb.append(String.format("%02x", b));
            return sb.toString();
        } catch (Exception e) {
            return "";
        }
    }
}