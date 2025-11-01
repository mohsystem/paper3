import java.nio.file.*;
import java.nio.channels.SeekableByteChannel;
import java.nio.ByteBuffer;
import java.io.IOException;
import java.util.Arrays;

public class Lse103 {

    // Create or truncate and write: analogous to O_CREAT | O_WRONLY | O_TRUNC
    public static int createOrTruncateWrite(String path, byte[] data) {
        Path p = Paths.get(path);
        try {
            Files.createDirectories(p.getParent() == null ? Paths.get(".") : p.getParent());
        } catch (IOException ignored) {}
        try {
            Files.write(p, data,
                    StandardOpenOption.CREATE,
                    StandardOpenOption.WRITE,
                    StandardOpenOption.TRUNCATE_EXISTING);
            return 0;
        } catch (IOException e) {
            return -1;
        }
    }

    // Create new exclusively and write: analogous to O_CREAT | O_WRONLY | O_EXCL
    public static int createExclusiveWrite(String path, byte[] data) {
        Path p = Paths.get(path);
        try {
            Files.createDirectories(p.getParent() == null ? Paths.get(".") : p.getParent());
        } catch (IOException ignored) {}
        try (SeekableByteChannel ch = Files.newByteChannel(
                p,
                java.util.EnumSet.of(StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE))) {
            ch.write(ByteBuffer.wrap(data));
            return 0;
        } catch (FileAlreadyExistsException e) {
            return 1; // indicate exists
        } catch (IOException e) {
            return -1; // other error
        }
    }

    // Helper read content for verification (not necessary but useful)
    public static byte[] readAll(String path) {
        try {
            return Files.readAllBytes(Paths.get(path));
        } catch (IOException e) {
            return new byte[0];
        }
    }

    public static void main(String[] args) {
        String tmp = System.getProperty("java.io.tmpdir");
        long nonce = System.nanoTime();
        String pathA = Paths.get(tmp, "lse103_java_A_" + nonce + ".txt").toString();
        String pathB = Paths.get(tmp, "lse103_java_B_" + nonce + ".txt").toString();

        // Test cases (5):
        // 1) Exclusive create new file
        int r1 = createExclusiveWrite(pathA, "hello exclusive A".getBytes());
        System.out.println("T1 ex-create A: " + r1);

        // 2) Exclusive create same file again (should fail with exists)
        int r2 = createExclusiveWrite(pathA, "should fail".getBytes());
        System.out.println("T2 ex-create A again: " + r2);

        // 3) Truncate existing file and write new content
        int r3 = createOrTruncateWrite(pathA, "truncated A".getBytes());
        System.out.println("T3 truncate-write A: " + r3 + " len=" + readAll(pathA).length);

        // 4) Create-or-truncate on new file B (should create)
        int r4 = createOrTruncateWrite(pathB, "new B".getBytes());
        System.out.println("T4 create-or-truncate B: " + r4 + " len=" + readAll(pathB).length);

        // 5) Exclusive create on existing B (should fail with exists)
        int r5 = createExclusiveWrite(pathB, "should fail B".getBytes());
        System.out.println("T5 ex-create B again: " + r5);

        // Clean up (optional)
        try { Files.deleteIfExists(Paths.get(pathA)); } catch (Exception ignored) {}
        try { Files.deleteIfExists(Paths.get(pathB)); } catch (Exception ignored) {}
    }
}