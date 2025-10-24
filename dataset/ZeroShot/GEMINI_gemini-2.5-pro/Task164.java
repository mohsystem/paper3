import java.util.Arrays;
import java.util.Comparator;

public class Task164 {

    /**
     * Extracts the last name from a full name string.
     * Handles null, empty, and whitespace-only strings, as well as names without spaces.
     * @param fullName The full name string (e.g., "John Doe").
     * @return The last name.
     */
    private static String getLastName(String fullName) {
        if (fullName == null || fullName.trim().isEmpty()) {
            return "";
        }
        String trimmedName = fullName.trim();
        int lastSpaceIndex = trimmedName.lastIndexOf(' ');
        if (lastSpaceIndex == -1) {
            return trimmedName; // No space, the whole name is the last name
        }
        return trimmedName.substring(lastSpaceIndex + 1);
    }

    /**
     * Sorts an array of names by the length of their last names, then alphabetically.
     * @param names An array of names in "First Last" format.
     * @return A new array with the names sorted.
     */
    public static String[] lastNameLensort(String[] names) {
        if (names == null) {
            return new String[0];
        }
        // Create a copy to avoid modifying the original array and for security
        String[] namesCopy = Arrays.copyOf(names, names.length);
        
        Comparator<String> nameComparator = (name1, name2) -> {
            String lastName1 = getLastName(name1);
            String lastName2 = getLastName(name2);

            int lengthCompare = Integer.compare(lastName1.length(), lastName2.length());
            if (lengthCompare != 0) {
                return lengthCompare;
            } else {
                return lastName1.compareTo(lastName2);
            }
        };

        Arrays.sort(namesCopy, nameComparator);
        return namesCopy;
    }

    public static void main(String[] args) {
        // Test Case 1: Example from prompt
        String[] test1 = {
            "Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz", "Nicole Yoder", "Melissa Hoffman"
        };
        System.out.println("Test Case 1:");
        System.out.println("Input: " + Arrays.toString(test1));
        System.out.println("Output: " + Arrays.toString(lastNameLensort(test1)));
        System.out.println();

        // Test Case 2: Last names with same length
        String[] test2 = {"Steven King", "John Smith", "Jane Doe"};
        System.out.println("Test Case 2:");
        System.out.println("Input: " + Arrays.toString(test2));
        System.out.println("Output: " + Arrays.toString(lastNameLensort(test2)));
        System.out.println();

        // Test Case 3: Names without spaces
        String[] test3 = {"Beyonce", "Adele", "Cher"};
        System.out.println("Test Case 3:");
        System.out.println("Input: " + Arrays.toString(test3));
        System.out.println("Output: " + Arrays.toString(lastNameLensort(test3)));
        System.out.println();

        // Test Case 4: Mixed names (with and without spaces)
        String[] test4 = {"David Copperfield", "David Blaine", "Penn"};
        System.out.println("Test Case 4:");
        System.out.println("Input: " + Arrays.toString(test4));
        System.out.println("Output: " + Arrays.toString(lastNameLensort(test4)));
        System.out.println();
        
        // Test Case 5: Edge cases (empty array and empty/malformed strings)
        String[] test5 = {"", " ", "Single", "Peter Pan", null};
        System.out.println("Test Case 5:");
        System.out.println("Input: " + Arrays.toString(test5));
        System.out.println("Output: " + Arrays.toString(lastNameLensort(test5)));
        System.out.println();
    }
}