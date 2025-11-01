
import java.util.Random;

public class Lse119 {
    public static String[] generateRandomFloatStrings() {
        Random rand = new Random();
        
        // Generate three random floats
        float float1 = rand.nextFloat();
        float float2 = rand.nextFloat();
        float float3 = rand.nextFloat();
        
        // Convert to strings
        String string1 = String.format("%f", float1);
        String string2 = String.format("%f", float2);
        String string3 = String.format("%f", float3);
        
        return new String[]{string1, string2, string3};
    }
    
    public static void main(String[] args) {
        // Test case 1
        String[] result1 = generateRandomFloatStrings();
        System.out.println("Test 1: " + result1[0] + ", " + result1[1] + ", " + result1[2]);
        
        // Test case 2
        String[] result2 = generateRandomFloatStrings();
        System.out.println("Test 2: " + result2[0] + ", " + result2[1] + ", " + result2[2]);
        
        // Test case 3
        String[] result3 = generateRandomFloatStrings();
        System.out.println("Test 3: " + result3[0] + ", " + result3[1] + ", " + result3[2]);
        
        // Test case 4
        String[] result4 = generateRandomFloatStrings();
        System.out.println("Test 4: " + result4[0] + ", " + result4[1] + ", " + result4[2]);
        
        // Test case 5
        String[] result5 = generateRandomFloatStrings();
        System.out.println("Test 5: " + result5[0] + ", " + result5[1] + ", " + result5[2]);
    }
}
