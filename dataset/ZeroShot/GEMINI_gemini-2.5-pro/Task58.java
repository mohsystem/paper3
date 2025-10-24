public class Task58 {
    /**
     * Checks if a string has the same amount of 'x's and 'o's, case-insensitively.
     *
     * @param str The string to check. The string can contain any character.
     * @return true if the count of 'x's and 'o's is equal, false otherwise.
     */
    public static boolean XO(String str) {
        if (str == null) {
            return true; // No 'x's or 'o's in a null string
        }
        
        int xCount = 0;
        int oCount = 0;
        
        // Loop through each character of the string
        for (int i = 0; i < str.length(); i++) {
            char c = Character.toLowerCase(str.charAt(i));
            if (c == 'x') {
                xCount++;
            } else if (c == 'o') {
                oCount++;
            }
        }
        
        return xCount == oCount;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("XO(\"ooxx\") => " + XO("ooxx"));          // Expected: true
        System.out.println("XO(\"xooxx\") => " + XO("xooxx"));        // Expected: false
        System.out.println("XO(\"ooxXm\") => " + XO("ooxXm"));        // Expected: true
        System.out.println("XO(\"zpzpzpp\") => " + XO("zpzpzpp"));    // Expected: true
        System.out.println("XO(\"zzoo\") => " + XO("zzoo"));          // Expected: false
    }
}