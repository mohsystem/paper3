import java.io.RandomAccessFile;
import java.io.IOException;

public class Lse104 {

    public static boolean createFileAndWrite(String filename, String content) {
        if (filename == null || filename.isEmpty()) {
            System.err.println("Error: Filename cannot be null or empty.");
            return false;
        }
        try (RandomAccessFile raf = new RandomAccessFile(filename, "rw")) {
            // Set the file length to 0 to clear it before writing,
            // mimicking truncation which often happens with O_CREAT on existing files.
            raf.setLength(0); 
            raf.write(content.getBytes());
            return true;
        } catch (IOException e) {
            System.err.println("Error processing file " + filename + ": " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        String content = "important_config";
        
        System.out.println("--- Java Test Cases ---");
        
        // Test Case 1: Standard file creation
        String file1 = "java_test1.cfg";
        System.out.println("\nTest 1: Writing to '" + file1 + "'");
        if (createFileAndWrite(file1, content)) {
            System.out.println("Success.");
        } else {
            System.out.println("Failed.");
        }

        // Test Case 2: Different content
        String file2 = "java_test2.dat";
        System.out.println("\nTest 2: Writing to '" + file2 + "'");
        if (createFileAndWrite(file2, content + " for test 2")) {
            System.out.println("Success.");
        } else {
            System.out.println("Failed.");
        }

        // Test Case 3: Path with non-existent directory (should fail)
        String file3 = "data/java_test3.txt";
        System.out.println("\nTest 3: Writing to '" + file3 + "'");
        if (createFileAndWrite(file3, content)) {
            System.out.println("Success.");
        } else {
            System.out.println("Failed.");
        }
        
        // Test Case 4: Empty filename (should fail)
        String file4 = "";
        System.out.println("\nTest 4: Writing to empty filename");
        if (createFileAndWrite(file4, content)) {
            System.out.println("Success.");
        } else {
            System.out.println("Failed.");
        }

        // Test Case 5: Writing empty content
        String file5 = "java_test5.log";
        System.out.println("\nTest 5: Writing an empty string to '" + file5 + "'");
        if (createFileAndWrite(file5, "")) {
            System.out.println("Success.");
        } else {
            System.out.println("Failed.");
        }
    }
}