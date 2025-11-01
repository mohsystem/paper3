import java.util.Random;
import java.util.Arrays;

public class Lse117 {
    
    /**
     * Generates three random floats and converts them to strings.
     * @return An array of three strings, each representing a random float.
     */
    public static String[] generateRandomFloatStrings() {
        Random rand = new Random();
        
        // Generate three random floats
        float a = rand.nextFloat() * 100.0f;
        float b = rand.nextFloat() * 100.0f;
        float c = rand.nextFloat() * 100.0f;
        
        // Convert floats to strings
        String str_a = Float.toString(a);
        String str_b = Float.toString(b);
        String str_c = Float.toString(c);
        
        return new String[]{str_a, str_b, str_c};
    }

    public static void main(String[] args) {
        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            String[] results = generateRandomFloatStrings();
            String str_a = results[0];
            String str_b = results[1];
            String str_c = results[2];
            
            System.out.println("  str_a: " + str_a);
            System.out.println("  str_b: " + str_b);
            System.out.println("  str_c: " + str_c);
            System.out.println();
        }
    }
}