
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task118 {
    private static final Pattern IPV4_PATTERN = Pattern.compile(
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
    );
    
    private static final Pattern IPV6_PATTERN = Pattern.compile(
        "^([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$|" +
        "^::([0-9a-fA-F]{1,4}:){0,6}[0-9a-fA-F]{1,4}$|" +
        "^([0-9a-fA-F]{1,4}:){1}:([0-9a-fA-F]{1,4}:){0,5}[0-9a-fA-F]{1,4}$|" +
        "^([0-9a-fA-F]{1,4}:){2}:([0-9a-fA-F]{1,4}:){0,4}[0-9a-fA-F]{1,4}$|" +
        "^([0-9a-fA-F]{1,4}:){3}:([0-9a-fA-F]{1,4}:){0,3}[0-9a-fA-F]{1,4}$|" +
        "^([0-9a-fA-F]{1,4}:){4}:([0-9a-fA-F]{1,4}:){0,2}[0-9a-fA-F]{1,4}$|" +
        "^([0-9a-fA-F]{1,4}:){5}:([0-9a-fA-F]{1,4}:){0,1}[0-9a-fA-F]{1,4}$|" +
        "^([0-9a-fA-F]{1,4}:){6}:[0-9a-fA-F]{1,4}$|" +
        "^::$"
    );
    
    public static boolean isValidIPv4(String ip) {
        if (ip == null || ip.isEmpty() || ip.length() > 15) {
            return false;
        }
        return IPV4_PATTERN.matcher(ip).matches();
    }
    
    public static boolean isValidIPv6(String ip) {
        if (ip == null || ip.isEmpty() || ip.length() > 39) {
            return false;
        }
        return IPV6_PATTERN.matcher(ip).matches();
    }
    
    public static String validateIP(String ip) {
        if (ip == null) {
            return "Invalid: null input";
        }
        
        if (ip.isEmpty()) {
            return "Invalid: empty input";
        }
        
        if (ip.length() > 100) {
            return "Invalid: input too long";
        }
        
        String trimmed = ip.trim();
        
        if (isValidIPv4(trimmed)) {
            return "Valid IPv4: " + trimmed;
        } else if (isValidIPv6(trimmed)) {
            return "Valid IPv6: " + trimmed;
        } else {
            return "Invalid IP address: " + trimmed;
        }
    }
    
    public static void main(String[] args) {
        String[] testCases = {
            "192.168.1.1",
            "256.1.1.1",
            "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
            "::1",
            "invalid.ip.address"
        };
        
        for (String testCase : testCases) {
            System.out.println("Input: \\"" + testCase + "\\" -> " + validateIP(testCase));
        }
    }
}
