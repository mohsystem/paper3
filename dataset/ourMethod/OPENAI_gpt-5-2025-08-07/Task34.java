import java.math.BigInteger;
import java.util.Arrays;
import java.util.List;

public class Task34 {
    public static boolean narcissistic(long n) {
        if (n <= 0) {
            return false;
        }
        String s = Long.toString(n);
        int power = s.length();
        BigInteger sum = BigInteger.ZERO;
        for (int i = 0; i < s.length(); i++) {
            int digit = s.charAt(i) - '0';
            BigInteger term = BigInteger.valueOf(digit).pow(power);
            sum = sum.add(term);
        }
        return sum.equals(BigInteger.valueOf(n));
    }

    public static void main(String[] args) {
        List<Long> tests = Arrays.asList(1L, 153L, 370L, 1652L, 9474L);
        for (long t : tests) {
            System.out.println("n=" + t + " -> " + narcissistic(t));
        }
    }
}