
public class Task18 {
    public static int binaryArrayToInt(int[] binary) {
        int result = 0;
        for (int i = 0; i < binary.length; i++) {
            result = result * 2 + binary[i];
        }
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {0, 0, 0, 1};
        System.out.println("Testing: [0, 0, 0, 1] ==> " + binaryArrayToInt(test1));
        
        // Test case 2
        int[] test2 = {0, 0, 1, 0};
        System.out.println("Testing: [0, 0, 1, 0] ==> " + binaryArrayToInt(test2));
        
        // Test case 3
        int[] test3 = {0, 1, 0, 1};
        System.out.println("Testing: [0, 1, 0, 1] ==> " + binaryArrayToInt(test3));
        
        // Test case 4
        int[] test4 = {1, 0, 0, 1};
        System.out.println("Testing: [1, 0, 0, 1] ==> " + binaryArrayToInt(test4));
        
        // Test case 5
        int[] test5 = {1, 1, 1, 1};
        System.out.println("Testing: [1, 1, 1, 1] ==> " + binaryArrayToInt(test5));
    }
}
