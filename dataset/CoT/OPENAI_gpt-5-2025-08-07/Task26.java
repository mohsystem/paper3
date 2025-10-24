// Task26 - Java implementation
// 1) Understand: Find the integer that appears an odd number of times (exactly one exists).
// 2-5) Secure coding: handle null/empty inputs safely; use XOR to compute result.
public class Task26 {
    public static int findOddOccurrence(int[] arr) {
        if (arr == null || arr.length == 0) {
            return 0; // Safe default; per problem constraints this case shouldn't occur.
        }
        int x = 0;
        for (int v : arr) {
            x ^= v;
        }
        return x;
    }

    private static void runTest(int[] arr) {
        int result = findOddOccurrence(arr);
        System.out.println(result);
    }

    public static void main(String[] args) {
        // 5 test cases
        runTest(new int[]{7}); // -> 7
        runTest(new int[]{0}); // -> 0
        runTest(new int[]{1, 1, 2}); // -> 2
        runTest(new int[]{0, 1, 0, 1, 0}); // -> 0
        runTest(new int[]{1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1}); // -> 4
    }
}