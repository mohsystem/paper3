import java.lang.Math;

public class Task20 {
    public static int digPow(int n, int p) {
        String s = String.valueOf(n);
        long sum = 0;
        for (int i = 0; i < s.length(); i++) {
            int digit = Character.getNumericValue(s.charAt(i));
            sum += Math.pow(digit, p + i);
        }

        if (sum % n == 0) {
            return (int) (sum / n);
        } else {
            return -1;
        }
    }

    public static void main(String[] args) {
        System.out.println("n=89, p=1 -> " + digPow(89, 1));
        System.out.println("n=92, p=1 -> " + digPow(92, 1));
        System.out.println("n=695, p=2 -> " + digPow(695, 2));
        System.out.println("n=46288, p=3 -> " + digPow(46288, 3));
        System.out.println("n=114, p=3 -> " + digPow(114, 3));
    }
}