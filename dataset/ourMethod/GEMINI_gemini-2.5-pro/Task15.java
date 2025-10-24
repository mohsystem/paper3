import java.util.Arrays;

public class Task15 {

    /**
     * Builds a pyramid-shaped tower.
     *
     * @param nFloors The number of floors in the tower (must be positive).
     * @return An array of strings representing the tower.
     */
    public static String[] buildTower(int nFloors) {
        if (nFloors <= 0) {
            return new String[0];
        }

        String[] tower = new String[nFloors];
        int towerWidth = 2 * nFloors - 1;

        for (int i = 0; i < nFloors; i++) {
            int stars = 2 * i + 1;
            int padding = (towerWidth - stars) / 2;
            
            StringBuilder sb = new StringBuilder(towerWidth);
            for (int j = 0; j < padding; j++) {
                sb.append(' ');
            }
            for (int j = 0; j < stars; j++) {
                sb.append('*');
            }
            for (int j = 0; j < padding; j++) {
                sb.append(' ');
            }
            tower[i] = sb.toString();
        }

        return tower;
    }

    public static void main(String[] args) {
        int[] testCases = {1, 3, 6, 0, -5};
        for (int floors : testCases) {
            System.out.println("Tower with " + floors + " floors:");
            String[] tower = buildTower(floors);
            if (tower.length == 0) {
                System.out.println("[]");
            } else {
                System.out.println("[");
                for (String floor : tower) {
                    System.out.println("  \"" + floor + "\"");
                }
                System.out.println("]");
            }
            System.out.println();
        }
    }
}