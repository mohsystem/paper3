public class Task20 {

    /**
     * Finds a positive integer k, if it exists, such that the sum of the digits of n
     * raised to consecutive powers starting from p is equal to k * n.
     *
     * @param n A positive integer.
     * @param p A positive integer.
     * @return The positive integer k, or -1 if it does not exist.
     */
    public static long digPow(int n, int p) {
        String s = String.valueOf(n);
        long sum = 0;
        for (int i = 0; i < s.length(); i++) {
            int digit = Character.getNumericValue(s.charAt(i));
            sum += (long) Math.pow(digit, p + i);
        }

        if (sum % n == 0) {
            return sum / n;
        } else {
            return -1;
        }
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println(digPow(89, 1));
        // Test Case 2
        System.out.println(digPow(92, 1));
        // Test Case 3
        System.out.println(digPow(695, 2));
        // Test Case 4
        System.out.println(digPow(46288, 3));
        // Test Case 5
        System.out.println(digPow(1, 1));
    }
}