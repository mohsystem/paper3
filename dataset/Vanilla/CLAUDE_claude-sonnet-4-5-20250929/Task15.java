
public class Task15 {
    public static String[] buildTower(int nFloors) {
        String[] tower = new String[nFloors];
        int maxWidth = 2 * nFloors - 1;
        
        for (int i = 0; i < nFloors; i++) {
            int stars = 2 * i + 1;
            int spaces = (maxWidth - stars) / 2;
            
            StringBuilder floor = new StringBuilder();
            for (int j = 0; j < spaces; j++) {
                floor.append(" ");
            }
            for (int j = 0; j < stars; j++) {
                floor.append("*");
            }
            for (int j = 0; j < spaces; j++) {
                floor.append(" ");
            }
            
            tower[i] = floor.toString();
        }
        
        return tower;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test case 1: 3 floors");
        String[] tower1 = buildTower(3);
        for (String floor : tower1) {
            System.out.println("\\"" + floor + "\\"");
        }
        System.out.println();
        
        // Test case 2
        System.out.println("Test case 2: 6 floors");
        String[] tower2 = buildTower(6);
        for (String floor : tower2) {
            System.out.println("\\"" + floor + "\\"");
        }
        System.out.println();
        
        // Test case 3
        System.out.println("Test case 3: 1 floor");
        String[] tower3 = buildTower(1);
        for (String floor : tower3) {
            System.out.println("\\"" + floor + "\\"");
        }
        System.out.println();
        
        // Test case 4
        System.out.println("Test case 4: 5 floors");
        String[] tower4 = buildTower(5);
        for (String floor : tower4) {
            System.out.println("\\"" + floor + "\\"");
        }
        System.out.println();
        
        // Test case 5
        System.out.println("Test case 5: 10 floors");
        String[] tower5 = buildTower(10);
        for (String floor : tower5) {
            System.out.println("\\"" + floor + "\\"");
        }
    }
}
