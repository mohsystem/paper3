import java.util.*;
import java.nio.charset.StandardCharsets;

public class Task164 {

    public static String[] lastNameLensort(String[] names) {
        if (names == null) {
            return new String[0];
        }
        String[] copy = Arrays.copyOf(names, names.length);
        Arrays.sort(copy, new Comparator<String>() {
            private String safe(String s) { return s == null ? "" : s; }

            private String extractLastName(String name) {
                if (name == null) return "";
                int end = name.length() - 1;
                while (end >= 0 && Character.isWhitespace(name.charAt(end))) end--;
                if (end < 0) return "";
                int i = end;
                while (i >= 0 && !Character.isWhitespace(name.charAt(i))) i--;
                int start = i + 1;
                return name.substring(start, end + 1);
            }

            private int codePointLength(String s) {
                return s.codePointCount(0, s.length());
            }

            @Override
            public int compare(String a, String b) {
                String la = extractLastName(a);
                String lb = extractLastName(b);
                int lenCmp = Integer.compare(codePointLength(la), codePointLength(lb));
                if (lenCmp != 0) return lenCmp;
                int lastCmp = la.compareToIgnoreCase(lb);
                if (lastCmp != 0) return lastCmp;
                String aa = safe(a);
                String bb = safe(b);
                return aa.compareToIgnoreCase(bb);
            }
        });
        return copy;
    }

    private static void printArray(String[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        // Test case 1 (given example)
        String[] t1 = {
            "Jennifer Figueroa",
            "Heather Mcgee",
            "Amanda Schwartz",
            "Nicole Yoder",
            "Melissa Hoffman"
        };
        printArray(lastNameLensort(t1));

        // Test case 2 (all same last-name length -> alphabetical by last name)
        String[] t2 = {"Anna Zed", "Bob Kay", "Cara May", "Dan Ray"};
        printArray(lastNameLensort(t2));

        // Test case 3 (extra spaces and punctuation)
        String[] t3 = {"  John   Doe  ", "Alice   Smith", "Bob  O'Niel"};
        printArray(lastNameLensort(t3));

        // Test case 4 (same last name -> fall back to full name alphabetical)
        String[] t4 = {"Cindy Lee", "Anna Lee", "Brian Lee"};
        printArray(lastNameLensort(t4));

        // Test case 5 (edge cases: empty, null, single token)
        String[] t5 = {"", null, "Single", "Mary Ann", "  "};
        printArray(lastNameLensort(t5));
    }
}