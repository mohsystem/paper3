
import java.util.*;

public class Task164 {
    public static String[] lastNameLensort(String[] names) {
        if (names == null || names.length == 0) {
            return names;
        }
        
        Arrays.sort(names, (a, b) -> {
            String lastNameA = a.substring(a.lastIndexOf(' ') + 1);
            String lastNameB = b.substring(b.lastIndexOf(' ') + 1);
            
            int lenDiff = lastNameA.length() - lastNameB.length();
            if (lenDiff != 0) {
                return lenDiff;
            }
            return lastNameA.compareTo(lastNameB);
        });
        
        return names;
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
        String[] test2 = {
            "John Smith",
            "Jane Doe",
            "Bob Lee"
        };
        System.out.println("Test 2: " + Arrays.toString(lastNameLensort(test2)));
        
        // Test case 3
        String[] test3 = {
            "Alice Johnson",
            "Charlie Brown",
            "David Anderson"
        };
        System.out.println("Test 3: " + Arrays.toString(lastNameLensort(test3)));
        
        // Test case 4
        String[] test4 = {
            "Emily White",
            "Michael Black",
            "Sarah Green"
        };
        System.out.println("Test 4: " + Arrays.toString(lastNameLensort(test4)));
        
        // Test case 5
        String[] test5 = {
            "Tom Martinez",
            "Lisa Garcia",
            "Kevin Rodriguez"
        };
        System.out.println("Test 5: " + Arrays.toString(lastNameLensort(test5)));
    }
}
