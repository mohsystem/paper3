import java.util.Arrays;
import java.util.Comparator;

public class Task164 {
    /**
     * Sorts an array of names based on the length of the last name, then alphabetically by last name.
     * @param names An array of strings, where each string is a name in "First Last" format.
     * @return The sorted array of names.
     */
    public static String[] lastNameLensort(String[] names) {
        Arrays.sort(names, (name1, name2) -> {
            String lastName1 = name1.substring(name1.lastIndexOf(' ') + 1);
            String lastName2 = name2.substring(name2.lastIndexOf(' ') + 1);

            int lenCompare = Integer.compare(lastName1.length(), lastName2.length());
            if (lenCompare != 0) {
                return lenCompare;
            } else {
                return lastName1.compareTo(lastName2);
            }
        });
        return names;
    }

    public static void main(String[] args) {
        String[][] testCases = {
            {"Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz", "Nicole Yoder", "Melissa Hoffman"},
            {"A B", "C D", "E F"},
            {"Morty Smith", "Rick Sanchez", "Jerry Smith"},
            {"Peter Parker", "Clark Kent", "Bruce Wayne"},
            {"Leo Tolstoy", "Jane Austen", "Charles Dickens"}
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            String[] testData = testCases[i].clone(); // Use a clone to preserve the original test case
            System.out.println("Input: " + Arrays.toString(testData));
            String[] result = lastNameLensort(testData);
            System.out.println("Output: " + Arrays.toString(result));
            System.out.println();
        }
    }
}