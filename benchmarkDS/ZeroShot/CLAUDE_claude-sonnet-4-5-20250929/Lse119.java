
import java.util.Random;

public class Lse119 {
    public static String[] generateRandomFloatStrings() {
        Random rand = new Random();
        
        float float1 = rand.nextFloat();
        float float2 = rand.nextFloat();
        float float3 = rand.nextFloat();
        
        String string1 = String.format("%f", float1);
        String string2 = String.format("%f", float2);
        String string3 = String.format("%f", float3);
        
        return new String[]{string1, string2, string3};
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test case 1:");
        String[] result1 = generateRandomFloatStrings();
        System.out.println("String 1: " + result1[0]);
        System.out.println("String 2: " + result1[1]);
        System.out.println("String 3: " + result1[2]);
        System.out.println();
        
        // Test case 2
        System.out.println("Test case 2:");
        String[] result2 = generateRandomFloatStrings();
        System.out.println("String 1: " + result2[0]);
        System.out.println("String 2: " + result2[1]);
        System.out.println("String 3: " + result2[2]);
        System.out.println();
        
        // Test case 3
        System.out.println("Test case 3:");
        String[] result3 = generateRandomFloatStrings();
        System.out.println("String 1: " + result3[0]);
        System.out.println("String 2: " + result3[1]);
        System.out.println("String 3: " + result3[2]);
        System.out.println();
        
        // Test case 4
        System.out.println("Test case 4:");
        String[] result4 = generateRandomFloatStrings();
        System.out.println("String 1: " + result4[0]);
        System.out.println("String 2: " + result4[1]);
        System.out.println("String 3: " + result4[2]);
        System.out.println();
        
        // Test case 5
        System.out.println("Test case 5:");
        String[] result5 = generateRandomFloatStrings();
        System.out.println("String 1: " + result5[0]);
        System.out.println("String 2: " + result5[1]);
        System.out.println("String 3: " + result5[2]);
    }
}
