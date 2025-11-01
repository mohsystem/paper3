import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.charset.StandardCharsets;

public class Lse104 {

    /**
     * Opens a file for reading and writing, creating it if it doesn't exist.
     * It then writes the given content to the file. This behavior is similar to
     * using O_RDWR | O_CREAT flags in POSIX systems.
     *
     * @param fileName The name of the file to open/create.
     * @param content  The string content to write to the file.
     * @return true if the operation was successful, false otherwise.
     */
    public static boolean writeConfig(String fileName, String content) {
        // In Java, RandomAccessFile with "rw" mode opens for reading and writing.
        // It creates the file if it does not exist.
        File file = new File(fileName);
        // Using try-with-resources to ensure the file is closed automatically
        try (RandomAccessFile raf = new RandomAccessFile(file, "rw")) {
            // Truncate the file to 0 bytes if it already exists to simulate
            // a fresh write, which is common for configuration files.
            raf.setLength(0);
            raf.write(content.getBytes(StandardCharsets.UTF_8));
            return true;
        } catch (IOException | SecurityException e) {
            System.err.println("Error writing to file '" + fileName + "': " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String content = "important_config";
        String[] testFiles = {
            "config1.txt",
            "test/config2.txt", // Note: This will fail if 'test' directory doesn't exist.
            "config3.txt",
            "config4.txt",
            "config5.txt"
        };
        
        // Create a directory for the second test case to succeed
        new File("test").mkdir();

        for (String fileName : testFiles) {
            System.out.println("Attempting to write to: " + fileName);
            boolean result = writeConfig(fileName, content);
            System.out.println("  -> Success: " + result);
            if (result) {
                 new File(fileName).delete(); // Clean up created file
            }
        }
        // Clean up the created directory
        new File("test").delete();
    }
}