import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.FileAlreadyExistsException;

public class Lse103 {

    /**
     * Opens or creates a file with the specified options, demonstrating behavior
     * equivalent to POSIX open() flags.
     * O_CREAT:          StandardOpenOption.CREATE
     * O_TRUNC:          StandardOpenOption.TRUNCATE_EXISTING
     * O_WRONLY:         StandardOpenOption.WRITE
     * O_CREAT | O_EXCL: StandardOpenOption.CREATE_NEW
     *
     * In a real-world application, sanitize the 'filePath' parameter to prevent path traversal vulnerabilities.
     *
     * @param filePath The path to the file.
     * @param options The set of options specifying how the file is opened.
     * @return true if successful, false otherwise.
     */
    public static boolean openFileWithOptions(String filePath, StandardOpenOption... options) {
        // In a production environment, filePath should be validated to prevent path traversal.
        Path path = Paths.get(filePath);
        try (OutputStream os = Files.newOutputStream(path, options)) {
            System.out.println("Success: Opened/created '" + filePath + "'");
            // Optionally write some data to the file to show it's writable.
            os.write("Java test content.\n".getBytes());
            return true;
        } catch (FileAlreadyExistsException e) {
            System.err.println("Error: File '" + filePath + "' already exists (CREATE_NEW / O_EXCL effect).");
            return false;
        } catch (IOException e) {
            System.err.println("Error handling file '" + filePath + "': " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        String file1 = "test_java_1.txt";
        String file2 = "test_java_2.txt";
        String file3 = "test_java_3.txt";

        // TC1: O_CREAT | O_WRONLY | O_TRUNC on a new file.
        System.out.println("\n[TC1] Creating a new file for writing (CREATE, WRITE, TRUNCATE_EXISTING)...");
        openFileWithOptions(file1, StandardOpenOption.CREATE, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);

        // TC2: O_CREAT | O_WRONLY | O_TRUNC on an existing file (should truncate).
        System.out.println("\n[TC2] Opening an existing file for writing (should truncate)...");
        openFileWithOptions(file1, StandardOpenOption.CREATE, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);

        // TC3: O_CREAT | O_WRONLY | O_EXCL on a new file.
        System.out.println("\n[TC3] Creating a new file exclusively (CREATE_NEW)...");
        openFileWithOptions(file2, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);

        // TC4: O_CREAT | O_WRONLY | O_EXCL on an existing file (should fail).
        System.out.println("\n[TC4] Attempting to create an existing file exclusively (should fail)...");
        openFileWithOptions(file2, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);

        // TC5: O_WRONLY on an existing file (no CREATE).
        System.out.println("\n[TC5] Opening an existing file with WRITE only...");
        // First, create the file for the test
        openFileWithOptions(file3, StandardOpenOption.CREATE);
        // Now, open it with only WRITE (and TRUNCATE to see an effect)
        openFileWithOptions(file3, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);


        // Cleanup created files
        try {
            System.out.println("\n--- Cleanup ---");
            Files.deleteIfExists(Paths.get(file1));
            Files.deleteIfExists(Paths.get(file2));
            Files.deleteIfExists(Paths.get(file3));
            System.out.println("Cleanup complete.");
        } catch (IOException e) {
            System.err.println("Cleanup failed: " + e.getMessage());
        }
    }
}