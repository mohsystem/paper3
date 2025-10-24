
import java.util.*;

public class Task164 {
    public static String[] lastNameLensort(String[] names) {
        if (names == null) {
            return new String[0];
        }
        
        // Create a copy to avoid modifying original array
        String[] result = Arrays.copyOf(names, names.length);
        
        Arrays.sort(result, new Comparator<String>() {
            @Override
            public int compare(String name1, String name2) {
                if (name1 == null || name2 == null) {
                    return name1 == null ? (name2 == null ? 0 : -1) : 1;
                }
                
                // Extract last names safely
                String lastName1 = extractLastName(name1);
                String lastName2 = extractLastName(name2);
                
                // Compare by length first
                int lengthDiff = lastName1.length() - lastName2.length();
                if (lengthDiff != 0) {
                    return lengthDiff;
                }
                
                // If lengths are equal, compare alphabetically
                return lastName1.compareTo(lastName2);
            }
            
            private String extractLastName(String fullName) {
                if (fullName == null || fullName.trim().isEmpty()) {
                    return "";
                }
                String trimmed = fullName.trim();
                int lastSpace = trimmed.lastIndexOf(' ');
                if (lastSpace == -1) {
                    return trimmed;
                }
                return trimmed.substring(lastSpace + 1);
            }
        });
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        String[] test1 = {
            "Jennifer Figueroa",
            "Heather Mcgee",
            "Amanda Schwartz",
            "Nicole Yoder",
            "Melissa Hoffman"
        };
        System.out.println("Test 1: " + Arrays.toString(lastNameLensort(test1)));
        
        // Test case 2
        String[] test2 = {"John Doe", "Jane Smith", "Bob Lee"};
        System.out.println("Test 2: " + Arrays.toString(lastNameLensort(test2)));
        
        // Test case 3
        String[] test3 = {"Alice Brown", "Charlie Brown", "David Green"};
        System.out.println("Test 3: " + Arrays.toString(lastNameLensort(test3)));
        
        // Test case 4 - Empty array
        String[] test4 = {};
        System.out.println("Test 4: " + Arrays.toString(lastNameLensort(test4)));
        
        // Test case 5 - Single name
        String[] test5 = {"Michael Jackson"};
        System.out.println("Test 5: " + Arrays.toString(lastNameLensort(test5)));
    }
}
