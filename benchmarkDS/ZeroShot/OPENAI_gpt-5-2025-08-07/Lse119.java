import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Locale;

public class Lse119 {
    public static String[] generateRandomFloatStrings(int precision) {
        // Clamp precision to a safe range
        if (precision < 0) precision = 0;
        if (precision > 9) precision = 9;

        SecureRandom rng = new SecureRandom();

        float f1 = rng.nextFloat();
        float f2 = rng.nextFloat();
        float f3 = rng.nextFloat();

        String fmt = "%." + precision + "f";
        String string = String.format(Locale.ROOT, fmt, f1);
        String string2 = String.format(Locale.ROOT, fmt, f2);
        String string3 = String.format(Locale.ROOT, fmt, f3);

        return new String[] { string, string2, string3 };
    }

    public static void main(String[] args) {
        int[] tests = {2, 3, 0, 5, 7};
        for (int t : tests) {
            String[] out = generateRandomFloatStrings(t);
            System.out.println("Precision " + t + " -> " + Arrays.toString(out));
        }
    }
}