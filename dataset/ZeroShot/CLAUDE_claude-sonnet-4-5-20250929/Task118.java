
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task118 {
    public static boolean isValidIPv4(String ip) {
        if (ip == null || ip.isEmpty()) {
            return false;
        }
        
        String ipv4Pattern = "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";
        Pattern pattern = Pattern.compile(ipv4Pattern);
        Matcher matcher = pattern.matcher(ip);
        return matcher.matches();
    }
    
    public static boolean isValidIPv6(String ip) {
        if (ip == null || ip.isEmpty()) {
            return false;
        }
        
        String ipv6Pattern = "^([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$";
        String ipv6CompressedPattern = "^(([0-9a-fA-F]{1,4}:){0,7}[0-9a-fA-F]{0,4})?::([0-9a-fA-F]{1,4}:){0,7}[0-9a-fA-F]{0,4}$";
        
        Pattern pattern1 = Pattern.compile(ipv6Pattern);
        Pattern pattern2 = Pattern.compile(ipv6CompressedPattern);
        
        return pattern1.matcher(ip).matches() || pattern2.matcher(ip).matches();
    }
    
    public static String validateIP(String ip) {
        if (isValidIPv4(ip)) {
            return "Valid IPv4";
        } else if (isValidIPv6(ip)) {
            return "Valid IPv6";
        } else {
            return "Invalid IP";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {
            "192.168.1.1",
            "256.1.1.1",
            "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
            "::1",
            "invalid.ip.address"
        };
        
        System.out.println("IP Address Validation Results:");
        System.out.println("================================");
        for (String testCase : testCases) {
            System.out.println("IP: " + testCase);
            System.out.println("Result: " + validateIP(testCase));
            System.out.println("--------------------------------");
        }
    }
}
