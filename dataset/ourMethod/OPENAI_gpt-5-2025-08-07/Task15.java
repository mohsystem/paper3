import java.util.ArrayList;
import java.util.List;

public class Task15 {

    private static final int MAX_FLOORS = 1000;

    public static List<String> buildTower(int floors) {
        if (floors < 1 || floors > MAX_FLOORS) {
            throw new IllegalArgumentException("floors must be between 1 and " + MAX_FLOORS);
        }
        final int width = 2 * floors - 1;
        List<String> result = new ArrayList<>(floors);
        for (int i = 1; i <= floors; i++) {
            int spaces = floors - i;
            int stars = 2 * i - 1;
            StringBuilder sb = new StringBuilder(width);
            for (int s = 0; s < spaces; s++) sb.append(' ');
            for (int st = 0; st < stars; st++) sb.append('*');
            for (int s = 0; s < spaces; s++) sb.append(' ');
            result.add(sb.toString());
        }
        return result;
    }

    private static void printTower(List<String> tower) {
        System.out.println("[");
        for (int i = 0; i < tower.size(); i++) {
            String row = tower.get(i);
            System.out.println("  \"" + row + "\"" + (i + 1 < tower.size() ? "," : ""));
        }
        System.out.println("]");
    }

    public static void main(String[] args) {
        int[] tests = new int[] {1, 3, 6, 2, 0};
        for (int t : tests) {
            System.out.println("Test floors=" + t);
            try {
                List<String> tower = buildTower(t);
                printTower(tower);
            } catch (IllegalArgumentException ex) {
                System.out.println("Error: " + ex.getMessage());
            }
            System.out.println();
        }
    }
}