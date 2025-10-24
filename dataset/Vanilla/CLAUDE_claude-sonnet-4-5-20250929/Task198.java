
class Task198 {
    // Simulating rand7() API
    private static int rand7() {
        return (int)(Math.random() * 7) + 1;
    }
    
    public static int rand10() {
        while (true) {
            // Generate a number in range [1, 49]
            int row = rand7();
            int col = rand7();
            int num = (row - 1) * 7 + col;
            
            // Only use numbers 1-40 to ensure uniformity
            if (num <= 40) {
                return (num - 1) % 10 + 1;
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: n = 1
        System.out.print("Test 1 (n=1): [");
        for (int i = 0; i < 1; i++) {
            System.out.print(rand10());
            if (i < 0) System.out.print(",");
        }
        System.out.println("]");
        
        // Test case 2: n = 2
        System.out.print("Test 2 (n=2): [");
        for (int i = 0; i < 2; i++) {
            System.out.print(rand10());
            if (i < 1) System.out.print(",");
        }
        System.out.println("]");
        
        // Test case 3: n = 3
        System.out.print("Test 3 (n=3): [");
        for (int i = 0; i < 3; i++) {
            System.out.print(rand10());
            if (i < 2) System.out.print(",");
        }
        System.out.println("]");
        
        // Test case 4: n = 5
        System.out.print("Test 4 (n=5): [");
        for (int i = 0; i < 5; i++) {
            System.out.print(rand10());
            if (i < 4) System.out.print(",");
        }
        System.out.println("]");
        
        // Test case 5: n = 10
        System.out.print("Test 5 (n=10): [");
        for (int i = 0; i < 10; i++) {
            System.out.print(rand10());
            if (i < 9) System.out.print(",");
        }
        System.out.println("]");
    }
}
