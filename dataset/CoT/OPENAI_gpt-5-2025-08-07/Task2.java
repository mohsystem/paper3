// Chain-of-Through process in code generation:
// 1) Problem understanding: Create a function that takes exactly 10 integers (0-9) and returns "(XXX) XXX-XXXX".
// 2) Security requirements: Validate inputs (null, length, range), avoid unsafe operations.
// 3) Secure coding generation: Implement using safe constructs, clear validation, and immutable return.
// 4) Code review: Ensure no unchecked access, proper error handling, and no resource leaks.
// 5) Secure code output: Final version with validation and tests.

public class Task2 {
    public static String createPhoneNumber(int[] numbers) {
        if (numbers == null) {
            throw new IllegalArgumentException("Input array is null.");
        }
        if (numbers.length != 10) {
            throw new IllegalArgumentException("Input array must contain exactly 10 integers.");
        }
        StringBuilder sb = new StringBuilder(14);
        // Validate and build
        for (int i = 0; i < 10; i++) {
            int n = numbers[i];
            if (n < 0 || n > 9) {
                throw new IllegalArgumentException("All integers must be in the range 0-9.");
            }
        }
        sb.append('(')
          .append(numbers[0]).append(numbers[1]).append(numbers[2])
          .append(')').append(' ')
          .append(numbers[3]).append(numbers[4]).append(numbers[5])
          .append('-')
          .append(numbers[6]).append(numbers[7]).append(numbers[8]).append(numbers[9]);
        return sb.toString();
    }

    public static void main(String[] args) {
        // 5 test cases
        int[] t1 = new int[] {1,2,3,4,5,6,7,8,9,0};
        int[] t2 = new int[] {0,1,2,3,4,5,6,7,8,9};
        int[] t3 = new int[] {0,1,2,3,4,5,6,7,8}; // invalid length
        int[] t4 = new int[] {-1,1,2,3,4,5,6,7,8,9}; // invalid value
        int[] t5 = null; // null input

        try {
            System.out.println("Test 1: " + createPhoneNumber(t1));
        } catch (Exception e) {
            System.out.println("Test 1 Error: " + e.getMessage());
        }
        try {
            System.out.println("Test 2: " + createPhoneNumber(t2));
        } catch (Exception e) {
            System.out.println("Test 2 Error: " + e.getMessage());
        }
        try {
            System.out.println("Test 3: " + createPhoneNumber(t3));
        } catch (Exception e) {
            System.out.println("Test 3 Error: " + e.getMessage());
        }
        try {
            System.out.println("Test 4: " + createPhoneNumber(t4));
        } catch (Exception e) {
            System.out.println("Test 4 Error: " + e.getMessage());
        }
        try {
            System.out.println("Test 5: " + createPhoneNumber(t5));
        } catch (Exception e) {
            System.out.println("Test 5 Error: " + e.getMessage());
        }
    }
}