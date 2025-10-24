public class Task58 {

    /**
     * Checks if a string has the same amount of 'x's and 'o's, case-insensitive.
     *
     * @param str The string to check. Can contain any character.
     * @return true if the count of 'x's and 'o's are equal, false otherwise.
     */
    public static boolean XO(String str) {
        if (str == null) {
            return true;
        }

        int xCount = 0;
        int oCount = 0;
        
        // Using str.toLowerCase() is a simple way to handle case-insensitivity.
        String lowerCaseStr = str.toLowerCase();

        for (int i = 0; i < lowerCaseStr.length(); i++) {
            char c = lowerCaseStr.charAt(i);
            if (c == 'x') {
                xCount++;
            } else if (c == 'o') {
                oCount++;
            }
        }
        
        return xCount == oCount;
    }

    public static void main(String[] args) {
        // Test Case 1: "ooxx" -> true
        System.out.println("Test 1 \"ooxx\": " + XO("ooxx"));
        // Test Case 2: "xooxx" -> false
        System.out.println("Test 2 \"xooxx\": " + XO("xooxx"));
        // Test Case 3: "ooxXm" -> true
        System.out.println("Test 3 \"ooxXm\": " + XO("ooxXm"));
        // Test Case 4: "zpzpzpp" -> true
        System.out.println("Test 4 \"zpzpzpp\": " + XO("zpzpzpp"));
        // Test Case 5: "zzoo" -> false
        System.out.println("Test 5 \"zzoo\": " + XO("zzoo"));
    }
}