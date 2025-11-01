import java.io.FileWriter;
import java.io.IOException;
import java.io.BufferedWriter;
import java.util.Random;

public class Lse118 {

    /**
     * Generates 3 random floats, concatenates them into a string,
     * and appends the string to the specified file.
     * @param filename The name of the file to write to.
     */
    public static void generateAndWriteRandomFloats(String filename) {
        Random rand = new Random();

        // 1. Generate 3 random floats
        float f1 = rand.nextFloat();
        float f2 = rand.nextFloat();
        float f3 = rand.nextFloat();

        // 2. Convert them to strings and 3. Concatenate them
        // A StringBuilder is more efficient for concatenation in a loop, but for 3, '+' is fine.
        String concatenatedString = String.valueOf(f1) + String.valueOf(f2) + String.valueOf(f3) + "\n";

        // 4. Writes the string to a file (in append mode)
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(filename, true))) {
            writer.write(concatenatedString);
        } catch (IOException e) {
            System.err.println("An error occurred while writing to the file: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        String filename = "output.java.txt";
        
        // Clear the file at the beginning of the run for consistent output
        try (FileWriter writer = new FileWriter(filename, false)) {
            // Opening in non-append mode and closing it clears the file.
        } catch (IOException e) {
            System.err.println("An error occurred while clearing the file: " + e.getMessage());
        }

        // Run 5 test cases
        System.out.println("Running 5 Java test cases. Writing to " + filename);
        for (int i = 0; i < 5; i++) {
            generateAndWriteRandomFloats(filename);
        }
        System.out.println("Java tests finished.");
    }
}