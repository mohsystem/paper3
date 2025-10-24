import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.SecureRandom;

public class Task88 {

    private static final String CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    private static final int STRING_LENGTH = 32;
    private static final SecureRandom RANDOM = new SecureRandom();

    /**
     * Creates a temporary file using UTF-8 encoding, writes a randomly generated
     * string's Unicode representation to it, and returns the file path.
     *
     * @return The absolute path of the created temporary file.
     * @throws IOException if an I/O error occurs.
     */
    public static String createTempFileWithUnicodeString() throws IOException {
        Path tempFile = null;
        try {
            // 1. Create a temporary file securely.
            // Files.createTempFile creates a file with secure default permissions.
            tempFile = Files.createTempFile("task88_", ".txt");

            // 2. Create a random string using a cryptographically secure random number generator.
            StringBuilder randomString = new StringBuilder(STRING_LENGTH);
            for (int i = 0; i < STRING_LENGTH; i++) {
                int randomIndex = RANDOM.nextInt(CHARACTERS.length());
                randomString.append(CHARACTERS.charAt(randomIndex));
            }

            // 3. Convert the string into a Unicode representation and write to the file using UTF-8.
            // try-with-resources ensures the writer is closed automatically.
            try (BufferedWriter writer = Files.newBufferedWriter(tempFile, StandardCharsets.UTF_8)) {
                StringBuilder unicodeRepresentation = new StringBuilder();
                randomString.toString().codePoints().forEach(codePoint -> 
                    unicodeRepresentation.append(String.format("U+%04X ", codePoint))
                );
                // trim() to remove the trailing space
                writer.write(unicodeRepresentation.toString().trim());
            }

            // 4. Return the path of the temporary file.
            return tempFile.toAbsolutePath().toString();
        } catch (IOException e) {
            // Clean up the file if it was created before an error occurred during writing.
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
        for (int i = 0; i < 5; i++) {
            String filePath = null;
            try {
                filePath = createTempFileWithUnicodeString();
                System.out.println("Test Case " + (i + 1) + ": Created temp file at: " + filePath);
                // Optional: Read and print content for verification
                String content = new String(Files.readAllBytes(Path.of(filePath)), StandardCharsets.UTF_8);
                System.out.println("  File content: " + content);
            } catch (IOException e) {
                System.err.println("Test Case " + (i + 1) + ": An error occurred: " + e.getMessage());
                e.printStackTrace();
            } finally {
                // Clean up the created file
                if (filePath != null) {
                    try {
                        Files.deleteIfExists(Path.of(filePath));
                        System.out.println("  Cleaned up temp file: " + filePath);
                    } catch (IOException e) {
                        System.err.println("  Failed to clean up temp file: " + filePath);
                    }
                }
            }
        }
    }
}