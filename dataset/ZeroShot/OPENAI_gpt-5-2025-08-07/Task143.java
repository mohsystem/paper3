import java.util.Arrays;

public class Task143 {
    public static int[] sortAscending(int[] input) {
        if (input == null) {
            return new int[0];
        }
        int[] copy = Arrays.copyOf(input, input.length);
        Arrays.sort(copy);
        return copy;
    }

    private static void printArray(int[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        // Test case 1
        int[] t1 = {5, 2, 9, 1, 5, 6};
        printArray(sortAscending(t1));

        // Test case 2
        int[] t2 = {-3, 0, -1, -7, 2};
        printArray(sortAscending(t2));

        // Test case 3
        int[] t3 = {};
        printArray(sortAscending(t3));

        // Test case 4
        int[] t4 = {42};
        printArray(sortAscending(t4));

        // Test case 5
        int[] t5 = {Integer.MAX_VALUE, Integer.MIN_VALUE, 0, -1, 1};
        printArray(sortAscending(t5));
    }
}