import java.util.Scanner;
import java.util.InputMismatchException;

public class Lse11 {
    private static final int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    private static final int id_sequence_size = id_sequence.length;

    /**
     * Securely gets a value from an array at a given index.
     * Performs bounds checking to prevent out-of-bounds access.
     * If the index is out of bounds, it prints an error and terminates the program.
     *
     * @param array The array to access.
     * @param size The size of the array.
     * @param index The index of the element to retrieve.
     * @return The value at the specified index.
     */
    public static int getValueFromArray(int[] array, int size, int index) {
        // Securely check if the index is within the valid bounds of the array.
        if (index < 0 || index >= size) {
            System.err.println("Error: Index out of bounds.");
            // Exit with a non-zero status code to indicate an error.
            System.exit(1);
        }
        return array[index];
    }

    /**
     * The main method reads an integer index from standard input, retrieves a value
     * from a predefined array at that index, and returns the value as the exit code.
     */
    public static void main(String[] args) {
        /*
         * Test cases to run manually by providing input:
         * 1. Input: 3  (valid index) 
         *    Expected: Program exits with code 40.
         * 2. Input: 0  (valid first element)
         *    Expected: Program exits with code 10.
         * 3. Input: 9  (valid last element)
         *    Expected: Program exits with code 100.
         * 4. Input: -1 (invalid, negative)
         *    Expected: "Error: Index out of bounds." on stderr, program exits with code 1.
         * 5. Input: 10 (invalid, too large)
         *    Expected: "Error: Index out of bounds." on stderr, program exits with code 1.
         */
        Scanner scanner = new Scanner(System.in);
        int index;

        try {
            index = scanner.nextInt();
        } catch (InputMismatchException e) {
            System.err.println("Error: Invalid input. Please enter an integer.");
            System.exit(1);
            return; // To satisfy compiler, though unreachable
        } finally {
            scanner.close();
        }
        
        int value = getValueFromArray(id_sequence, id_sequence_size, index);
        
        // The returned value is used as the exit code.
        // Note: Exit codes are typically 0-255. Larger values may be truncated.
        System.exit(value);
    }
}