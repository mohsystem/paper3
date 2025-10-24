import java.lang.Math;

public class Task20 {

    /**
     * Finds a positive integer k, if it exists, such that the sum of the digits of n
     * raised to consecutive powers starting from p is equal to k * n.
     *
     * @param n A positive integer.
     * @param p A positive integer.
     * @return The integer k if found, otherwise -1.
     */
    public static long digPow(int n, int p) {
        String nStr = Integer.toString(n);
        long sum = 0;
        for (int i = 0; i < nStr.length(); i++) {
            int digit = Character.getNumericValue(nStr.charAt(i));
            sum += (long) Math.pow(digit, p + i);
        }

        if (sum % n == 0) {
            return sum / n;
        } else {
            return -1;
        }
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("n=89, p=1 -> " + digPow(89, 1));
        System.out.println("n=92, p=1 -> " + digPow(92, 1));
        System.out.println("n=695, p=2 -> " + digPow(695, 2));
        System.out.println("n=46288, p=3 -> " + digPow(46288, 3));
        System.out.println("n=135, p=1 -> " + digPow(135, 1));
    }
}