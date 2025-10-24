import java.io.IOException;
import java.nio.file.*;
import java.util.Arrays;

public class Task127 {

    public static String copyToTemp(String sourcePath) {
        if (sourcePath == null || sourcePath.isBlank()) {
            System.err.println("[Java] Error: sourcePath is null or empty.");
            return null;
        }
        try {
            Path src = Paths.get(sourcePath);
            if (!Files.exists(src)) {
                System.err.println("[Java] Error: Source file does not exist: " + sourcePath);
                return null;
            }
            if (Files.isDirectory(src)) {
                System.err.println("[Java] Error: Source path is a directory, not a file: " + sourcePath);
                return null;
            }
            Path tempDir = Paths.get(System.getProperty("java.io.tmpdir"));
            String baseName = src.getFileName().toString();
            String prefix = ("copy-" + baseName).replaceAll("[^A-Za-z0-9._-]", "_");
            if (prefix.length() < 3) prefix = "cpy";
            Path dest = Files.createTempFile(tempDir, prefix + "-", ".tmp");
            Files.copy(src, dest, StandardCopyOption.REPLACE_EXISTING);
            return dest.toString();
        } catch (Exception e) {
            System.err.println("[Java] Exception while copying to temp: " + e.getMessage());
            return null;
        }
    }

    private static String createSampleFile(String content) {
        try {
            Path temp = Files.createTempFile("task127-sample-", ".txt");
            Files.write(temp, content.getBytes());
            return temp.toString();
        } catch (IOException e) {
            System.err.println("[Java] Failed to create sample file: " + e.getMessage());
            return null;
        }
    }

    private static boolean filesEqual(Path a, Path b) {
        try {
            byte[] ba = Files.readAllBytes(a);
            byte[] bb = Files.readAllBytes(b);
            return Arrays.equals(ba, bb);
        } catch (IOException e) {
            System.err.println("[Java] Failed to compare files: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        // Test 1: Valid small file
        String src1 = createSampleFile("Hello, Task127!");
        String dst1 = copyToTemp(src1);
        System.out.println("[Java][Test1] src=" + src1 + " dst=" + dst1 + " equal=" + (dst1 != null && filesEqual(Paths.get(src1), Paths.get(dst1))));

        // Test 2: Non-existent file
        String src2 = Paths.get(System.getProperty("java.io.tmpdir"), "nonexistent_12345_abcdef.txt").toString();
        String dst2 = copyToTemp(src2);
        System.out.println("[Java][Test2] src=" + src2 + " dst=" + dst2);

        // Test 3: Directory instead of file
        String src3 = System.getProperty("java.io.tmpdir");
        String dst3 = copyToTemp(src3);
        System.out.println("[Java][Test3] src=" + src3 + " dst=" + dst3);

        // Test 4: Empty string path
        String dst4 = copyToTemp("");
        System.out.println("[Java][Test4] src='' dst=" + dst4);

        // Test 5: Larger content file
        StringBuilder big = new StringBuilder();
        for (int i = 0; i < 10000; i++) big.append("Line ").append(i).append(" - Task127\n");
        String src5 = createSampleFile(big.toString());
        String dst5 = copyToTemp(src5);
        System.out.println("[Java][Test5] src=" + src5 + " dst=" + dst5 + " equal=" + (dst5 != null && filesEqual(Paths.get(src5), Paths.get(dst5))));
    }
}