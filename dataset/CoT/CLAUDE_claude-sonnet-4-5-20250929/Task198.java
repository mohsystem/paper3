
public class Task198 {
    // Simulated rand7() API
    private static int rand7() {
        return (int)(Math.random() * 7) + 1;
    }
    
    // rand10() implementation using rejection sampling
    public static int rand10() {
        int result;
        while (true) {
            // Generate number in range [0, 48]
            int num = (rand7() - 1) * 7 + (rand7() - 1);
            
            // Only accept numbers in range [0, 39] for uniform distribution
            if (num < 40) {
                result = (num % 10) + 1;
                break;
            }
            // Reject and retry if num >= 40
        }
        return result;
    }
    
    // Test function
    public static int[] testRand10(int n) {
        int[] results = new int[n];
        for (int i = 0; i < n; i++) {
            results[i] = rand10();
        }
        return results;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = testRand10(1);
        System.out.print("Test 1: [");
        for (int i = 0; i < test1.length; i++) {
            System.out.print(test1[i] + (i < test1.length - 1 ? "," : ""));
        }
        System.out.println("]");
        
        // Test case 2
        int[] test2 = testRand10(2);
        System.out.print("Test 2: [");
        for (int i = 0; i < test2.length; i++) {
            System.out.print(test2[i] + (i < test2.length - 1 ? "," : ""));
        }
        System.out.println("]");
        
        // Test case 3
        int[] test3 = testRand10(3);
        System.out.print("Test 3: [");
        for (int i = 0; i < test3.length; i++) {
            System.out.print(test3[i] + (i < test3.length - 1 ? "," : ""));
        }
        System.out.println("]");
        
        // Test case 4
        int[] test4 = testRand10(5);
        System.out.print("Test 4: [");
        for (int i = 0; i < test4.length; i++) {
            System.out.print(test4[i] + (i < test4.length - 1 ? "," : ""));
        }
        System.out.println("]");
        
        // Test case 5
        int[] test5 = testRand10(10);
        System.out.print("Test 5: [");
        for (int i = 0; i < test5.length; i++) {
            System.out.print(test5[i] + (i < test5.length - 1 ? "," : ""));
        }
        System.out.println("]");
    }
}
