
public class Task10 {
    public static int getSum(int a, int b) {
        // Handle case where a and b are equal
        if (a == b) {
            return a;
        }
        
        // Ensure min and max are correctly ordered
        int min = Math.min(a, b);
        int max = Math.max(a, b);
        
        // Use arithmetic series formula: sum = n * (first + last) / 2
        // where n is the count of numbers
        int count = max - min + 1;
        return count * (min + max) / 2;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println(getSum(1, 0));    // Expected: 1
        System.out.println(getSum(1, 2));    // Expected: 3
        System.out.println(getSum(0, 1));    // Expected: 1
        System.out.println(getSum(1, 1));    // Expected: 1
        System.out.println(getSum(-1, 0));   // Expected: -1
        System.out.println(getSum(-1, 2));   // Expected: 2
    }
}
