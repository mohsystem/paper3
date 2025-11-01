
import java.util.Random;

public class Lse119 {
    public static void generateRandomFloatStrings() {
        Random rand = new Random();
        
        // Generate three random floats
        float float1 = rand.nextFloat();
        float float2 = rand.nextFloat();
        float float3 = rand.nextFloat();
        
        // Convert to strings (Java uses String.format instead of sprintf)
        String string1 = String.format("%f", float1);
        String string2 = String.format("%f", float2);
        String string3 = String.format("%f", float3);
        
        System.out.println("Float 1: " + string1);
        System.out.println("Float 2: " + string2);
        System.out.println("Float 3: " + string3);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        generateRandomFloatStrings();
        
        System.out.println("\\nTest Case 2:");
        generateRandomFloatStrings();
        
        System.out.println("\\nTest Case 3:");
        generateRandomFloatStrings();
        
        System.out.println("\\nTest Case 4:");
        generateRandomFloatStrings();
        
        System.out.println("\\nTest Case 5:");
        generateRandomFloatStrings();
    }
}
