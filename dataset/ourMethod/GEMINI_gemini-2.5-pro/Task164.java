import java.util.Arrays;
import java.util.Comparator;

public class Task164 {

    /**
     * Sorts an array of names by the length of their last name, then alphabetically by last name.
     *
     * @param names An array of strings, where each string is a full name.
     * @return A new array containing the sorted names.
     */
    public static String[] lastNameLensort(String[] names) {
        if (names == null) {
            return new String[0];
        }
        
        String[] sortedNames = Arrays.copyOf(names, names.length);

        Comparator<String> lastNameComparator = new Comparator<String>() {
            private String getLastName(String fullName) {
                if (fullName == null || fullName.isEmpty()) {
                    return "";
                }
                int lastSpaceIndex = fullName.lastIndexOf(' ');
                if (lastSpaceIndex == -1) {
                    return fullName;
                }
                return fullName.substring(lastSpaceIndex + 1);
            }

            @Override
            public int compare(String name1, String name2) {
                String lastName1 = getLastName(name1);
                String lastName2 = getLastName(name2);

                int lengthCompare = Integer.compare(lastName1.length(), lastName2.length());
                if (lengthCompare != 0) {
                    return lengthCompare;
                } else {
                    return lastName1.compareTo(lastName2);
                }
            }
        };

        Arrays.sort(sortedNames, lastNameComparator);
        return sortedNames;
    }

    public static void main(String[] args) {
        // Test Case 1
        String[] names1 = {
            "Jennifer Figueroa", "Heather Mcgee", "Amanda Schwartz", "Nicole Yoder", "Melissa Hoffman"
        };
        System.out.println("Test Case 1:");
        System.out.println("Input: " + Arrays.toString(names1));
        System.out.println("Output: " + Arrays.toString(lastNameLensort(names1)));
        System.out.println();

        // Test Case 2: Same last name length
        String[] names2 = {"John Smith", "Jane Doe", "Peter Jones"};
        System.out.println("Test Case 2:");
        System.out.println("Input: " + Arrays.toString(names2));
        System.out.println("Output: " + Arrays.toString(lastNameLensort(names2)));
        System.out.println();

        // Test Case 3: Empty array
        String[] names3 = {};
        System.out.println("Test Case 3:");
        System.out.println("Input: " + Arrays.toString(names3));
        System.out.println("Output: " + Arrays.toString(lastNameLensort(names3)));
        System.out.println();

        // Test Case 4: Single-word names
        String[] names4 = {"Cher", "Madonna", "Prince"};
        System.out.println("Test Case 4:");
        System.out.println("Input: " + Arrays.toString(names4));
        System.out.println("Output: " + Arrays.toString(lastNameLensort(names4)));
        System.out.println();

        // Test Case 5: Names with same last names
        String[] names5 = {"David Lee", "Bruce Lee", "Sara Lee"};
        System.out.println("Test Case 5:");
        System.out.println("Input: " + Arrays.toString(names5));
        System.out.println("Output: " + Arrays.toString(lastNameLensort(names5)));
        System.out.println();
    }
}