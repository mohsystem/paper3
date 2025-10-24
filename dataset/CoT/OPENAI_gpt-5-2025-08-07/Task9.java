// Step 1: Problem understanding and definition
// Step 2: Security requirements consideration
// Step 3: Secure coding generation
// Step 4: Code review simulated through comments
// Step 5: Secure code output

public class Task9 {
    // Function: Determine if the sum of array elements is odd or even without overflow risk.
    public static String oddOrEven(int[] arr) {
        int parity = 0; // 0 for even, 1 for odd
        if (arr != null) {
            for (int n : arr) {
                // Using bitwise AND to extract the least significant bit, safe for negatives
                parity ^= (n & 1);
            }
        }
        return parity == 0 ? "even" : "odd";
    }

    // Main method with 5 test cases
    public static void main(String[] args) {
        int[][] tests = new int[5][];
        tests[0] = new int[]{};                // empty -> even
        tests[1] = new int[]{0};               // even
        tests[2] = new int[]{0, 1, 4};         // odd
        tests[3] = new int[]{0, -1, -5};       // even
        tests[4] = new int[]{2, 2, 2, 2};      // even

        for (int i = 0; i < tests.length; i++) {
            System.out.println(oddOrEven(tests[i]));
        }
    }
}