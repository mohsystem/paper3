import java.io.*;
import java.nio.file.*;
import java.util.Comparator;

public class Task127 {

    /**
     * Copies a source file to a destination file, handling I/O exceptions.
     * @param sourcePath The path to the source file.
     * @param destPath The path to the destination file.
     */
    public static void copyFile(String sourcePath, String destPath) {
        // Use try-with-resources to ensure file streams are closed automatically.
        try (BufferedReader reader = new BufferedReader(new FileReader(sourcePath));
             BufferedWriter writer = new BufferedWriter(new FileWriter(destPath))) {

            String line;
            while ((line = reader.readLine()) != null) {
                writer.write(line);
                writer.newLine();
            }
            System.out.println("Success: File copied from " + sourcePath + " to " + destPath);

        } catch (FileNotFoundException e) {
            System.err.println("Error: The file '" + e.getMessage() + "' was not found.");
        } catch (IOException e) {
            // Catches other I/O errors like permission denied, disk full, etc.
            System.err.println("Error: An I/O error occurred. " + e.getMessage());
        } catch (Exception e) {
            // A general catch-all for any other unexpected errors.
            System.err.println("Error: An unexpected error occurred: " + e.getMessage());
        }
    }

    public static void main(String[] args) throws IOException {
        String testDir = "java_test_files";
        Path testDirPath = Paths.get(testDir);
        String validInputFile = testDir + File.separator + "input.txt";
        String validOutputFile = testDir + File.separator + "output.txt";
        String readOnlyFile = testDir + File.separator + "readonly.txt";

        // Setup test environment
        if (Files.exists(testDirPath)) { // Clean up previous runs
             try (var stream = Files.walk(testDirPath)) {
                stream.sorted(Comparator.reverseOrder()).map(Path::toFile).forEach(File::delete);
            }
        }
        Files.createDirectories(testDirPath);
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(validInputFile))) {
            writer.write("This is a test file for Java.\n");
            writer.write("It has multiple lines.\n");
        }

        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Successful file copy
        System.out.println("\n[Test Case 1: Successful Copy]");
        copyFile(validInputFile, validOutputFile);

        // Test Case 2: Input file not found
        System.out.println("\n[Test Case 2: Input File Not Found]");
        copyFile(testDir + "/non_existent.txt", validOutputFile);

        // Test Case 3: Read error (trying to read a directory)
        System.out.println("\n[Test Case 3: Read Error (Reading a directory)]");
        copyFile(testDir, validOutputFile);

        // Test Case 4: Write error (invalid destination path)
        System.out.println("\n[Test Case 4: Write Error (Invalid Path)]");
        copyFile(validInputFile, testDir + "/invalid_dir/output.txt");

        // Test Case 5: Write error (permission denied)
        System.out.println("\n[Test Case 5: Write Error (Permission Denied)]");
        Path readOnlyPath = Paths.get(readOnlyFile);
        Files.createFile(readOnlyPath);
        readOnlyPath.toFile().setReadOnly();
        copyFile(validInputFile, readOnlyFile);
        
        // Cleanup
        readOnlyPath.toFile().setWritable(true);
        try (var stream = Files.walk(testDirPath)) {
             stream.sorted(Comparator.reverseOrder()).map(Path::toFile).forEach(File::delete);
        }
        System.out.println("\n--- Java Tests Complete ---");
    }
}