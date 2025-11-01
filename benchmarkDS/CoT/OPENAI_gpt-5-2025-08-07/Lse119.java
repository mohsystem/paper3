// Chain-of-Through Step 1: Problem understanding
// - Generate three random floats and convert them to strings similar to sprintf, store in variables: string, string2, string3.
// - Function should accept input parameters and return output.
// - Include main method with 5 test cases. Class name: Lse119.

// Chain-of-Through Step 2: Security requirements
// - Use safe formatting (String.format) and avoid insecure practices.
// - Allow seeding for reproducibility without relying on global state.

// Chain-of-Through Step 3: Secure coding generation
// - Use java.util.Random with optional seed parameter.
// - Format numbers with fixed precision.

// Chain-of-Through Step 4: Code review
// - No shared mutable state.
// - Proper handling of null seed.
// - Locale-safe formatting.

// Chain-of-Through Step 5: Secure code output
import java.util.*;

public class Lse119 {

    // Generates three random floats and returns their string representations.
    // Parameters:
    // - seed: Long seed for deterministic output (nullable). If null, a non-deterministic seed is used.
    // - precision: number of digits after decimal point.
    // Returns: String[] containing string, string2, string3.
    public static String[] generateRandomFloatStrings(Long seed, int precision) {
        Random rng = (seed == null) ? new Random() : new Random(seed);
        float f1 = rng.nextFloat();
        float f2 = rng.nextFloat();
        float f3 = rng.nextFloat();

        String fmt = "%." + Math.max(0, precision) + "f";
        String string  = String.format(Locale.US, fmt, f1);
        String string2 = String.format(Locale.US, fmt, f2);
        String string3 = String.format(Locale.US, fmt, f3);

        return new String[]{string, string2, string3};
    }

    public static void main(String[] args) {
        // 5 test cases with varying seeds and precision
        Long[] seeds = new Long[]{0L, 1L, 42L, 123456789L, null};
        int[] precisions = new int[]{6, 4, 2, 8, 6};

        for (int i = 0; i < seeds.length; i++) {
            String[] out = generateRandomFloatStrings(seeds[i], precisions[i]);
            System.out.println("Seed=" + seeds[i] + ", precision=" + precisions[i] + " -> " + Arrays.toString(out));
        }
    }
}