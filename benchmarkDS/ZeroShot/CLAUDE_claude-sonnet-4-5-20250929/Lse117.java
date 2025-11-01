
import java.util.Random;

public class Lse117 {
    public static String floatToString(float value) {
        return String.valueOf(value);
    }
    
    public static void main(String[] args) {
        Random random = new Random();
        
        // Test case 1
        float a1 = random.nextFloat();
        float b1 = random.nextFloat();
        float c1 = random.nextFloat();
        String str_a1 = floatToString(a1);
        String str_b1 = floatToString(b1);
        String str_c1 = floatToString(c1);
        System.out.println("Test 1: " + str_a1 + ", " + str_b1 + ", " + str_c1);
        
        // Test case 2
        float a2 = random.nextFloat();
        float b2 = random.nextFloat();
        float c2 = random.nextFloat();
        String str_a2 = floatToString(a2);
        String str_b2 = floatToString(b2);
        String str_c2 = floatToString(c2);
        System.out.println("Test 2: " + str_a2 + ", " + str_b2 + ", " + str_c2);
        
        // Test case 3
        float a3 = random.nextFloat();
        float b3 = random.nextFloat();
        float c3 = random.nextFloat();
        String str_a3 = floatToString(a3);
        String str_b3 = floatToString(b3);
        String str_c3 = floatToString(c3);
        System.out.println("Test 3: " + str_a3 + ", " + str_b3 + ", " + str_c3);
        
        // Test case 4
        float a4 = random.nextFloat();
        float b4 = random.nextFloat();
        float c4 = random.nextFloat();
        String str_a4 = floatToString(a4);
        String str_b4 = floatToString(b4);
        String str_c4 = floatToString(c4);
        System.out.println("Test 4: " + str_a4 + ", " + str_b4 + ", " + str_c4);
        
        // Test case 5
        float a5 = random.nextFloat();
        float b5 = random.nextFloat();
        float c5 = random.nextFloat();
        String str_a5 = floatToString(a5);
        String str_b5 = floatToString(b5);
        String str_c5 = floatToString(c5);
        System.out.println("Test 5: " + str_a5 + ", " + str_b5 + ", " + str_c5);
    }
}
