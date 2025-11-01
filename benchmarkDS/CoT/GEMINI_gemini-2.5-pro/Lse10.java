import java.util.Scanner;
import java.util.InputMismatchException;

public class Lse10 {

    /**
     * Returns the value at a given index in an array.
     * Performs bounds checking to prevent out-of-bounds access.
     *
     * @param arr   The array of integers.
     * @param size  The size of the array.
     * @param index The index of the element to retrieve.
     * @return The value at the specified index, or -1 if the index is out of bounds.
     */
    public static int getValueFromArray(int[] arr, int size, int index) {
        // Security: Check if the index is within the valid bounds of the array.
        if (index >= 0 && index < size) {
            return arr[index];
        }
        // Return an error code for invalid index.
        return -1;
    }

    public static void main(String[] args) {
        int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int size = id_sequence.length;
        int index = 0;

        System.out.println("Enter an integer index:");
        try (Scanner scanner = new Scanner(System.in)) {
            // Security: Ensure the input is an integer.
            index = scanner.nextInt();
        } catch (InputMismatchException e) {
            System.err.println("Error: Invalid input. Please enter an integer.");
            System.exit(1); // Exit with a general error code for bad input.
        }

        int result = getValueFromArray(id_sequence, size, index);

        if (result == -1) {
            System.err.println("Error: Index out of bounds.");
            System.exit(1); // Exit with a general error code for out-of-bounds access.
        } else {
            // Exit with the value from the array as the exit code.
            // Note: Exit codes are typically 0-255. Larger values may be truncated.
            System.out.println("Value at index " + index + " is " + result + ". Exiting with this value.");
            System.exit(result);
        }
    }
}
/*
Main method test cases:
1. Input: 2
   Expected Output: Program exits with code 30. (Value at index 2 is 30)
2. Input: 0
   Expected Output: Program exits with code 10. (Value at index 0 is 10)
3. Input: 9
   Expected Output: Program exits with code 100. (Value at index 9 is 100)
4. Input: -1
   Expected Output: Prints "Error: Index out of bounds." and exits with code 1.
5. Input: 10
   Expected Output: Prints "Error: Index out of bounds." and exits with code 1.
*/