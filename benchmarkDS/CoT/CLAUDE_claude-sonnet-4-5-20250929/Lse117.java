
import java.util.Random;

public class Lse117 {
    public static String[] generateRandomFloatStrings() {
        // Create a secure random number generator
        Random random = new Random();
        
        // Generate three random floats
        float floatA = random.nextFloat();
        float floatB = random.nextFloat();
        float floatC = random.nextFloat();
        
        // Convert floats to strings
        String strA = String.valueOf(floatA);
        String strB = String.valueOf(floatB);
        String strC = String.valueOf(floatC);
        
        // Return the strings in an array
        return new String[]{strA, strB, strC};
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test Case 1:");
        String[] result1 = generateRandomFloatStrings();
        System.out.println("str_a: " + result1[0]);
        System.out.println("str_b: " + result1[1]);
        System.out.println("str_c: " + result1[2]);
        System.out.println();
        
        // Test case 2
        System.out.println("Test Case 2:");
        String[] result2 = generateRandomFloatStrings();
        System.out.println("str_a: " + result2[0]);
        System.out.println("str_b: " + result2[1]);
        System.out.println("str_c: " + result2[2]);
        System.out.println();
        
        // Test case 3
        System.out.println("Test Case 3:");
        String[] result3 = generateRandomFloatStrings();
        System.out.println("str_a: " + result3[0]);
        System.out.println("str_b: " + result3[1]);
        System.out.println("str_c: " + result3[2]);
        System.out.println();
        
        // Test case 4
        System.out.println("Test Case 4:");
        String[] result4 = generateRandomFloatStrings();
        System.out.println("str_a: " + result4[0]);
        System.out.println("str_b: " + result4[1]);
        System.out.println("str_c: " + result4[2]);
        System.out.println();
        
        // Test case 5
        System.out.println("Test Case 5:");
        String[] result5 = generateRandomFloatStrings();
        System.out.println("str_a: " + result5[0]);
        System.out.println("str_b: " + result5[1]);
        System.out.println("str_c: " + result5[2]);
    }
}
