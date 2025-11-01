import java.util.Random;
import java.util.Arrays;

public class Lse119 {

    /**
     * Generates three random float numbers and converts them to strings.
     *
     * @return An array of three strings, each representing a random float.
     */
    public static String[] generateRandomFloatStrings() {
        Random rand = new Random();
        String[] resultStrings = new String[3];

        // Generate three random floats
        float f1 = rand.nextFloat();
        float f2 = rand.nextFloat();
        float f3 = rand.nextFloat();

        // Convert floats to strings using String.format (similar to sprintf)
        String string1 = String.format("%f", f1);
        String string2 = String.format("%f", f2);
        String string3 = String.format("%f", f3);

        resultStrings[0] = string1;
        resultStrings[1] = string2;
        resultStrings[2] = string3;

        return resultStrings;
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases for Java:");
        for (int i = 0; i < 5; i++) {
            String[] randomStrings = generateRandomFloatStrings();
            System.out.println("Test Case " + (i + 1) + ": " + Arrays.toString(randomStrings));
        }
    }
}