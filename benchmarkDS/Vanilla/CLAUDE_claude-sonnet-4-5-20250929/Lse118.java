
import java.io.FileWriter;
import java.io.IOException;
import java.util.Random;

public class Lse118 {
    public static void writeRandomFloatsToFile(String filename) {
        Random random = new Random();
        
        // Generate 3 random floats
        float float1 = random.nextFloat();
        float float2 = random.nextFloat();
        float float3 = random.nextFloat();
        
        // Convert them to strings
        String str1 = Float.toString(float1);
        String str2 = Float.toString(float2);
        String str3 = Float.toString(float3);
        
        // Concatenate them into a single string
        String concatenated = str1 + str2 + str3;
        
        // Write the string to a file
        try (FileWriter writer = new FileWriter(filename)) {
            writer.write(concatenated);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        writeRandomFloatsToFile("output1.txt");
        
        // Test case 2
        writeRandomFloatsToFile("output2.txt");
        
        // Test case 3
        writeRandomFloatsToFile("output3.txt");
        
        // Test case 4
        writeRandomFloatsToFile("output4.txt");
        
        // Test case 5
        writeRandomFloatsToFile("output5.txt");
        
        System.out.println("Files created successfully!");
    }
}
