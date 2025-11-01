import java.util.Random;
import java.util.Arrays;

public class Lse117 {

    /**
     * Generates three random floats and converts them to strings.
     * @return A String array containing the three converted random floats.
     */
    public static String[] generateAndConvertRandomFloats() {
        Random rand = new Random();
        
        // Generate three random floats (between 0.0 and 100.0)
        float a = rand.nextFloat() * 100.0f;
        float b = rand.nextFloat() * 100.0f;
        float c = rand.nextFloat() * 100.0f;
        
        // Convert floats to strings
        String str_a = Float.toString(a);
        String str_b = String.valueOf(b); // Another way to convert
        String str_c = Float.toString(c);
        
        return new String[]{str_a, str_b, str_c};
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases...");
        for (int i = 1; i <= 5; i++) {
            System.out.println("\n--- Test Case " + i + " ---");
            String[] results = generateAndConvertRandomFloats();
            String str_a = results[0];
            String str_b = results[1];
            String str_c = results[2];
            
            System.out.println("str_a: " + str_a);
            System.out.println("str_b: " + str_b);
            System.out.println("str_c: " + str_c);
        }
    }
}