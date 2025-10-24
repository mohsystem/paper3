
public class Task198 {
    // Simulated rand7() function for testing
    private static int rand7() {
        return (int)(Math.random() * 7) + 1;
    }
    
    public static int rand10() {
        int result;
        do {
            // Generate a number in range [1, 49]
            int row = rand7();
            int col = rand7();
            result = (row - 1) * 7 + col;
        } while (result > 40); // Reject numbers > 40 to maintain uniform distribution
        
        // Map [1, 40] to [1, 10]
        return (result - 1) % 10 + 1;
    }
    
    public static int[] testRand10(int n) {
        int[] results = new int[n];
        for (int i = 0; i < n; i++) {
            results[i] = rand10();
        }
        return results;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] result1 = testRand10(1);
        System.out.print("Test 1 (n=1): [");
        for (int i = 0; i < result1.length; i++) {
            System.out.print(result1[i] + (i < result1.length - 1 ? "," : ""));
        }
        System.out.println("]");
        
        // Test case 2
        int[] result2 = testRand10(2);
        System.out.print("Test 2 (n=2): [");
        for (int i = 0; i < result2.length; i++) {
            System.out.print(result2[i] + (i < result2.length - 1 ? "," : ""));
        }
        System.out.println("]");
        
        // Test case 3
        int[] result3 = testRand10(3);
        System.out.print("Test 3 (n=3): [");
        for (int i = 0; i < result3.length; i++) {
            System.out.print(result3[i] + (i < result3.length - 1 ? "," : ""));
        }
        System.out.println("]");
        
        // Test case 4
        int[] result4 = testRand10(5);
        System.out.print("Test 4 (n=5): [");
        for (int i = 0; i < result4.length; i++) {
            System.out.print(result4[i] + (i < result4.length - 1 ? "," : ""));
        }
        System.out.println("]");
        
        // Test case 5
        int[] result5 = testRand10(10);
        System.out.print("Test 5 (n=10): [");
        for (int i = 0; i < result5.length; i++) {
            System.out.print(result5[i] + (i < result5.length - 1 ? "," : ""));
        }
        System.out.println("]");
    }
}
