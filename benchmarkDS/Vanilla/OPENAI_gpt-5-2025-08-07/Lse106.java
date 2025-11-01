import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Lse106 {
    public static String writeSecret(String filename, String secret) {
        try {
            Path p = Paths.get(filename);
            Files.write(p, secret.getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
            File f = p.toFile();
            f.setReadable(true, true);
            f.setWritable(false, false);
            f.setExecutable(false, false);
            return f.getName();
        } catch (IOException e) {
            throw new RuntimeException("Failed to write secret: " + e.getMessage(), e);
        }
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            String fname = "secret_java_" + System.currentTimeMillis() + "_" + i + ".txt";
            String name = writeSecret(fname, "Super secret info #" + i);
            System.out.println(name);
            try { Thread.sleep(2); } catch (InterruptedException ignored) {}
        }
    }
}