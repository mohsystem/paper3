public class Task58 {

    public static boolean XO(String str) {
        if (str == null) {
            return true;
        }

        int xCount = 0;
        int oCount = 0;
        
        // Convert the string to lower case to make the comparison case-insensitive
        String lowerStr = str.toLowerCase();

        for (int i = 0; i < lowerStr.length(); i++) {
            char c = lowerStr.charAt(i);
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
        System.out.println("ooxx -> " + XO("ooxx"));          // Expected: true
        System.out.println("xooxx -> " + XO("xooxx"));        // Expected: false
        System.out.println("ooxXm -> " + XO("ooxXm"));        // Expected: true
        System.out.println("zpzpzpp -> " + XO("zpzpzpp"));    // Expected: true
        System.out.println("zzoo -> " + XO("zzoo"));          // Expected: false
    }
}