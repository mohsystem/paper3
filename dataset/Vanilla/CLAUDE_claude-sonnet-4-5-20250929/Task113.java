
import java.util.Scanner;

public class Task113 {
    public static String concatenateStrings(String[] strings) {
        StringBuilder result = new StringBuilder();
        for (String str : strings) {
            result.append(str);
        }
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1
        String[] test1 = {"Hello", " ", "World"};
        System.out.println("Test 1: " + concatenateStrings(test1));
        
        // Test case 2
        String[] test2 = {"Java", "Programming", "Language"};
        System.out.println("Test 2: " + concatenateStrings(test2));
        
        // Test case 3
        String[] test3 = {"One", "Two", "Three", "Four"};
        System.out.println("Test 3: " + concatenateStrings(test3));
        
        // Test case 4
        String[] test4 = {"", "Empty", "", "Strings"};
        System.out.println("Test 4: " + concatenateStrings(test4));
        
        // Test case 5
        String[] test5 = {"Single"};
        System.out.println("Test 5: " + concatenateStrings(test5));
    }
}
