import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Task15 {
    public static List<String> buildTower(int n) {
        if (n <= 0) return Collections.emptyList();
        List<String> result = new ArrayList<>(n);
        int width = 2 * n - 1;
        for (int i = 1; i <= n; i++) {
            int stars = 2 * i - 1;
            int spaces = (width - stars) / 2;
            StringBuilder sb = new StringBuilder(width);
            for (int s = 0; s < spaces; s++) sb.append(' ');
            for (int st = 0; st < stars; st++) sb.append('*');
            for (int s = 0; s < spaces; s++) sb.append(' ');
            result.add(sb.toString());
        }
        return result;
    }

    private static void printTower(List<String> tower) {
        for (String line : tower) {
            System.out.println(line);
        }
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 3, 6, 0};
        for (int n : tests) {
            System.out.println("n = " + n);
            printTower(buildTower(n));
            System.out.println();
        }
    }
}