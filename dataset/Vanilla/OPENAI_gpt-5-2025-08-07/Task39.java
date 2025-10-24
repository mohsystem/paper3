import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

public class Task39 {
    public static String displayFile(String filename) throws IOException, InterruptedException {
        List<String> cmd;
        String os = System.getProperty("os.name").toLowerCase();
        if (os.contains("win")) {
            cmd = Arrays.asList("cmd", "/c", "type", filename);
        } else {
            cmd = Arrays.asList("cat", filename);
        }
        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);
        Process process = pb.start();
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try (InputStream is = process.getInputStream()) {
            byte[] buf = new byte[4096];
            int n;
            while ((n = is.read(buf)) != -1) {
                baos.write(buf, 0, n);
            }
        }
        process.waitFor();
        return baos.toString(StandardCharsets.UTF_8);
    }

    public static void main(String[] args) throws Exception {
        String[] names = {
            "file1.txt",
            "empty.txt",
            "multi line.txt",
            "unicode.txt",
            "numbers.txt"
        };
        String[] contents = {
            "Hello world\nSecond line\n",
            "",
            "Line A\nLine B\nLine C\n",
            "こんにちは世界\nПривет мир\nHola mundo\n",
            "1\n2\n3\n4\n5\n"
        };

        for (int i = 0; i < names.length; i++) {
            Files.write(Paths.get(names[i]), contents[i].getBytes(StandardCharsets.UTF_8));
        }

        for (String n : names) {
            String out = displayFile(n);
            System.out.println("--- " + n + " ---");
            System.out.print(out);
        }
    }
}