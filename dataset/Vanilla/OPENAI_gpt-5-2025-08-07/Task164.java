import java.util.*;

public class Task164 {
    public static String[] lastNameLensort(String[] names) {
        String[] arr = Arrays.copyOf(names, names.length);
        Arrays.sort(arr, (a, b) -> {
            String la = lastToken(a);
            String lb = lastToken(b);
            int laLen = la.length();
            int lbLen = lb.length();
            if (laLen != lbLen) return Integer.compare(laLen, lbLen);
            int cmp = la.compareToIgnoreCase(lb);
            if (cmp != 0) return cmp;
            return a.compareToIgnoreCase(b);
        });
        return arr;
    }

    private static String lastToken(String s) {
        s = s.trim();
        if (s.isEmpty()) return s;
        String[] parts = s.split("\\s+");
        return parts[parts.length - 1];
    }

    private static void printArray(String[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        String[] t1 = {
            "Jennifer Figueroa",
            "Heather Mcgee",
            "Amanda Schwartz",
            "Nicole Yoder",
            "Melissa Hoffman"
        };
        String[] t2 = {
            "John Doe",
            "Alice Roe",
            "Bob Poe",
            "Zed Kay"
        };
        String[] t3 = {
            "Anna Smith",
            "Brian Smith",
            "Aaron Smith",
            "Zoe Adams"
        };
        String[] t4 = {
            "Al A",
            "Bea Bee",
            "Ce Ce",
            "Dee Eee",
            "Eff Eff"
        };
        String[] t5 = {
            "Mary Ann Van Dyke",
            "Jean-Claude Van Damme",
            "Jo Van",
            "Li Wei",
            "X AE A-12 Musk"
        };

        printArray(lastNameLensort(t1));
        printArray(lastNameLensort(t2));
        printArray(lastNameLensort(t3));
        printArray(lastNameLensort(t4));
        printArray(lastNameLensort(t5));
    }
}