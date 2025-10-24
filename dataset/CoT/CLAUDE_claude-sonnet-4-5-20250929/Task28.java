
public class Task28 {
    public static String whoLikesIt(String[] names) {
        if (names == null) {
            return "no one likes this";
        }
        
        int length = names.length;
        
        switch (length) {
            case 0:
                return "no one likes this";
            case 1:
                return names[0] + " likes this";
            case 2:
                return names[0] + " and " + names[1] + " like this";
            case 3:
                return names[0] + ", " + names[1] + " and " + names[2] + " like this";
            default:
                return names[0] + ", " + names[1] + " and " + (length - 2) + " others like this";
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(whoLikesIt(new String[]{}));
        
        // Test case 2
        System.out.println(whoLikesIt(new String[]{"Peter"}));
        
        // Test case 3
        System.out.println(whoLikesIt(new String[]{"Jacob", "Alex"}));
        
        // Test case 4
        System.out.println(whoLikesIt(new String[]{"Max", "John", "Mark"}));
        
        // Test case 5
        System.out.println(whoLikesIt(new String[]{"Alex", "Jacob", "Mark", "Max"}));
    }
}
