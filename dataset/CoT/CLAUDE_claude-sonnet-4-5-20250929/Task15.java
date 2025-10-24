
public class Task15 {
    private static final int MAX_FLOORS = 1000; // Prevent excessive memory usage
    
    public static String[] buildTower(int nFloors) {
        // Input validation
        if (nFloors <= 0) {
            return new String[0];
        }
        if (nFloors > MAX_FLOORS) {
            throw new IllegalArgumentException("Number of floors exceeds maximum allowed: " + MAX_FLOORS);
        }
        
        String[] tower = new String[nFloors];
        int width = 2 * nFloors - 1;
        
        for (int i = 0; i < nFloors; i++) {
            int stars = 2 * i + 1;
            int spaces = (width - stars) / 2;
            
            StringBuilder floor = new StringBuilder();
            // Add leading spaces
            for (int j = 0; j < spaces; j++) {
                floor.append(' ');
            }
            // Add stars
            for (int j = 0; j < stars; j++) {
                floor.append('*');
            }
            // Add trailing spaces
            for (int j = 0; j < spaces; j++) {
                floor.append(' ');
            }
            
            tower[i] = floor.toString();
        }
        
        return tower;
    }
    
    public static void main(String[] args) {
        // Test case 1: 3 floors
        System.out.println("Test 1 - 3 floors:");
        String[] result1 = buildTower(3);
        for (String floor : result1) {
            System.out.println("\\"" + floor + "\\"");
        }
        
        // Test case 2: 6 floors
        System.out.println("\\nTest 2 - 6 floors:");
        String[] result2 = buildTower(6);
        for (String floor : result2) {
            System.out.println("\\"" + floor + "\\"");
        }
        
        // Test case 3: 1 floor
        System.out.println("\\nTest 3 - 1 floor:");
        String[] result3 = buildTower(1);
        for (String floor : result3) {
            System.out.println("\\"" + floor + "\\"");
        }
        
        // Test case 4: 0 floors (edge case)
        System.out.println("\\nTest 4 - 0 floors:");
        String[] result4 = buildTower(0);
        System.out.println("Empty array, length: " + result4.length);
        
        // Test case 5: 10 floors
        System.out.println("\\nTest 5 - 10 floors:");
        String[] result5 = buildTower(10);
        for (String floor : result5) {
            System.out.println("\\"" + floor + "\\"");
        }
    }
}
