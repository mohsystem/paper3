
public class Task58 {
    public static boolean XO(String str) {
        if (str == null) {
            return true;
        }
        
        String lowerStr = str.toLowerCase();
        int countX = 0;
        int countO = 0;
        
        for (int i = 0; i < lowerStr.length(); i++) {
            char ch = lowerStr.charAt(i);
            if (ch == 'x') {
                countX++;
            } else if (ch == 'o') {
                countO++;
            }
        }
        
        return countX == countO;
    }
    
    public static void main(String[] args) {
        System.out.println("Test 1: XO(\\"ooxx\\") = " + XO("ooxx") + " (expected: true)");
        System.out.println("Test 2: XO(\\"xooxx\\") = " + XO("xooxx") + " (expected: false)");
        System.out.println("Test 3: XO(\\"ooxXm\\") = " + XO("ooxXm") + " (expected: true)");
        System.out.println("Test 4: XO(\\"zpzpzpp\\") = " + XO("zpzpzpp") + " (expected: true)");
        System.out.println("Test 5: XO(\\"zzoo\\") = " + XO("zzoo") + " (expected: false)");
    }
}
