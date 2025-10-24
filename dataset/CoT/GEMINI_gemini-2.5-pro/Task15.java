public class Task15 {

    /**
     * Builds a pyramid-shaped tower as an array of strings.
     *
     * @param nFloors The number of floors in the tower (must be a positive integer).
     * @return An array of strings representing the tower. Returns an empty array if nFloors is not positive.
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
            
            StringBuilder sb = new StringBuilder(towerWidth);
            
            // Append leading spaces
            for (int j = 0; j < numSpaces; j++) {
                sb.append(' ');
            }
            // Append stars
            for (int j = 0; j < numStars; j++) {
                sb.append('*');
            }
            // Append trailing spaces
            for (int j = 0; j < numSpaces; j++) {
                sb.append(' ');
            }
            
            tower[i] = sb.toString();
        }

        return tower;
    }

    public static void main(String[] args) {
        int[] testCases = {1, 3, 6, 0, 5};

        for (int i = 0; i < testCases.length; i++) {
            int nFloors = testCases[i];
            System.out.println("Test Case " + (i + 1) + ": nFloors = " + nFloors);
            String[] tower = buildTower(nFloors);
            
            if (tower.length == 0) {
                System.out.println("[]");
            } else {
                System.out.println("[");
                for (String floor : tower) {
                    System.out.println("  \"" + floor + "\"");
                }
                System.out.println("]");
            }
            System.out.println("--------------------");
        }
    }
}