import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Arrays;

public class Lse103 {

    /**
     * Opens or creates a file with the specified options, demonstrating flags.
     * @param filePath The path to the file.
     * @param options The set of open options to use.
     * @return true on success, false on failure.
     */
    public static boolean openFileWithOptions(String filePath, StandardOpenOption... options) {
        Path path = Paths.get(filePath);
        System.out.print("Attempting to open '" + filePath + "' with options: " + Arrays.toString(options) + "\n");

        try (OutputStream out = Files.newOutputStream(path, options)) {
            System.out.println("  -> Success: File opened/created.");
            // Write some data to make the file non-empty
            out.write("hello".getBytes());
            return true;
        } catch (FileAlreadyExistsException e) {
            System.out.println("  -> Error: File already exists, and CREATE_NEW was specified.");
            return false;
        } catch (IOException e) {
            System.out.println("  -> Error: An I/O error occurred: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String filename = "java_test_file.txt";
        Path path = Paths.get(filename);

        // Ensure the file does not exist before starting
        try {
            Files.deleteIfExists(path);
        } catch (IOException e) {
            System.err.println("Initial cleanup failed: " + e.getMessage());
        }

        System.out.println("--- Java File Open Flags Demonstration ---");

        // Test 1: O_CREAT | O_WRONLY | O_EXCL
        // Java equivalent is StandardOpenOption.CREATE_NEW
        System.out.println("\n[Test 1: Exclusive Create on new file - Should Succeed]");
        openFileWithOptions(filename, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);

        // Test 2: O_CREAT | O_WRONLY | O_EXCL
        // Trying again on the now-existing file
        System.out.println("\n[Test 2: Exclusive Create on existing file - Should Fail]");
        openFileWithOptions(filename, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);

        // Test 3: O_WRONLY
        // Java's WRITE option opens for writing. Used with CREATE, it acts like O_CREAT.
        System.out.println("\n[Test 3: Open for Writing (O_WRONLY) - Should Succeed]");
        openFileWithOptions(filename, StandardOpenOption.WRITE);

        // Test 4: O_WRONLY | O_TRUNC
        // Adding TRUNCATE_EXISTING to clear the file on open.
        System.out.println("\n[Test 4: Open for Writing with Truncate (O_TRUNC) - Should Succeed]");
        openFileWithOptions(filename, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);

        // Test 5: Cleanup
        System.out.println("\n[Test 5: Cleanup]");
        try {
            Files.delete(path);
            System.out.println("  -> Success: Deleted '" + filename + "'");
        } catch (IOException e) {
            System.out.println("  -> Error: Failed to delete '" + filename + "': " + e.getMessage());
        }
    }
}