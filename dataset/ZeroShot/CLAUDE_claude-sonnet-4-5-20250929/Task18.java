
public class Task18 {
    public static int binaryArrayToNumber(int[] binary) {
        if (binary == null || binary.length == 0) {
            return 0;
        }
        
        int result = 0;
        for (int i = 0; i < binary.length; i++) {
            result = result * 2 + binary[i];
        }
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {0, 0, 0, 1};
        System.out.println("Testing: [0, 0, 0, 1] ==> " + binaryArrayToNumber(test1));
        
        // Test case 2
        int[] test2 = {0, 0, 1, 0};
        System.out.println("Testing: [0, 0, 1, 0] ==> " + binaryArrayToNumber(test2));
        
        // Test case 3
        int[] test3 = {0, 1, 0, 1};
        System.out.println("Testing: [0, 1, 0, 1] ==> " + binaryArrayToNumber(test3));
        
        // Test case 4
        int[] test4 = {1, 0, 0, 1};
        System.out.println("Testing: [1, 0, 0, 1] ==> " + binaryArrayToNumber(test4));
        
        // Test case 5
        int[] test5 = {1, 1, 1, 1};
        System.out.println("Testing: [1, 1, 1, 1] ==> " + binaryArrayToNumber(test5));
    }
}
