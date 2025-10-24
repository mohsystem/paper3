public class Task34 {

    /**
     * Helper function to calculate integer power.
     * This avoids floating-point inaccuracies of Math.pow().
     */
    private static long power(int base, int exp) {
        long result = 1;
        for (int i = 0; i < exp; i++) {
            result *= base;
        }
        return result;
    }

    /**
     * Checks if a number is a Narcissistic Number.
     * A Narcissistic Number is a positive number which is the sum of its own digits,
     * each raised to the power of the number of digits.
     * @param value The positive integer to check.
     * @return true if the number is narcissistic, false otherwise.
     */
    public static boolean isNarcissistic(int value) {
        if (value <= 0) {
            return false;
        }

        // Count the number of digits
        int temp = value;
        int numDigits = 0;
        while (temp > 0) {
            temp /= 10;
            numDigits++;
        }

        // Calculate the sum of powers of digits
        long sum = 0;
        temp = value;
        while (temp > 0) {
            int digit = temp % 10;
            sum += power(digit, numDigits);
            // Early exit if sum exceeds value, as it can only increase
            if (sum > value) {
                return false;
            }
            temp /= 10;
        }

        return sum == value;
    }

    public static void main(String[] args) {
        int[] testCases = {153, 1652, 7, 371, 9474};
        for (int testCase : testCases) {
            System.out.println("isNarcissistic(" + testCase + ") -> " + isNarcissistic(testCase));
        }
    }
}