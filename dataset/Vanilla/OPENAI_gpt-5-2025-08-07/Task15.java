import java.util.ArrayList;
import java.util.List;

public class Task15 {
    public static List<String> buildTower(int n) {
        List<String> result = new ArrayList<>();
        if (n <= 0) return result;
        int width = 2 * n - 1;
        for (int i = 1; i <= n; i++) {
            int stars = 2 * i - 1;
            int spaces = n - i;
            StringBuilder sb = new StringBuilder(width);
            for (int s = 0; s < spaces; s++) sb.append(' ');
            for (int st = 0; st < stars; st++) sb.append('*');
            for (int s = 0; s < spaces; s++) sb.append(' ');
            result.add(sb.toString());
        }
        return result;
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 3, 4, 6};
        for (int t : tests) {
            System.out.println("Tower with " + t + " floors:");
            List<String> tower = buildTower(t);
            for (String row : tower) {
                System.out.println(row);
            }
            System.out.println();
        }
    }
}