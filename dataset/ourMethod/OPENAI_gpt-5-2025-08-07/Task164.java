import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.Locale;

public class Task164 {

    public static String[] lastNameLensort(String[] names) {
        if (names == null) {
            throw new IllegalArgumentException("Input array cannot be null.");
        }
        // Validate inputs and prepare a list with indices to maintain stability if needed
        List<IndexedName> list = new ArrayList<>(names.length);
        for (int i = 0; i < names.length; i++) {
            String s = names[i];
            if (s == null) {
                throw new IllegalArgumentException("Name at index " + i + " is null.");
            }
            String last = extractLastName(s);
            list.add(new IndexedName(s, last.toLowerCase(Locale.ROOT), last.length(), i));
        }

        list.sort(Comparator
                .comparingInt((IndexedName in) -> in.lastLen)
                .thenComparing(in -> in.lastLower)
                .thenComparingInt(in -> in.index));

        String[] result = new String[names.length];
        for (int i = 0; i < list.size(); i++) {
            result[i] = list.get(i).original;
        }
        return result;
    }

    private static String extractLastName(String s) {
        String t = s.trim();
        int lastSpace = t.lastIndexOf(' ');
        if (lastSpace <= 0 || lastSpace >= t.length() - 1) {
            throw new IllegalArgumentException("Invalid name format (must contain first and last name): " + s);
        }
        return t.substring(lastSpace + 1);
    }

    private static final class IndexedName {
        final String original;
        final String lastLower;
        final int lastLen;
        final int index;
        IndexedName(String original, String lastLower, int lastLen, int index) {
            this.original = original;
            this.lastLower = lastLower;
            this.lastLen = lastLen;
            this.index = index;
        }
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
                {
                        "Jennifer Figueroa",
                        "Heather Mcgee",
                        "Amanda Schwartz",
                        "Nicole Yoder",
                        "Melissa Hoffman"
                },
                {
                        "Jim Bo",
                        "Ana Li",
                        "Zoe Wu",
                        "Tom Ng",
                        "Foo Bar"
                },
                {
                        "Mary Jane Watson",
                        "Jean Claude Van Damme",
                        "Peter Parker",
                        "Tony Stark",
                        "Bruce Wayne"
                },
                {
                        "  Alice   Smith  ",
                        "bob a",
                        "CARL B",
                        "dave  aa",
                        "Eve   Z"
                },
                {
                        "Alice Hope",
                        "Bob Hope",
                        "Carl Hope",
                        "Ann Hope",
                        "Zed Hope"
                }
        };

        for (int i = 0; i < tests.length; i++) {
            String[] res = lastNameLensort(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + Arrays.toString(res));
        }
    }
}