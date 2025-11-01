
import java.security.SecureRandom;
import java.util.Locale;

public class Lse117 {
    // Use SecureRandom instead of Random for cryptographically secure random generation
    private static final SecureRandom secureRandom = new SecureRandom();
    
    /**
     * Generates a cryptographically secure random float between 0.0 and 1.0
     * @return a random float value
     */
    public static float generateRandomFloat() {
        // SecureRandom.nextFloat() provides cryptographically secure randomness
        return secureRandom.nextFloat();
    }
    
    /**
     * Converts a float to string with fixed locale to prevent locale-dependent issues
     * @param value the float to convert
     * @return string representation of the float
     */
    public static String floatToString(float value) {
        // Use Locale.ROOT to ensure consistent string formatting regardless of system locale
        // This prevents potential injection or parsing issues with locale-specific separators
        return String.format(Locale.ROOT, "%f", value);
    }
    
    /**
     * Generates three random floats and converts them to strings
     * @return array containing three string representations of random floats
     */
    public static String[] generateThreeRandomFloatStrings() {
        // Generate three cryptographically secure random floats
        float floatA = generateRandomFloat();
        float floatB = generateRandomFloat();
        float floatC = generateRandomFloat();
        
        // Convert to strings using secure conversion with fixed locale
        String strA = floatToString(floatA);
        String strB = floatToString(floatB);
        String strC = floatToString(floatC);
        
        // Return as array for easy access
        return new String[]{strA, strB, strC};
    }
    
    public static void main(String[] args) {
        // Test case 1
        String[] result1 = generateThreeRandomFloatStrings();
        System.out.println("Test 1 - str_a: " + result1[0] + ", str_b: " + result1[1] + ", str_c: " + result1[2]);
        
        // Test case 2
        String[] result2 = generateThreeRandomFloatStrings();
        System.out.println("Test 2 - str_a: " + result2[0] + ", str_b: " + result2[1] + ", str_c: " + result2[2]);
        
        // Test case 3
        String[] result3 = generateThreeRandomFloatStrings();
        System.out.println("Test 3 - str_a: " + result3[0] + ", str_b: " + result3[1] + ", str_c: " + result3[2]);
        
        // Test case 4
        String[] result4 = generateThreeRandomFloatStrings();
        System.out.println("Test 4 - str_a: " + result4[0] + ", str_b: " + result4[1] + ", str_c: " + result4[2]);
        
        // Test case 5
        String[] result5 = generateThreeRandomFloatStrings();
        System.out.println("Test 5 - str_a: " + result5[0] + ", str_b: " + result5[1] + ", str_c: " + result5[2]);
    }
}
