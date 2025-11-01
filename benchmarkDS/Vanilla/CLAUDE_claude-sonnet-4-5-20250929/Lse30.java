
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse30 {
    public static String processRedirect(String target) {
        // Regular expression: one or more alphanumeric characters or hyphens followed by a period
        String regex = "^[a-zA-Z0-9-]+\\\\..*";
        Pattern pattern = Pattern.compile(regex);
        Matcher matcher = pattern.matcher(target);
        
        if (matcher.matches()) {
            return target;
        } else {
            return "/";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {
            "example.com",
            "test-site.org",
            "my-page.html",
            ".invalid",
            "noperiod"
        };
        
        System.out.println("Java Test Results:");
        for (String testCase : testCases) {
            String result = processRedirect(testCase);
            System.out.println("Input: '" + testCase + "' -> Redirect to: '" + result + "'");
        }
    }
}
