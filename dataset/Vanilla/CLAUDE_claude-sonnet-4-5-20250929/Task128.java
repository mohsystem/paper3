
import java.util.Random;
import java.security.SecureRandom;

public class Task128 {
    // Generate random integer between min and max (inclusive)
    public static int generateRandomInt(int min, int max) {
        Random random = new Random();
        return random.nextInt(max - min + 1) + min;
    }
    
    // Generate random double between 0.0 and 1.0
    public static double generateRandomDouble() {
        Random random = new Random();
        return random.nextDouble();
    }
    
    // Generate random alphanumeric token of specified length
    public static String generateRandomToken(int length) {
        String characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        SecureRandom random = new SecureRandom();
        StringBuilder token = new StringBuilder(length);
        
        for (int i = 0; i < length; i++) {
            int index = random.nextInt(characters.length());
            token.append(characters.charAt(index));
        }
        
        return token.toString();
    }
    
    // Generate random boolean
    public static boolean generateRandomBoolean() {
        Random random = new Random();
        return random.nextBoolean();
    }
    
    // Generate array of random integers
    public static int[] generateRandomIntArray(int size, int min, int max) {
        Random random = new Random();
        int[] array = new int[size];
        
        for (int i = 0; i < size; i++) {
            array[i] = random.nextInt(max - min + 1) + min;
        }
        
        return array;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Random integer between 1 and 100");
        System.out.println(generateRandomInt(1, 100));
        
        System.out.println("\\nTest Case 2: Random double");
        System.out.println(generateRandomDouble());
        
        System.out.println("\\nTest Case 3: Random token of length 16");
        System.out.println(generateRandomToken(16));
        
        System.out.println("\\nTest Case 4: Random boolean");
        System.out.println(generateRandomBoolean());
        
        System.out.println("\\nTest Case 5: Array of 5 random integers between 10 and 50");
        int[] randomArray = generateRandomIntArray(5, 10, 50);
        for (int num : randomArray) {
            System.out.print(num + " ");
        }
        System.out.println();
    }
}
