import java.io.FileWriter;
import java.io.IOException;
import java.util.Random;

public class Lse118 {

    /**
     * Generates 3 random floats, concatenates them into a single string,
     * and writes the string to a specified file.
     *
     * @param filename The name of the file to write to.
     */
    public static void generateAndWriteToFile(String filename) {
        Random rand = new Random();

        // 1. Generate 3 random floats
        float f1 = rand.nextFloat();
        float f2 = rand.nextFloat();
        float f3 = rand.nextFloat();

        // 2. Convert them to strings and 3. Concatenate them
        // Using StringBuilder is efficient for string concatenation
        StringBuilder sb = new StringBuilder();
        sb.append(f1);
        sb.append(f2);
        sb.append(f3);
        String concatenatedString = sb.toString();

        // 4. Write the string to a file
        // Using try-with-resources ensures the FileWriter is closed automatically,
        // preventing resource leaks.
        try (FileWriter writer = new FileWriter(filename)) {
            writer.write(concatenatedString);
            System.out.println("Successfully wrote to " + filename);
        } catch (IOException e) {
            System.err.println("An error occurred while writing to file " + filename + ": " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // Run 5 test cases
        System.out.println("--- Java Test Cases ---");
        for (int i = 1; i <= 5; i++) {
            String filename = "java_output_" + i + ".txt";
            System.out.println("Running test case " + i + "...");
            generateAndWriteToFile(filename);
            System.out.println("--------------------");
        }
    }
}