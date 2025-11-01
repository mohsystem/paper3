import java.util.Arrays;
import java.util.Locale;
import java.util.Random;

public class Lse119 {
    public static String[] generateRandomFloatStrings(long seed) {
        Random rand = new Random(seed);
        float f1 = rand.nextFloat();
        float f2 = rand.nextFloat();
        float f3 = rand.nextFloat();

        String string = String.format(Locale.US, "%.6f", f1);
        String string2 = String.format(Locale.US, "%.6f", f2);
        String string3 = String.format(Locale.US, "%.6f", f3);

        return new String[]{string, string2, string3};
    }

    public static void main(String[] args) {
        long[] seeds = {1L, 2L, 3L, 4L, 5L};
        for (long s : seeds) {
            String[] out = generateRandomFloatStrings(s);
            System.out.println("Seed " + s + ": " + Arrays.toString(out));
        }
    }
}