import java.nio.file.*;
import java.nio.charset.StandardCharsets;

public class Lse43 {
    public static String readFromSafe(String filename) {
        try {
            Path base = Paths.get("/safe").toRealPath();
            Path target = base.resolve(filename).normalize().toAbsolutePath();
            if (!target.startsWith(base)) {
                return "ERROR: Access outside /safe is not allowed";
            }
            return Files.readString(target, StandardCharsets.UTF_8);
        } catch (Exception e) {
            return "ERROR: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "example.txt",
            "notes.md",
            "data.json",
            "subdir/file.txt",
            "../etc/passwd"
        };
        for (String t : tests) {
            System.out.println("== " + t + " ==");
            System.out.println(readFromSafe(t));
            System.out.println();
        }
    }
}