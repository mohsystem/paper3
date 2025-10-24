
import java.util.*;

public class Task164 {
    public static String[] lastNameLensort(String[] names) {
        if (names == null || names.length == 0) {
            return names;
        }
        
        String[] result = names.clone();
        Arrays.sort(result, (name1, name2) -> {
            String lastName1 = name1.substring(name1.lastIndexOf(' ') + 1);
            String lastName2 = name2.substring(name2.lastIndexOf(' ') + 1);
            
            int lengthComparison = Integer.compare(lastName1.length(), lastName2.length());
            if (lengthComparison != 0) {
                return lengthComparison;
            }
            return lastName1.compareTo(lastName2);
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
        String[] test2 = {
            "John Doe",
            "Jane Smith",
            "Bob Lee"
        };
        System.out.println("Test 2: " + Arrays.toString(lastNameLensort(test2)));
        
        // Test case 3
        String[] test3 = {
            "Alice Johnson",
            "Charlie Brown",
            "David Green"
        };
        System.out.println("Test 3: " + Arrays.toString(lastNameLensort(test3)));
        
        // Test case 4
        String[] test4 = {
            "Emily White",
            "Frank Black",
            "Grace Blue"
        };
        System.out.println("Test 4: " + Arrays.toString(lastNameLensort(test4)));
        
        // Test case 5
        String[] test5 = {
            "Michael Anderson",
            "Sarah Williams",
            "Tom Martinez"
        };
        System.out.println("Test 5: " + Arrays.toString(lastNameLensort(test5)));
    }
}
