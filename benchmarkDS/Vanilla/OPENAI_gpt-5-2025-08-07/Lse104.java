import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.EnumSet;

public class Lse104 {
    public static int writeImportantConfig(String filePath) throws IOException {
        byte[] data = "important_config".getBytes(StandardCharsets.UTF_8);
        try (SeekableByteChannel ch = Files.newByteChannel(
                Paths.get(filePath),
                EnumSet.of(StandardOpenOption.READ, StandardOpenOption.WRITE, StandardOpenOption.CREATE))) {
            return ch.write(ByteBuffer.wrap(data));
        }
    }

    public static void main(String[] args) throws Exception {
        String[] tests = {
            "java_test1.txt",
            "java_test2.txt",
            "java_test3.txt",
            "java_test4.txt",
            "java_test5.txt"
        };
        for (String t : tests) {
            int n = writeImportantConfig(t);
            System.out.println("Wrote " + n + " bytes to " + t);
        }
    }
}