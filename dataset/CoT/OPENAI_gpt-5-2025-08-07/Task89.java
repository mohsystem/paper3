import java.io.BufferedWriter;
import java.io.BufferedReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

public class Task89 {

    public static boolean comparePasswordFromFile(String filePath, String userInput) {
        char[] inputChars = null;
        char[] storedChars = null;
        try (BufferedReader reader = Files.newBufferedReader(Path.of(filePath), StandardCharsets.UTF_8)) {
            String storedLine = reader.readLine();
            if (storedLine == null) {
                storedLine = "";
            }
            storedChars = storedLine.toCharArray();
            inputChars = userInput != null ? userInput.toCharArray() : new char[0];
            return constantTimeEquals(storedChars, inputChars);
        } catch (IOException e) {
            return false;
        } finally {
            if (storedChars != null) {
                java.util.Arrays.fill(storedChars, '\0');
            }
            if (inputChars != null) {
                java.util.Arrays.fill(inputChars, '\0');
            }
        }
    }

    private static boolean constantTimeEquals(char[] a, char[] b) {
        int alen = a != null ? a.length : 0;
        int blen = b != null ? b.length : 0;
        int max = Math.max(alen, blen);
        int diff = alen ^ blen;
        for (int i = 0; i < max; i++) {
            char ac = i < alen ? a[i] : 0;
            char bc = i < blen ? b[i] : 0;
            diff |= (ac ^ bc);
        }
        return diff == 0;
    }

    private static Path createTempPasswordFile(String content) throws IOException {
        Path temp = Files.createTempFile("passwd_", ".txt");
        try (BufferedWriter writer = Files.newBufferedWriter(temp, StandardCharsets.UTF_8)) {
            writer.write(content);
        }
        return temp;
    }

    public static void main(String[] args) throws Exception {
        // Test case 1: exact match
        Path f1 = createTempPasswordFile("Secret123!");
        System.out.println(comparePasswordFromFile(f1.toString(), "Secret123!"));

        // Test case 2: mismatch
        Path f2 = createTempPasswordFile("Secret123!");
        System.out.println(comparePasswordFromFile(f2.toString(), "secret123!"));

        // Test case 3: different length
        Path f3 = createTempPasswordFile("Short");
        System.out.println(comparePasswordFromFile(f3.toString(), "Shorter"));

        // Test case 4: file has trailing newline, still matches
        Path f4 = createTempPasswordFile("WithNewline\n");
        System.out.println(comparePasswordFromFile(f4.toString(), "WithNewline"));

        // Test case 5: file not found
        System.out.println(comparePasswordFromFile("nonexistent_file_abcdefg.txt", "anything"));

        // Cleanup
        try { Files.deleteIfExists(f1); } catch (Exception ignored) {}
        try { Files.deleteIfExists(f2); } catch (Exception ignored) {}
        try { Files.deleteIfExists(f3); } catch (Exception ignored) {}
        try { Files.deleteIfExists(f4); } catch (Exception ignored) {}
    }
}