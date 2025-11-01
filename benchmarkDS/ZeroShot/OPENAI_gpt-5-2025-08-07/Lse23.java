import java.security.SecureRandom;

public class Lse23 {
    public static int[] generateThreeAndSum(SecureRandom rng, int minExclusive, int maxInclusive) {
        if (rng == null) {
            throw new IllegalArgumentException("rng must not be null");
        }
        long baseL = Math.max((long) minExclusive + 1L, 1001L);
        if ((long) maxInclusive < baseL) {
            throw new IllegalArgumentException("maxInclusive must be >= max(minExclusive+1, 1001)");
        }
        int base = (int) baseL;
        long rangeL = (long) maxInclusive - (long) base + 1L;
        if (rangeL <= 0L || rangeL > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("Invalid range");
        }
        int bound = (int) rangeL;

        int a = rng.nextInt(bound) + base;
        int b = rng.nextInt(bound) + base;
        int c = rng.nextInt(bound) + base;

        long sumL = (long) a + (long) b + (long) c;
        if (sumL > Integer.MAX_VALUE || sumL < Integer.MIN_VALUE) {
            throw new IllegalStateException("Sum overflow");
        }

        return new int[]{a, b, c, (int) sumL};
    }

    public static void main(String[] args) {
        SecureRandom rng;
        try {
            rng = SecureRandom.getInstanceStrong();
        } catch (Exception ex) {
            rng = new SecureRandom();
        }
        for (int i = 0; i < 5; i++) {
            int[] r = generateThreeAndSum(rng, 1000, 1_000_000);
            System.out.println("a : " + r[0] + " b : " + r[1] + " c : " + r[2] + " sum: " + r[3] + " sum2: " + r[3]);
        }
    }
}