import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

public class Task70 {
    public static String readFile(String filePath) throws IOException {
        byte[] bytes = Files.readAllBytes(Path.of(filePath));
        return new String(bytes);
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            for (String path : args) {
                try {
                    String content = readFile(path);
                    System.out.print(content);
                    if (!content.endsWith("\n")) System.out.println();
                } catch (IOException e) {
                    System.err.println("ERROR reading " + path + ": " + e.getMessage());
                }
            }
            return;
        }

        // 5 test cases using temporary files if no command-line args provided
        try {
            for (int i = 1; i <= 5; i++) {
                Path tmp = Files.createTempFile("task70_test_" + i + "_", ".txt");
                String content = "Test case " + i + " content.\n";
                Files.writeString(tmp, content);
                String read = readFile(tmp.toString());
                System.out.print(read);
                Files.deleteIfExists(tmp);
            }
        } catch (IOException e) {
            System.err.println("ERROR during tests: " + e.getMessage());
        }
    }
}