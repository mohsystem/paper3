import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

public class Task36 {

    public static String readFile(String filePath) {
        try {
            return Files.readString(Path.of(filePath), StandardCharsets.UTF_8);
        } catch (IOException e) {
            return null;
        }
    }

    public static void main(String[] args) throws Exception {
        String[] contents = new String[] {
            "Java Test File 1\nLine A\nLine B",
            "Java Test File 2\nHello World",
            "Java Test File 3\nSample Content",
            "Java Test File 4\nLorem Ipsum",
            "Java Test File 5\nEnd of File"
        };

        String[] paths = new String[5];
        for (int i = 0; i < 5; i++) {
            File f = File.createTempFile("task36_java_test_" + i + "_", ".txt");
            f.deleteOnExit();
            Files.writeString(f.toPath(), contents[i], StandardCharsets.UTF_8);
            paths[i] = f.getAbsolutePath();
        }

        for (int i = 0; i < paths.length; i++) {
            System.out.println("== Java Test " + (i + 1) + " ==");
            String data = readFile(paths[i]);
            if (data != null) {
                System.out.println(data);
            } else {
                System.out.println("Error reading file: " + paths[i]);
            }
        }
    }
}