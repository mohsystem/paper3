import java.util.*;

public class Task164 {
    // Extracts the last name from a full name string
    private static String extractLastName(String name) {
        if (name == null) return "";
        String trimmed = name.trim();
        int lastSpace = trimmed.lastIndexOf(' ');
        if (lastSpace == -1) {
            return trimmed; // fallback if no space found
        }
        return trimmed.substring(lastSpace + 1);
    }

    // Returns a new array sorted by last name length, then alphabetically by last name (case-insensitive)
    public static String[] lastNameLensort(String[] names) {
        if (names == null) return new String[0];
        String[] result = Arrays.copyOf(names, names.length);
        // Stable sort to preserve order when last names are exactly equal
        Arrays.sort(result, new Comparator<String>() {
            @Override
            public int compare(String a, String b) {
                String la = extractLastName(a);
                String lb = extractLastName(b);
                int lenDiff = Integer.compare(la.length(), lb.length());
                if (lenDiff != 0) return lenDiff;
                int ci = la.compareToIgnoreCase(lb);
                if (ci != 0) return ci;
                // If equal ignoring case, compare with case to ensure deterministic ordering
                return la.compareTo(lb);
            }
        });
        return result;
    }

    private static void printArray(String[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        // Test case 1: Provided example
        String[] t1 = {
            "Jennifer Figueroa",
            "Heather Mcgee",
            "Amanda Schwartz",
            "Nicole Yoder",
            "Melissa Hoffman"
        };
        printArray(lastNameLensort(t1));

        // Test case 2: Equal last name lengths, alphabetical by last name
        String[] t2 = {
            "John Smith",
            "Amy Brown",
            "Zoey Clark",
            "Evan White",
            "Liam Jones"
        };
        printArray(lastNameLensort(t2));

        // Test case 3: Multiple spaces and multi-word names, last token as last name
        String[] t3 = {
            "Alice   van Helsing",
            "Bob de la Cruz",
            "  Carla   Delaney ",
            "Derek O'Neill",
            "Eve   Stone  "
        };
        printArray(lastNameLensort(t3));

        // Test case 4: Single element
        String[] t4 = { "Solo Person" };
        printArray(lastNameLensort(t4));

        // Test case 5: Identical last names to check stability
        String[] t5 = {
            "Ann Lee",
            "Ben Lee",
            "Cara Zee",
            "Dan Lee",
            "Eli Zee"
        };
        printArray(lastNameLensort(t5));
    }
}