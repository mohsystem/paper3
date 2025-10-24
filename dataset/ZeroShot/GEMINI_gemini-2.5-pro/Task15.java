import java.util.Arrays;

public class Task15 {

    /**
     * Builds a pyramid-shaped tower as an array of strings.
     *
     * @param nFloors The number of floors in the tower. Must be a positive integer.
     * @return An array of strings representing the tower. Returns an empty array for nFloors <= 0.
     */
    public static String[] buildTower(int nFloors) {
        if (nFloors <= 0) {
            return new String[0];
        }

        String[] tower = new String[nFloors];
        int towerWidth = 2 * nFloors - 1;

        for (int i = 0; i < nFloors; i++) {
            int numStars = 2 * i + 1;
            int numSpaces = (towerWidth - numStars) / 2;
            
            // Using StringBuilder for efficient string construction
            StringBuilder floorBuilder = new StringBuilder(towerWidth);
            floorBuilder.append(" ".repeat(numSpaces));
            floorBuilder.append("*".repeat(numStars));
            floorBuilder.append(" ".repeat(numSpaces));
            
            tower[i] = floorBuilder.toString();
        }

        return tower;
    }

    /**
     * Helper function to print a tower in the specified format.
     */
    public static void printTower(String[] tower) {
        if (tower == null || tower.length == 0) {
            System.out.println("[]");
            return;
        }
        System.out.println("[");
        for (int i = 0; i < tower.length; i++) {
            System.out.print("  \"" + tower[i] + "\"");
            if (i < tower.length - 1) {
                System.out.println(",");
            } else {
                System.out.println();
            }
        }
        System.out.println("]");
    }

    public static void main(String[] args) {
        int[] testCases = {1, 3, 6, 0, -2};

        for (int n : testCases) {
            System.out.println("Tower with " + n + " floors:");
            String[] tower = buildTower(n);
            printTower(tower);
            System.out.println();
        }
    }
}