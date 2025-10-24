import java.io.BufferedReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Task89 {

    public static boolean comparePassword(String filePath, String enteredPassword) {
        String stored = null;
        try (BufferedReader br = Files.newBufferedReader(Paths.get(filePath), StandardCharsets.UTF_8)) {
            stored = br.readLine();
            if (stored == null) stored = "";
        } catch (IOException e) {
            return false;
        }

        if (stored.length() != enteredPassword.length()) {
            return false;
        }
        for (int i = 0; i < stored.length(); i++) {
            if (stored.charAt(i) != enteredPassword.charAt(i)) {
                return false;
            }
        }
        return true;
    }

    private static String writeFile(String filename, String content) throws IOException {
        Path p = Paths.get(filename);
        Files.write(p, content.getBytes(StandardCharsets.UTF_8));
        return p.toString();
    }

    public static void main(String[] args) throws Exception {
        // Prepare test files
        String f1 = writeFile("test_pw_1.txt", "hunter2");
        String f2 = writeFile("test_pw_2.txt", "hunter2");
        String f3 = writeFile("test_pw_3.txt", "");
        String f4 = writeFile("test_pw_4.txt", "pass");
        String f5 = writeFile("test_pw_5.txt", "password");

        // 5 test cases
        System.out.println(comparePassword(f1, "hunter2"));  // true
        System.out.println(comparePassword(f2, "hunter3"));  // false
        System.out.println(comparePassword(f3, ""));         // true
        System.out.println(comparePassword(f4, "password")); // false
        System.out.println(comparePassword(f5, "pass"));     // false
    }
}