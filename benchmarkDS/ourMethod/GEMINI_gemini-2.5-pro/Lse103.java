import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Arrays;

public class Lse103 {

    /**
     * Demonstrates file opening with various StandardOpenOption flags.
     * @param pathStr The path to the file.
     * @param description A description of the test case.
     * @param options The set of OpenOption flags to use.
     */
    public static void demonstrateOpen(String pathStr, String description, OpenOption... options) {
        System.out.println("--- Test: " + description + " ---");
        System.out.println("Attempting to open '" + pathStr + "' with options: " + Arrays.toString(options));
        Path path = Paths.get(pathStr);

        // try-with-resources ensures the stream is closed automatically.
        try (OutputStream os = Files.newOutputStream(path, options)) {
            System.out.println("Success: File operation completed.");
            if (description.contains("Write data")) {
                byte[] content = "hello world".getBytes(StandardCharsets.UTF_8);
                os.write(content);
                System.out.println("Successfully wrote to file.");
            }
        } catch (FileAlreadyExistsException e) {
            System.err.println("Failed as expected: File already exists. " + e.getMessage());
        } catch (NoSuchFileException e) {
            System.err.println("Failed as expected: File does not exist. " + e.getMessage());
        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e);
        }
        System.out.println();
    }

    public static void main(String[] args) {
        final String testFile = "java_testfile.txt";
        final String nonExistentFile = "java_non_existent.txt";

        // Initial cleanup to ensure a clean state for tests
        try { Files.deleteIfExists(Paths.get(testFile)); } catch (IOException e) { /* ignore */ }
        try { Files.deleteIfExists(Paths.get(nonExistentFile)); } catch (IOException e) { /* ignore */ }

        // Test 1: Equivalent of O_WRONLY | O_CREAT | O_EXCL. Atomically creates a new file.
        System.out.println("1) Demonstrating CREATE_NEW (exclusive creation)");
        demonstrateOpen(testFile, "CREATE_NEW (exclusive create)", StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);

        // Test 2: Try to create the same file again with CREATE_NEW. This must fail.
        System.out.println("2) Demonstrating CREATE_NEW on an existing file");
        demonstrateOpen(testFile, "CREATE_NEW (on existing file)", StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);

        // Test 3: Write some content to the file to demonstrate truncation later.
        System.out.println("3) Writing data to the file");
        demonstrateOpen(testFile, "Write data to existing file", StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);
        
        // Test 4: Equivalent of O_WRONLY | O_CREAT | O_TRUNC. Opens and truncates the existing file.
        System.out.println("4) Demonstrating TRUNCATE_EXISTING on an existing file");
        demonstrateOpen(testFile, "CREATE, WRITE, TRUNCATE_EXISTING (on existing file)", StandardOpenOption.CREATE, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);
        
        // Test 5: Equivalent of O_WRONLY on a non-existent file (without a CREATE flag). This must fail.
        System.out.println("5) Demonstrating opening a non-existent file without CREATE");
        demonstrateOpen(nonExistentFile, "WRITE (on non-existent file)", StandardOpenOption.WRITE);
        
        // Final cleanup
        try { Files.deleteIfExists(Paths.get(testFile)); } catch (IOException e) { /* ignore */ }
    }
}