import java.util.Scanner;
import java.util.InputMismatchException;

public class Lse10 {

    /**
     * Returns the value from an array at a given index.
     *
     * @param arr The array of integers.
     * @param size The size of the array.
     * @param index The index of the element to retrieve.
     * @return The value at the specified index, or -1 if the index is out of bounds.
     */
    public static int getValueFromArray(int[] arr, int size, int index) {
        // Ensure that buffer operations strictly adhere to the buffer's boundaries.
        if (arr != null && index >= 0 && index < size) {
            return arr[index];
        }
        // Return -1 to indicate an out-of-bounds error.
        return -1;
    }

    public static void main(String[] args) {
        /*
        Test cases:
        Let id_sequence = {10, 20, 30, 40, 50}

        To test, compile and run the program, then enter the input. The program's
        exit code will be the result.
        On Linux/macOS: java Lse10; echo $?
        On Windows (cmd): java Lse10 & echo %errorlevel%

        1. Input: 0
           Expected exit code: 10 (value at index 0)
        2. Input: 4
           Expected exit code: 50 (value at index 4)
        3. Input: 5 (out of bounds)
           Expected exit code: 255 (In Java, System.exit(-1) is mapped to 255 on POSIX)
        4. Input: -1 (out of bounds)
           Expected exit code: 255
        5. Input: "abc" (non-integer)
           Expected exit code: 1 (input validation failure)
        */
        final int[] id_sequence = {10, 20, 30, 40, 50};
        final int size = id_sequence.length;
        int index;

        try (Scanner scanner = new Scanner(System.in)) {
            // Ensure all input is validated.
            if (scanner.hasNextInt()) {
                index = scanner.nextInt();
            } else {
                System.err.println("Error: Invalid input. Please enter an integer.");
                System.exit(1);
                return; // Unreachable, but good practice.
            }
        } catch (Exception e) {
            System.err.println("An error occurred while reading input.");
            System.exit(1);
            return; // Unreachable
        }
        
        int value = getValueFromArray(id_sequence, size, index);
        
        System.exit(value);
    }
}