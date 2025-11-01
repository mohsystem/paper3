import java.security.SecureRandom;
import java.util.Arrays;

public class Lse117 {

    /**
     * Generates three cryptographically secure random floats and converts them to strings.
     * @return A String array of size 3 containing the string representations of the random floats.
     */
    public static String[] generateAndConvertRandomFloats() {
        // Use SecureRandom for cryptographically strong random numbers, which is essential for security.
        SecureRandom random = new SecureRandom();
        String[] result = new String[3];

        // Generate three random floats. nextFloat() returns a float value between 0.0 (inclusive) and 1.0 (exclusive).
        float a = random.nextFloat();
        float b = random.nextFloat();
        float c = random.nextFloat();

        // Convert floats to strings. String.valueOf() is a safe and standard way to do this.
        result[0] = String.valueOf(a);
        result[1] = String.valueOf(b);
        result[2] = String.valueOf(c);

        return result;
    }

    public static void main(String[] args) {
        // Run 5 test cases to demonstrate the functionality.
        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            String[] randomStrings = generateAndConvertRandomFloats();
            
            // The resulting strings are stored in these variables as per the prompt.
            String str_a = randomStrings[0];
            String str_b = randomStrings[1];
            String str_c = randomStrings[2];

            System.out.println("  str_a: " + str_a);
            System.out.println("  str_b: " + str_b);
            System.out.println("  str_c: " + str_c);
            System.out.println();
        }
    }
}