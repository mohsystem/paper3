
public class Task18 {
    public static int binaryArrayToInt(int[] binary) {
        if (binary == null) {
            throw new IllegalArgumentException("Input array cannot be null");
        }
        
        if (binary.length == 0) {
            throw new IllegalArgumentException("Input array cannot be empty");
        }
        
        if (binary.length > 31) {
            throw new IllegalArgumentException("Input array length exceeds maximum supported size (31 bits)");
        }
        
        int result = 0;
        
        for (int i = 0; i < binary.length; i++) {
            if (binary[i] != 0 && binary[i] != 1) {
                throw new IllegalArgumentException("Array must contain only 0 or 1 values");
            }
            result = (result << 1) | binary[i];
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        int[][] testCases = {
            {0, 0, 0, 1},
            {0, 0, 1, 0},
            {0, 1, 0, 1},
            {1, 0, 0, 1},
            {1, 1, 1, 1}
        };
        
        for (int[] testCase : testCases) {
            System.out.println("Input: " + java.util.Arrays.toString(testCase) + 
                             " => Output: " + binaryArrayToInt(testCase));
        }
    }
}
