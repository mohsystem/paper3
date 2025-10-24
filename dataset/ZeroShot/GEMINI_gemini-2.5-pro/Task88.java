import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.SecureRandom;

public class Task88 {
    private static final String CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    private static final SecureRandom RANDOM = new SecureRandom();

    /**
     * Generates a cryptographically secure random alphanumeric string of a given length.
     * @param length The length of the string to generate.
     * @return The randomly generated string, including a multi-byte Unicode character.
     */
    public static String generateRandomString(int length) {
        if (length <= 0) {
            throw new IllegalArgumentException("String length must be positive.");
        }
        StringBuilder sb = new StringBuilder(length + 2);
        for (int i = 0; i < length; i++) {
            sb.append(CHARACTERS.charAt(RANDOM.nextInt(CHARACTERS.length())));
        }
        // Add a unicode character to ensure it's not just ASCII
        sb.append("-€");
        return sb.toString();
    }

    /**
     * Creates a secure temporary file, writes a random Unicode string to it using UTF-8,
     * and returns the absolute path to the file.
     * @return The absolute path of the created temporary file.
     * @throws IOException If an I/O error occurs.
     */
    public static String createTempFileWithRandomUnicode() throws IOException {
        // 1. Create a temporary file securely. This avoids race conditions.
        Path tempFile = Files.createTempFile("tempfile_", ".txt");
        
        // For this example, schedule the file for deletion on JVM exit.
        // In a real application, the caller would be responsible for deletion.
        tempFile.toFile().deleteOnExit();

        // 2. Create a random string.
        String randomString = generateRandomString(32);
        
        // 3. Write the string to the file using UTF-8 encoding.
        // A Java String is a sequence of Unicode characters. Writing it with
        // UTF-8 encoding is the standard way to persist it.
        try (Writer writer = new OutputStreamWriter(new FileOutputStream(tempFile.toFile()), StandardCharsets.UTF_8)) {
            writer.write(randomString);
        }

        // 4. Return the path of the temporary file.
        return tempFile.toAbsolutePath().toString();
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases for Java...");
        for (int i = 0; i < 5; i++) {
            try {
                String filePath = createTempFileWithRandomUnicode();
                System.out.println("Test Case " + (i + 1) + ": Success. File created at: " + filePath);
                // Optional: Verify content
                String content = new String(Files.readAllBytes(Paths.get(filePath)), StandardCharsets.UTF_8);
                if (content.endsWith("-€")) {
                     System.out.println("   Content verification successful.");
                } else {
                     System.out.println("   Content verification failed.");
                }
            } catch (IOException e) {
                System.err.println("Test Case " + (i + 1) + ": Failed to create temporary file.");
                e.printStackTrace();
            }
        }
    }
}