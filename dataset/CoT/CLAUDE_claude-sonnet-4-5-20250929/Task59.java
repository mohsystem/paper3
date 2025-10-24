
public class Task59 {
    public static String accum(String s) {
        if (s == null || s.isEmpty()) {
            return "";
        }
        
        StringBuilder result = new StringBuilder();
        
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            
            // Validate input - only letters allowed
            if (!Character.isLetter(c)) {
                continue;
            }
            
            // Add separator if not first element
            if (i > 0) {
                result.append("-");
            }
            
            // First character uppercase
            result.append(Character.toUpperCase(c));
            
            // Rest lowercase, repeated i times
            for (int j = 0; j < i; j++) {
                result.append(Character.toLowerCase(c));
            }
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: " + accum("abcd"));
        // Expected: A-Bb-Ccc-Dddd
        
        // Test case 2
        System.out.println("Test 2: " + accum("RqaEzty"));
        // Expected: R-Qq-Aaa-Eeee-Zzzzz-Tttttt-Yyyyyyy
        
        // Test case 3
        System.out.println("Test 3: " + accum("cwAt"));
        // Expected: C-Ww-Aaa-Tttt
        
        // Test case 4
        System.out.println("Test 4: " + accum("ZpglnRxqenU"));
        // Expected: Z-Pp-Ggg-Llll-Nnnnn-Rrrrrr-Xxxxxxx-Qqqqqqqq-Eeeeeeeee-Nnnnnnnnnn-Uuuuuuuuuuu
        
        // Test case 5
        System.out.println("Test 5: " + accum("a"));
        // Expected: A
    }
}
