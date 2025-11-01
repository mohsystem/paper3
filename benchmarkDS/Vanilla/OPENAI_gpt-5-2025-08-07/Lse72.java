import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class Lse72 {
    public static String uploadFile(String filename, byte[] data) throws IOException {
        File dir = new File("uploads");
        if (!dir.exists()) {
            dir.mkdirs();
        }
        File out = new File(dir, filename);
        try (FileOutputStream fos = new FileOutputStream(out)) {
            fos.write(data);
        }
        return out.getAbsolutePath();
    }

    public static void main(String[] args) {
        try {
            System.out.println(uploadFile("java_test1.txt", "Hello from Java 1".getBytes(StandardCharsets.UTF_8)));
            System.out.println(uploadFile("java_test2.txt", "Hello from Java 2".getBytes(StandardCharsets.UTF_8)));
            System.out.println(uploadFile("java_test3.txt", "Hello from Java 3".getBytes(StandardCharsets.UTF_8)));
            System.out.println(uploadFile("java_test4.txt", "Hello from Java 4".getBytes(StandardCharsets.UTF_8)));
            System.out.println(uploadFile("java_test5.txt", "Hello from Java 5".getBytes(StandardCharsets.UTF_8)));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}