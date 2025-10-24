import java.lang.Math;

public class Task34 {

    /**
     * Checks if a number is a Narcissistic Number (Armstrong Number).
     *
     * @param value The positive integer to check.
     * @return true if the number is narcissistic, false otherwise.
     */
    public static boolean narcissistic(int value) {
        // The prompt guarantees positive non-zero integers, but this is a good practice.
        if (value <= 0) {
            return false;
        }

        String sValue = String.valueOf(value);
        int numDigits = sValue.length();
        long sum = 0;

        int temp = value;
        while (temp > 0) {
            int digit = temp % 10;
            // Use Math.pow which returns a double, and accumulate in a long sum
            // to prevent potential integer overflow for large numbers.
            sum += Math.pow(digit, numDigits);
            temp /= 10;
        }

        return sum == value;
    }

    public static void main(String[] args) {
        System.out.println("Is 7 narcissistic? " + narcissistic(7));       // Expected: true
        System.out.println("Is 153 narcissistic? " + narcissistic(153));     // Expected: true
        System.out.println("Is 371 narcissistic? " + narcissistic(371));     // Expected: true
        System.out.println("Is 1634 narcissistic? " + narcissistic(1634));    // Expected: true
        System.out.println("Is 1652 narcissistic? " + narcissistic(1652));    // Expected: false
    }
}