
public class Task58 {
    public static boolean XO(String str) {
        if (str == null) {
            return true;
        }
        
        String lowerStr = str.toLowerCase();
        int xCount = 0;
        int oCount = 0;
        
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
        // Test case 1
        System.out.println("XO(\\"ooxx\\") = " + XO("ooxx")); // Expected: true
        
        // Test case 2
        System.out.println("XO(\\"xooxx\\") = " + XO("xooxx")); // Expected: false
        
        // Test case 3
        System.out.println("XO(\\"ooxXm\\") = " + XO("ooxXm")); // Expected: true
        
        // Test case 4
        System.out.println("XO(\\"zpzpzpp\\") = " + XO("zpzpzpp")); // Expected: true
        
        // Test case 5
        System.out.println("XO(\\"zzoo\\") = " + XO("zzoo")); // Expected: false
    }
}
