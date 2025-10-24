import java.util.Arrays;
import java.util.Comparator;

public class Task164 {

    /**
     * Sorts an array of names based on the length of the last name, then alphabetically by last name.
     *
     * @param names An array of strings, where each string is a full name.
     * @return A new array with the names sorted according to the specified criteria.
     */
    public static String[] lastNameLensort(String[] names) {
        if (names == null) {
            return new String[0];
        }
        String[] sortedNames = Arrays.copyOf(names, names.length);
        
        Comparator<String> lastNameComparator = (s1, s2) -> {
            // substring with lastIndexOf(' ') + 1 correctly handles names with no spaces
            String lastName1 = s1.substring(s1.lastIndexOf(' ') + 1);
            String lastName2 = s2.substring(s2.lastIndexOf(' ') + 1);

            int lenDiff = lastName1.length() - lastName2.length();
            if (lenDiff != 0) {
                return lenDiff;
            } else {
                return lastName1.compareTo(lastName2);
            }
        };
        
        Arrays.sort(sortedNames, lastNameComparator);
        return sortedNames;
    }

    public static void main(String[] args) {
        String[][] testCases = {
            {
                "Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz",
                "Nicole Yoder", "Melissa Hoffman"
            },
            {
                "John Smith", "Jane Doe", "Peter Jones", "Mary Johnson"
            },
            {
                "alpha beta", "gamma delta"
            },
            {
                "A B", "C D", "E F"
            },
            {
                "SingleName", "Another SingleName"
            }
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Original: " + Arrays.toString(testCases[i]));
            String[] result = lastNameLensort(testCases[i]);
            System.out.println("Sorted:   " + Arrays.toString(result));
            System.out.println();
        }
    }
}