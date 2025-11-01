
import java.io.FileWriter;
import java.io.IOException;
import java.util.Random;

public class Lse118 {
    public static String generateAndConcatenateFloats() {
        Random random = new Random();
        float float1 = random.nextFloat();
        float float2 = random.nextFloat();
        float float3 = random.nextFloat();
        
        String concatenated = String.valueOf(float1) + String.valueOf(float2) + String.valueOf(float3);
        return concatenated;
    }
    
    public static void writeToFile(String content, String filename) {
        try (FileWriter writer = new FileWriter(filename)) {
            writer.write(content);
        } catch (IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        String result1 = generateAndConcatenateFloats();
        writeToFile(result1, "output1.txt");
        System.out.println("Test 1: " + result1);
        
        // Test case 2
        String result2 = generateAndConcatenateFloats();
        writeToFile(result2, "output2.txt");
        System.out.println("Test 2: " + result2);
        
        // Test case 3
        String result3 = generateAndConcatenateFloats();
        writeToFile(result3, "output3.txt");
        System.out.println("Test 3: " + result3);
        
        // Test case 4
        String result4 = generateAndConcatenateFloats();
        writeToFile(result4, "output4.txt");
        System.out.println("Test 4: " + result4);
        
        // Test case 5
        String result5 = generateAndConcatenateFloats();
        writeToFile(result5, "output5.txt");
        System.out.println("Test 5: " + result5);
    }
}
