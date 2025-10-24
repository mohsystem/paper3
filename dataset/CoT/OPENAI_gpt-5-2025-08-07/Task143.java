import java.util.Arrays;

public class Task143 {

    /*
     Chain-of-Through process:
     1. Problem understanding:
        - Functionality: Sort an array of integers in ascending order.
        - Input: int[] array.
        - Output: new int[] sorted ascending, without mutating the input.
     2. Security requirements:
        - Avoid null pointer dereference, do not mutate caller data, handle large arrays safely by using standard library which is optimized and tested.
     3. Secure coding generation:
        - Use Arrays.copyOf to avoid mutating the input, and Arrays.sort for correctness and performance.
        - Null-safe handling by returning an empty array for null input.
     4. Code review:
        - No untrusted I/O, no reflection, no external dependencies, constant-time not required for sorting, no integer overflows in standard library calls.
     5. Secure code output:
        - Final code below implements these considerations.
    */

    // Sorts the provided array in ascending order and returns a new sorted array (input remains unmodified).
    public static int[] sortArray(int[] arr) {
        if (arr == null) {
            return new int[0];
        }
        int[] copy = Arrays.copyOf(arr, arr.length);
        Arrays.sort(copy);
        return copy;
    }

    private static void printArray(int[] a) {
        System.out.println(Arrays.toString(a));
    }

    public static void main(String[] args) {
        // 5 test cases
        int[][] tests = new int[][]{
            {},                               // empty
            {1, 2, 3, 4, 5},                   // already sorted
            {5, 4, 3, 2, 1},                   // reverse
            {3, 1, 2, 1, 3, 2, 2, 1},          // duplicates
            {-10, 0, 500000, -200, 42, 42}     // negatives and large values
        };

        for (int[] test : tests) {
            int[] sorted = sortArray(test);
            printArray(sorted);
        }
    }
}