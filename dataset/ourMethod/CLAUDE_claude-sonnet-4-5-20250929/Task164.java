
import java.util.*;

public class Task164 {
    public static List<String> lastNameLensort(List<String> names) {
        if (names == null) {
            return new ArrayList<>();
        }
        
        List<String> result = new ArrayList<>();
        for (String name : names) {
            if (name != null && !name.trim().isEmpty()) {
                result.add(name);
            }
        }
        
        result.sort((name1, name2) -> {
            String lastName1 = extractLastName(name1);
            String lastName2 = extractLastName(name2);
            
            int lenDiff = lastName1.length() - lastName2.length();
            if (lenDiff != 0) {
                return lenDiff;
            }
            return lastName1.compareTo(lastName2);
        });
        
        return result;
    }
    
    private static String extractLastName(String fullName) {
        if (fullName == null || fullName.trim().isEmpty()) {
            return "";
        }
        String trimmed = fullName.trim();
        int lastSpace = trimmed.lastIndexOf(' ');
        if (lastSpace > 0 && lastSpace < trimmed.length() - 1) {
            return trimmed.substring(lastSpace + 1);
        }
        return trimmed;
    }
    
    public static void main(String[] args) {
        // Test case 1
        List<String> test1 = Arrays.asList(
            "Jennifer Figueroa",
            "Heather Mcgee",
            "Amanda Schwartz",
            "Nicole Yoder",
            "Melissa Hoffman"
        );
        System.out.println("Test 1: " + lastNameLensort(test1));
        
        // Test case 2
        List<String> test2 = Arrays.asList(
            "John Doe",
            "Jane Smith",
            "Bob Lee"
        );
        System.out.println("Test 2: " + lastNameLensort(test2));
        
        // Test case 3
        List<String> test3 = Arrays.asList(
            "Alice Anderson",
            "Charlie Brown",
            "David Clark"
        );
        System.out.println("Test 3: " + lastNameLensort(test3));
        
        // Test case 4
        List<String> test4 = Arrays.asList(
            "Emma Wilson",
            "Olivia Moore"
        );
        System.out.println("Test 4: " + lastNameLensort(test4));
        
        // Test case 5
        List<String> test5 = Arrays.asList(
            "Michael Johnson",
            "Sarah Williams",
            "James Davis"
        );
        System.out.println("Test 5: " + lastNameLensort(test5));
    }
}
