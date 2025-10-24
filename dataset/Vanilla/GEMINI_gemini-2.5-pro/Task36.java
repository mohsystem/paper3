import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.File;

public class Task36 {

    /**
     * Reads a file specified by the file path and prints its contents to the console.
     *
     * @param filePath The path to the file to be read.
     */
    public static void readFileAndPrint(String filePath) {
        System.out.println("Reading file: " + filePath);
        File file = new File(filePath);
        if (!file.exists()) {
            System.err.println("Error: File not found at '" + filePath + "'");
            return;
        }

        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
            String line;
            boolean empty = true;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
                empty = false;
            }
            if(empty){
                 System.out.println("(File is empty)");
            }
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
    }

    /**
     * Helper function to create a test file with specified content.
     */
    private static void createTestFile(String fileName, String content) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(fileName))) {
            writer.write(content);
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + fileName);
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        // --- Test Case 1: Read a simple, single-line file ---
        System.out.println("--- Test Case 1: Simple File ---");
        String file1 = "java_test1.txt";
        createTestFile(file1, "Hello, World from Java!");
        readFileAndPrint(file1);
        System.out.println("-------------------------------------\n");

        // --- Test Case 2: Read a multi-line file ---
        System.out.println("--- Test Case 2: Multi-line File ---");
        String file2 = "java_test2.txt";
        createTestFile(file2, "This is line one.\nThis is line two.\nAnd this is the third line.");
        readFileAndPrint(file2);
        System.out.println("-------------------------------------\n");

        // --- Test Case 3: Read an empty file ---
        System.out.println("--- Test Case 3: Empty File ---");
        String file3 = "java_test3_empty.txt";
        createTestFile(file3, "");
        readFileAndPrint(file3);
        System.out.println("-------------------------------------\n");

        // --- Test Case 4: Attempt to read a non-existent file ---
        System.out.println("--- Test Case 4: Non-existent File ---");
        readFileAndPrint("nonexistentfile.txt");
        System.out.println("-------------------------------------\n");

        // --- Test Case 5: Read a file with special characters ---
        System.out.println("--- Test Case 5: Special Characters File ---");
        String file4 = "java_test4_special.txt";
        createTestFile(file4, "Special chars: ñ, é, ö, ü, ç, α, β, γ. 😊🎉");
        readFileAndPrint(file4);
        System.out.println("-------------------------------------\n");

        // Cleanup test files
        new File(file1).delete();
        new File(file2).delete();
        new File(file3).delete();
        new File(file4).delete();
    }
}