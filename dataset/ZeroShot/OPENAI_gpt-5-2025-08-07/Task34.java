public class Task34 {
    public static boolean isNarcissistic(long n) {
        if (n <= 0) return false; // As per problem, positive non-zero integers expected
        int digits = countDigits(n);
        java.math.BigInteger sum = java.math.BigInteger.ZERO;
        long temp = n;
        while (temp > 0) {
            int d = (int)(temp % 10);
            java.math.BigInteger term = java.math.BigInteger.valueOf(d).pow(digits);
            sum = sum.add(term);
            temp /= 10;
        }
        return sum.equals(java.math.BigInteger.valueOf(n));
    }

    private static int countDigits(long n) {
        int count = 0;
        long t = n;
        while (t > 0) {
            count++;
            t /= 10;
        }
        return Math.max(count, 1);
    }

    public static void main(String[] args) {
        long[] tests = {153L, 1652L, 9474L, 370L, 9926315L};
        for (long t : tests) {
            System.out.println(isNarcissistic(t));
        }
    }
}