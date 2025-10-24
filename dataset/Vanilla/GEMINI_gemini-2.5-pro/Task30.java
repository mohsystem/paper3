import java.util.stream.Collectors;

public class Task30 {

    public static String longest(String s1, String s2) {
        String combined = s1 + s2;
        return combined.chars()
                       .distinct()
                       .sorted()
                       .mapToObj(c -> String.valueOf((char) c))
                       .collect(Collectors.joining());
    }

    public static void main(String[] args) {
        // Test case 1
        String result1 = longest("xyaabbbccccdefww", "xxxxyyyyabklmopq");
        System.out.println("Test 1: " + result1); // Expected: "abcdefklmopqwxy"

        // Test case 2
        String result2 = longest("abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz");
        System.out.println("Test 2: " + result2); // Expected: "abcdefghijklmnopqrstuvwxyz"

        // Test case 3
        String result3 = longest("aretheyhere", "yestheyarehere");
        System.out.println("Test 3: " + result3); // Expected: "aehrsty"
        
        // Test case 4
        String result4 = longest("loopingisfunbutdangerous", "lessdangerousthancoding");
        System.out.println("Test 4: " + result4); // Expected: "abcdefghilnoprstu"

        // Test case 5
        String result5 = longest("inmanylanguages", "theresapairoffunctions");
        System.out.println("Test 5: " + result5); // Expected: "acefghilmnoprstu"
    }
}