
import java.util.ArrayList;
import java.util.List;

public class Task15 {
    public static List<String> buildTower(int nFloors) {
        // Validate input
        if (nFloors <= 0) {
            throw new IllegalArgumentException("Number of floors must be positive");
        }
        if (nFloors > 10000) {
            throw new IllegalArgumentException("Number of floors exceeds maximum allowed");
        }
        
        List<String> tower = new ArrayList<>();
        int maxWidth = 2 * nFloors - 1;
        
        for (int i = 0; i < nFloors; i++) {
            int numStars = 2 * i + 1;
            int numSpaces = (maxWidth - numStars) / 2;
            
            StringBuilder floor = new StringBuilder();
            
            // Add leading spaces
            for (int j = 0; j < numSpaces; j++) {
                floor.append(' ');
            }
            
            // Add stars
            for (int j = 0; j < numStars; j++) {
                floor.append('*');
            }
            
            // Add trailing spaces
            for (int j = 0; j < numSpaces; j++) {
                floor.append(' ');
            }
            
            tower.add(floor.toString());
        }
        
        return tower;
    }
    
    public static void main(String[] args) {
        // Test case 1: 3 floors
        System.out.println("Test 1 - 3 floors:");
        List<String> tower1 = buildTower(3);
        for (String floor : tower1) {
            System.out.println("\\"" + floor + "\\"");
        }
        System.out.println();
        
        // Test case 2: 6 floors
        System.out.println("Test 2 - 6 floors:");
        List<String> tower2 = buildTower(6);
        for (String floor : tower2) {
            System.out.println("\\"" + floor + "\\"");
        }
        System.out.println();
        
        // Test case 3: 1 floor
        System.out.println("Test 3 - 1 floor:");
        List<String> tower3 = buildTower(1);
        for (String floor : tower3) {
            System.out.println("\\"" + floor + "\\"");
        }
        System.out.println();
        
        // Test case 4: 5 floors
        System.out.println("Test 4 - 5 floors:");
        List<String> tower4 = buildTower(5);
        for (String floor : tower4) {
            System.out.println("\\"" + floor + "\\"");
        }
        System.out.println();
        
        // Test case 5: 10 floors
        System.out.println("Test 5 - 10 floors:");
        List<String> tower5 = buildTower(10);
        for (String floor : tower5) {
            System.out.println("\\"" + floor + "\\"");
        }
    }
}
