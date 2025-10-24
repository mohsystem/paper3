public class Task10 {
    /**
     * Calculates the sum of all integers between and including two given integers.
     * Uses a mathematical formula for an arithmetic series to efficiently compute the sum
     * and uses 'long' to prevent potential integer overflow for large ranges.
     *
     * @param a The first integer.
     * @param b The second integer.
     * @return The sum of all integers between a and b, inclusive.
     */
    public static long getSum(int a, int b) {
        if (a == b) {
            return a;
        }
        
        // Use long for calculations to prevent overflow.
        long longA = a;
        long longB = b;
        
        long min = Math.min(longA, longB);
        long max = Math.max(longA, longB);
        
        // Formula for the sum of an arithmetic series: n * (first + last) / 2
        // where n is the number of terms (max - min + 1).
        return (max - min + 1) * (min + max) / 2;
    }

    public static void main(String[] args) {
        // Test Cases
        System.out.println(getSum(1, 0));    // Expected output: 1
        System.out.println(getSum(1, 2));    // Expected output: 3
        System.out.println(getSum(0, 1));    // Expected output: 1
        System.out.println(getSum(1, 1));    // Expected output: 1
        System.out.println(getSum(-1, 2));   // Expected output: 2
    }
}