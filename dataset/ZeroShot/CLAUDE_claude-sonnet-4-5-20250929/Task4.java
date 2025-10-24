
public class Task4 {
    public static boolean isIsogram(String str) {
        if (str == null || str.isEmpty()) {
            return true;
        }
        
        String lowerStr = str.toLowerCase();
        
        for (int i = 0; i < lowerStr.length(); i++) {
            for (int j = i + 1; j < lowerStr.length(); j++) {
                if (lowerStr.charAt(i) == lowerStr.charAt(j)) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println(isIsogram("Dermatoglyphics")); // true
        System.out.println(isIsogram("aba"));              // false
        System.out.println(isIsogram("moOse"));            // false
        System.out.println(isIsogram(""));                 // true
        System.out.println(isIsogram("isogram"));          // false
    }
}
