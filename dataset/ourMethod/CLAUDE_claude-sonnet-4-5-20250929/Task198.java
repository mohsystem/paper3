
public class Task198 {
    // Simulated rand7() for testing purposes
    private static int rand7() {
        return (int)(Math.random() * 7) + 1;
    }
    
    public static int rand10() {
        int row, col, idx;
        do {
            row = rand7();
            col = rand7();
            idx = (row - 1) * 7 + col;
        } while (idx > 40);
        return ((idx - 1) % 10) + 1;
    }
    
    public static void main(String[] args) {
        // Test case 1: n = 1
        System.out.print("Test 1: [");
        System.out.print(rand10());
        System.out.println("]");
        
        // Test case 2: n = 2
        System.out.print("Test 2: [");
        for (int i = 0; i < 2; i++) {
            if (i > 0) System.out.print(",");
            System.out.print(rand10());
        }
        System.out.println("]");
        
        // Test case 3: n = 3
        System.out.print("Test 3: [");
        for (int i = 0; i < 3; i++) {
            if (i > 0) System.out.print(",");
            System.out.print(rand10());
        }
        System.out.println("]");
        
        // Test case 4: n = 5
        System.out.print("Test 4: [");
        for (int i = 0; i < 5; i++) {
            if (i > 0) System.out.print(",");
            System.out.print(rand10());
        }
        System.out.println("]");
        
        // Test case 5: n = 10
        System.out.print("Test 5: [");
        for (int i = 0; i < 10; i++) {
            if (i > 0) System.out.print(",");
            System.out.print(rand10());
        }
        System.out.println("]");
    }
}
