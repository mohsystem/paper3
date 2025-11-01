import java.security.SecureRandom;
import java.util.Arrays;

public class Lse117 {

    /**
     * Converts a float to its string representation.
     * @param f The float to convert.
     * @return The string representation of the float.
     */
    public static String floatToString(float f) {
        return Float.toString(f);
    }

    /**
     * Generates three random floats using a cryptographically secure random number generator
     * and converts them to strings.
     *
     * @return An array of three strings, each representing a random float.
     */
    public static String[] generateThreeRandomFloatStrings() {
        SecureRandom random = new SecureRandom();
        
        // Generate three random floats in a wider range for variety
        float a = random.nextFloat() * 1000.0f;
        float b = random.nextFloat() * 1000.0f;
        float c = random.nextFloat() * 1000.0f;
        
        // Convert floats to strings using the helper function
        String str_a = floatToString(a);
        String str_b = floatToString(b);
        String str_c = floatToString(c);
        
        return new String[]{str_a, str_b, str_c};
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases...");
        for (int i = 0; i < 5; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            String[] results = generateThreeRandomFloatStrings();
            
            // Assign to variables as requested by the prompt's description
            String str_a = results[0];
            String str_b = results[1];
            String str_c = results[2];

            System.out.println("str_a: " + str_a);
            System.out.println("str_b: " + str_b);
            System.out.println("str_c: " + str_c);
        }
    }
}