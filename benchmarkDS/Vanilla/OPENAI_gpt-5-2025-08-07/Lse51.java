import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

public class Lse51 {
    public static int writeTextFile(String path, String text) {
        try {
            Path p = Paths.get(path);
            if (p.getParent() != null) {
                Files.createDirectories(p.getParent());
            }
            byte[] data = text.getBytes(StandardCharsets.UTF_8);
            Files.write(p, data, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            return data.length;
        } catch (IOException e) {
            System.out.println("writeTextFile error: " + e.getMessage());
            return -1;
        }
    }

    public static String readTextFile(String path) {
        try {
            byte[] data = Files.readAllBytes(Paths.get(path));
            return new String(data, StandardCharsets.UTF_8);
        } catch (IOException e) {
            System.out.println("readTextFile error: " + e.getMessage());
            return null;
        }
    }

    public static List<String> extractTarToTmp(String tarPath) {
        // Not implemented: Java standard library does not support TAR extraction without external libraries.
        // This stub returns an empty list.
        return new ArrayList<>();
    }

    private static void ensureDir(String dir) {
        try {
            Files.createDirectories(Paths.get(dir));
        } catch (IOException e) {
            System.out.println("ensureDir error: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        ensureDir("/tmp/unpack");
        // Test 1: Write then read a text file
        int written = writeTextFile("/tmp/java_sample.txt", "Hello from Java with context-like try-with-resources!");
        String read = readTextFile("/tmp/java_sample.txt");
        System.out.println("Test1 - written: " + written + ", read: " + (read != null ? read : "null"));

        // Test 2: Attempt to extract an uncompressed tar (stubbed)
        List<String> files1 = extractTarToTmp("/tmp/java_sample.tar");
        System.out.println("Test2 - extracted count (stub): " + files1.size());

        // Test 3: Attempt to extract a gzip tar (stubbed)
        List<String> files2 = extractTarToTmp("/tmp/java_sample.tar.gz");
        System.out.println("Test3 - extracted count (stub): " + files2.size());

        // Test 4: Attempt to extract a bzip2 tar (stubbed)
        List<String> files3 = extractTarToTmp("/tmp/java_sample.tar.bz2");
        System.out.println("Test4 - extracted count (stub): " + files3.size());

        // Test 5: Read non-existent file
        String missing = readTextFile("/tmp/java_missing.txt");
        System.out.println("Test5 - missing file read result: " + (missing == null ? "null" : missing));
    }
}