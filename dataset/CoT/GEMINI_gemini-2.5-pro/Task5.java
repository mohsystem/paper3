public class Task5 {

    /**
     * Calculates the multiplicative persistence of a positive number.
     *
     * @param n A positive long integer.
     * @return The number of times digits must be multiplied to reach a single digit.
     */
    public static int persistence(long n) {
        int count = 0;
        while (n >= 10) {
            count++;
            long product = 1;
            long temp = n;
            while (temp > 0) {
                product *= temp % 10;
                temp /= 10;
            }
            n = product;
        }
        return count;
    }

    public static void main(String[] args) {
        // Test cases
        long[] testCases = {39, 999, 4, 25, 679};
        int[] expectedResults = {3, 4, 0, 2, 5};

        for (int i = 0; i < testCases.length; i++) {
            long input = testCases[i];
            int expected = expectedResults[i];
            int result = persistence(input);
            System.out.println("Input: " + input + ", Output: " + result + ", Expected: " + expected);
        }
    }
}