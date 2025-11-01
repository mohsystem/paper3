
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;

public class Lse103 {
    /**
     * Opens a file with O_CREAT | O_TRUNC | O_WRONLY | O_EXCL flags
     * Creates file if it doesn't exist, fails if it exists
     */
    public static int openFileExclusive(String filename) {
        try {
            // O_CREAT | O_EXCL | O_WRONLY | O_TRUNC
            FileChannel channel = FileChannel.open(
                Paths.get(filename),
                StandardOpenOption.CREATE_NEW,  // O_CREAT | O_EXCL
                StandardOpenOption.WRITE,       // O_WRONLY
                StandardOpenOption.TRUNCATE_EXISTING  // O_TRUNC (though not needed with CREATE_NEW)
            );
            channel.close();
            return 0; // Success
        } catch (IOException e) {
            return -1; // Failure
        }
    }

    /**
     * Opens a file with O_CREAT | O_TRUNC | O_WRONLY flags
     * Creates or truncates existing file
     */
    public static int openFileTruncate(String filename) {
        try {
            // O_CREAT | O_WRONLY | O_TRUNC
            FileChannel channel = FileChannel.open(
                Paths.get(filename),
                StandardOpenOption.CREATE,              // O_CREAT
                StandardOpenOption.WRITE,               // O_WRONLY
                StandardOpenOption.TRUNCATE_EXISTING    // O_TRUNC
            );
            channel.close();
            return 0; // Success
        } catch (IOException e) {
            return -1; // Failure
        }
    }

    public static void main(String[] args) {
        System.out.println("Test Case 1: Create new file with exclusive flag");
        int result1 = openFileExclusive("test1.txt");
        System.out.println("Result: " + (result1 == 0 ? "SUCCESS" : "FAILED"));

        System.out.println("\\nTest Case 2: Try to create existing file with exclusive flag (should fail)");
        int result2 = openFileExclusive("test1.txt");
        System.out.println("Result: " + (result2 == -1 ? "FAILED AS EXPECTED" : "UNEXPECTED SUCCESS"));

        System.out.println("\\nTest Case 3: Create new file with truncate flag");
        int result3 = openFileTruncate("test2.txt");
        System.out.println("Result: " + (result3 == 0 ? "SUCCESS" : "FAILED"));

        System.out.println("\\nTest Case 4: Truncate existing file");
        int result4 = openFileTruncate("test2.txt");
        System.out.println("Result: " + (result4 == 0 ? "SUCCESS" : "FAILED"));

        System.out.println("\\nTest Case 5: Create file in subdirectory");
        new File("testdir").mkdir();
        int result5 = openFileTruncate("testdir/test3.txt");
        System.out.println("Result: " + (result5 == 0 ? "SUCCESS" : "FAILED"));

        // Cleanup
        try {
            Files.deleteIfExists(Paths.get("test1.txt"));
            Files.deleteIfExists(Paths.get("test2.txt"));
            Files.deleteIfExists(Paths.get("testdir/test3.txt"));
            Files.deleteIfExists(Paths.get("testdir"));
        } catch (IOException e) {
            // Ignore cleanup errors
        }
    }
}
