import java.util.Random;
import java.util.Arrays;

public class Lse119 {
    /**
     * Generates three random floats and converts them to strings.
     * @return An array of three strings, each representing a random float.
     */
    public static String[] generateRandomFloatStrings() {
        Random rand = new Random();
        float f1 = rand.nextFloat();
        float f2 = rand.nextFloat();
        float f3 = rand.nextFloat();

        String[] result = new String[3];
        // Using String.format which is Java's equivalent to sprintf
        result[0] = String.format("%f", f1);
        result[1] = String.format("%f", f2);
        result[2] = String.format("%f", f3);

        return result;
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases:");
        for (int i = 0; i < 5; i++) {
            String[] randomStrings = generateRandomFloatStrings();
            System.out.println("Test Case " + (i + 1) + ": " + Arrays.toString(randomStrings));
        }
    }
}