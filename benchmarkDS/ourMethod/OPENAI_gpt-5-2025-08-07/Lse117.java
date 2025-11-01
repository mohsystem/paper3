import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Locale;

public class Lse117 {

    private static final SecureRandom RNG = new SecureRandom();

    // Converts a float to a string with fixed precision using a safe, locale-stable format.
    public static String a(float f) {
        return String.format(Locale.ROOT, "%.6f", f);
    }

    // Generates n random floats in [0,1) and returns their string representations.
    public static String[] generateNRandomFloatStrings(int n) {
        if (n <= 0 || n > 1000) {
            throw new IllegalArgumentException("invalid input");
        }
        String[] out = new String[n];
        for (int i = 0; i < n; i++) {
            float v = RNG.nextFloat(); // [0.0, 1.0)
            out[i] = a(v);
        }
        return out;
    }

    public static void main(String[] args) {
        // Core requirement demonstration: generate three random floats, convert using a(), store in str_a, str_b, str_c
        String[] trio = generateNRandomFloatStrings(3);
        String str_a = trio[0];
        String str_b = trio[1];
        String str_c = trio[2];
        System.out.println("Test 1: " + Arrays.toString(new String[]{str_a, str_b, str_c}));

        // Additional 4 test cases
        String[] t2 = generateNRandomFloatStrings(3);
        System.out.println("Test 2: " + Arrays.toString(t2));

        String[] t3 = generateNRandomFloatStrings(3);
        System.out.println("Test 3: " + Arrays.toString(t3));

        String[] t4 = generateNRandomFloatStrings(3);
        System.out.println("Test 4: " + Arrays.toString(t4));

        String[] t5 = generateNRandomFloatStrings(3);
        System.out.println("Test 5: " + Arrays.toString(t5));
    }
}