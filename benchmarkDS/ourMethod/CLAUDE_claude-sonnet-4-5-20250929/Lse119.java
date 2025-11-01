
import java.security.SecureRandom;
import java.util.Locale;

public class Lse119 {
    // Use SecureRandom instead of Math.random() for security-sensitive randomness
    private static final SecureRandom secureRandom = new SecureRandom();
    
    /**
     * Generates three random floats and converts them to strings.
     * Returns an array containing the three string representations.
     * 
     * Security measures:
     * - Uses SecureRandom instead of Math.random() for cryptographically strong randomness
     * - Uses String.format with fixed format to prevent format string vulnerabilities
     * - Validates output strings are not null before returning
     * - Uses Locale.US to ensure consistent decimal formatting across locales
     */
    public static String[] generateRandomFloatStrings() {
        // Generate three cryptographically strong random floats between 0.0 and 1.0
        float random1 = secureRandom.nextFloat();
        float random2 = secureRandom.nextFloat();
        float random3 = secureRandom.nextFloat();
        
        // Convert to strings using String.format (safe sprintf equivalent in Java)
        // Using Locale.US ensures consistent decimal point formatting
        String string1 = String.format(Locale.US, "%f", random1);
        String string2 = String.format(Locale.US, "%f", random2);
        String string3 = String.format(Locale.US, "%f", random3);
        
        // Validate outputs are not null (defensive programming)
        if (string1 == null || string2 == null || string3 == null) {
            throw new RuntimeException("String conversion failed");
        }
        
        return new String[]{string1, string2, string3};
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test case 1:");
        String[] result1 = generateRandomFloatStrings();
        System.out.println("Random float 1: " + result1[0]);
        System.out.println("Random float 2: " + result1[1]);
        System.out.println("Random float 3: " + result1[2]);
        System.out.println();
        
        // Test case 2
        System.out.println("Test case 2:");
        String[] result2 = generateRandomFloatStrings();
        System.out.println("Random float 1: " + result2[0]);
        System.out.println("Random float 2: " + result2[1]);
        System.out.println("Random float 3: " + result2[2]);
        System.out.println();
        
        // Test case 3
        System.out.println("Test case 3:");
        String[] result3 = generateRandomFloatStrings();
        System.out.println("Random float 1: " + result3[0]);
        System.out.println("Random float 2: " + result3[1]);
        System.out.println("Random float 3: " + result3[2]);
        System.out.println();
        
        // Test case 4
        System.out.println("Test case 4:");
        String[] result4 = generateRandomFloatStrings();
        System.out.println("Random float 1: " + result4[0]);
        System.out.println("Random float 2: " + result4[1]);
        System.out.println("Random float 3: " + result4[2]);
        System.out.println();
        
        // Test case 5
        System.out.println("Test case 5:");
        String[] result5 = generateRandomFloatStrings();
        System.out.println("Random float 1: " + result5[0]);
        System.out.println("Random float 2: " + result5[1]);
        System.out.println("Random float 3: " + result5[2]);
    }
}
