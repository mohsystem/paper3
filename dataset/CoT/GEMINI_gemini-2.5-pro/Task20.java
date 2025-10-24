import java.lang.Math;

public class Task20 {

    /**
     * Finds a positive integer k, if it exists, such that the sum of the digits of n
     * raised to consecutive powers starting from p is equal to k * n.
     *
     * @param n a positive integer.
     * @param p a positive integer.
     * @return a positive integer k, or -1 if no such k exists.
     */
    public static long digPow(int n, int p) {
        String nStr = String.valueOf(n);
        long sum = 0;
        for (int i = 0; i < nStr.length(); i++) {
            // Get the numeric value of the character
            int digit = Character.getNumericValue(nStr.charAt(i));
            // Add the digit raised to the power (p + i) to the sum
            sum += Math.pow(digit, p + i);
        }

        // Check if the sum is a multiple of n
        if (sum % n == 0) {
            return sum / n;
        } else {
            return -1;
        }
    }

    public static void main(String[] args) {
        System.out.println("n = 89, p = 1 --> " + digPow(89, 1));
        System.out.println("n = 92, p = 1 --> " + digPow(92, 1));
        System.out.println("n = 695, p = 2 --> " + digPow(695, 2));
        System.out.println("n = 46288, p = 3 --> " + digPow(46288, 3));
        System.out.println("n = 135, p = 1 --> " + digPow(135, 1));
    }
}