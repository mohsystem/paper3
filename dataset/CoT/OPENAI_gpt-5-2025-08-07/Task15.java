import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Task15 {

    // Build tower function: returns list of strings representing the tower
    public static List<String> buildTower(int n) {
        if (n <= 0) {
            return Collections.emptyList();
        }
        long widthLong = 2L * n - 1L;
        if (widthLong > Integer.MAX_VALUE) {
            return Collections.emptyList();
        }
        int width = (int) widthLong;

        List<String> result = new ArrayList<>(n);
        for (int i = 1; i <= n; i++) {
            int stars = 2 * i - 1;
            int spaces = (width - stars) / 2;
            char[] line = new char[width];
            for (int j = 0; j < width; j++) {
                line[j] = ' ';
            }
            for (int j = 0; j < stars; j++) {
                line[spaces + j] = '*';
            }
            result.add(new String(line));
        }
        return result;
    }

    // Simple display helper
    private static void printTower(List<String> tower) {
        System.out.println("[");
        for (int i = 0; i < tower.size(); i++) {
            System.out.println("  \"" + tower.get(i) + "\"" + (i + 1 < tower.size() ? "," : ""));
        }
        System.out.println("]");
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 3, 5, 6};
        for (int t : tests) {
            System.out.println("Floors: " + t);
            List<String> tower = buildTower(t);
            printTower(tower);
            System.out.println();
        }
    }
}