import java.util.Random;
import java.util.Arrays;

public class Lse119 {

    /**
     * Generates three random floats and converts them to strings.
     *
     * @return An array of three strings, each representing a random float.
     */
    public static String[] generateRandomFloatStrings() {
        // Using Random is sufficient for non-cryptographic purposes.
        // For security-sensitive applications, java.security.SecureRandom should be used.
        Random rand = new Random();
        String[] result = new String[3];

        float f1 = rand.nextFloat();
        float f2 = rand.nextFloat();
        float f3 = rand.nextFloat();

        // String.format is a safe alternative to C's sprintf, preventing buffer overflows.
        result[0] = String.format("%f", f1);
        result[1] = String.format("%f", f2);
        result[2] = String.format("%f", f3);

        return result;
    }

    public static void main(String[] args) {
        // 5 test cases
        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            String[] randomStrings = generateRandomFloatStrings();
            System.out.println("string: " + randomStrings[0]);
            System.out.println("string2: " + randomStrings[1]);
            System.out.println("string3: " + randomStrings[2]);
            System.out.println();
        }
    }
}