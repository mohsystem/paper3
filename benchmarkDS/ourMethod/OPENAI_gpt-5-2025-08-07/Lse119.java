import java.security.SecureRandom;
import java.util.Locale;

public class Lse119 {

    // Generates three random floats in [0,1) and returns their string representations with given precision.
    public static String[] generateRandomFloatStrings(int precision) {
        if (precision < 0 || precision > 9) {
            throw new IllegalArgumentException("Invalid precision");
        }
        SecureRandom rng = RNGHolder.RAND; // Thread-safe singleton
        double v1 = rng.nextDouble();
        double v2 = rng.nextDouble();
        double v3 = rng.nextDouble();

        String fmt = "%." + precision + "f";
        String s1 = String.format(Locale.ROOT, fmt, v1);
        String s2 = String.format(Locale.ROOT, fmt, v2);
        String s3 = String.format(Locale.ROOT, fmt, v3);
        return new String[]{s1, s2, s3};
    }

    // Holder for a single SecureRandom instance
    private static final class RNGHolder {
        private static final SecureRandom RAND = new SecureRandom();
        private RNGHolder() {}
    }

    public static void main(String[] args) {
        int[] testPrecisions = new int[]{0, 2, 4, 6, 9};
        for (int p : testPrecisions) {
            String[] out = generateRandomFloatStrings(p);
            System.out.println("Precision " + p + " -> string: " + out[0] + ", string2: " + out[1] + ", string3: " + out[2]);
        }
    }
}