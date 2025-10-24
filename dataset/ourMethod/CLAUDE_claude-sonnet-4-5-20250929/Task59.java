
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class Task59 {
    public static String accum(String s) {
        if (s == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        if (s.length() > 10000) {
            throw new IllegalArgumentException("Input string too long");
        }
        
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
                throw new IllegalArgumentException("Input must contain only letters a-z and A-Z");
            }
        }
        
        List<String> parts = new ArrayList<>();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            char upper = Character.toUpperCase(c);
            char lower = Character.toLowerCase(c);
            
            StringBuilder part = new StringBuilder();
            part.append(upper);
            for (int j = 0; j < i; j++) {
                part.append(lower);
            }
            parts.add(part.toString());
        }
        
        return String.join("-", parts);
    }
    
    public static void main(String[] args) {
        System.out.println(accum("abcd"));
        System.out.println(accum("RqaEzty"));
        System.out.println(accum("cwAt"));
        System.out.println(accum("ZpglnRxqenU"));
        System.out.println(accum("NyffsGeyylB"));
    }
}
