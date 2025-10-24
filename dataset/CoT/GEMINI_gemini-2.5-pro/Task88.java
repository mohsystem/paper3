import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.SecureRandom;

public class Task88 {

    private static final String ALPHANUMERIC_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    private static final int STRING_LENGTH = 20;

    /**
     * Creates a secure temporary file, generates a random string, converts it to
     * Unicode escape sequences, writes them to the file using UTF-8, and returns
     * the file's absolute path.
     *
     * @return The absolute path to the newly created temporary file.
     * @throws IOException if an I/O error occurs.
     */
    public static String createTempFileWithUnicodeString() throws IOException {
        Path tempFile = null;
        try {
            // 1. Create a temporary file securely.
            // Files.createTempFile is secure against race conditions.
            tempFile = Files.createTempFile("tempfile_", ".txt");

            // 2. Create a random string using a cryptographically strong random number generator.
            StringBuilder randomString = new StringBuilder(STRING_LENGTH);
            for (int i = 0; i < STRING_LENGTH; i++) {
                int randomIndex = SECURE_RANDOM.nextInt(ALPHANUMERIC_CHARS.length());
                randomString.append(ALPHANUMERIC_CHARS.charAt(randomIndex));
            }

            // 3. Convert the string into its Unicode escape sequence representation.
            StringBuilder unicodeEscapedString = new StringBuilder();
            for (char c : randomString.toString().toCharArray()) {
                // Format each character as a \uXXXX Unicode escape sequence
                unicodeEscapedString.append(String.format("\\u%04x", (int) c));
            }

            // 4. Write the Unicode string to the file using UTF-8 encoding.
            // Using try-with-resources to ensure the writer is closed automatically.
            try (BufferedWriter writer = Files.newBufferedWriter(tempFile, StandardCharsets.UTF_8)) {
                writer.write(unicodeEscapedString.toString());
            }

            // 5. Return the path of the temporary file.
            return tempFile.toAbsolutePath().toString();
        } catch (IOException e) {
            // In case of an error, attempt to clean up the file if it was created.
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException cleanupEx) {
                    e.addSuppressed(cleanupEx);
                }
            }
            throw e; // Re-throw the original exception
        }
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases for Java...");
        for (int i = 1; i <= 5; i++) {
            String filePath = null;
            try {
                filePath = createTempFileWithUnicodeString();
                System.out.printf("Test case %d: Success.\n", i);
                System.out.printf("  - File created at: %s\n", filePath);

                // For verification, read the content back
                String content = new String(Files.readAllBytes(Paths.get(filePath)), StandardCharsets.UTF_8);
                System.out.printf("  - File content: %s\n", content);

            } catch (IOException e) {
                System.err.printf("Test case %d: Failed to create temp file. Error: %s\n", i, e.getMessage());
            } finally {
                // Clean up the created file
                if (filePath != null) {
                    try {
                        Files.deleteIfExists(Paths.get(filePath));
                        System.out.printf("  - Cleaned up file: %s\n", filePath);
                    } catch (IOException e) {
                        System.err.printf("  - Failed to clean up file %s. Error: %s\n", filePath, e.getMessage());
                    }
                }
            }
            System.out.println("---------------------------------");
        }
    }
}