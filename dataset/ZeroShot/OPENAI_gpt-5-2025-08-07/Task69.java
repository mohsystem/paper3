import java.io.IOException;
import java.nio.file.*;

public class Task69 {
    public static boolean deleteFileSecure(String path) {
        if (path == null || path.trim().isEmpty()) return false;
        try {
            Path p = Paths.get(path).toAbsolutePath().normalize();
            if (p.getParent() == null) return false; // avoid root or drive root
            if (!Files.exists(p, LinkOption.NOFOLLOW_LINKS)) return false;
            if (Files.isDirectory(p, LinkOption.NOFOLLOW_LINKS)) return false; // do not delete directories
            Files.delete(p); // deletes files and symlinks (the link itself)
            return true;
        } catch (SecurityException | IOException e) {
            return false;
        }
    }

    private static void runTests() {
        try {
            Path t1 = Files.createTempFile("task69_test1_", ".tmp");
            Path t2 = Files.createTempFile("task69_test2_", ".tmp");
            Path t3 = Files.createTempFile("task69_test3_", ".tmp");
            Path dir = Files.createTempDirectory("task69_dir_");
            Path nonexist = dir.resolve("nonexistent_" + System.nanoTime() + ".tmp");

            System.out.println("Test1 delete file: " + deleteFileSecure(t1.toString()));
            System.out.println("Test2 delete file: " + deleteFileSecure(t2.toString()));
            System.out.println("Test3 delete directory (should be false): " + deleteFileSecure(dir.toString()));
            System.out.println("Test4 delete non-existent (should be false): " + deleteFileSecure(nonexist.toString()));
            System.out.println("Test5 delete file: " + deleteFileSecure(t3.toString()));

            try { Files.deleteIfExists(dir); } catch (IOException ignored) {}
        } catch (IOException e) {
            System.out.println("Setup failed: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        if (args != null && args.length > 0) {
            for (String p : args) {
                boolean ok = deleteFileSecure(p);
                System.out.println(p + " -> " + ok);
            }
        } else {
            runTests();
        }
    }
}