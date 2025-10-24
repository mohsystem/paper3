
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task118 {
    public static boolean isValidIPv4(String ip) {
        if (ip == null || ip.isEmpty()) {
            return false;
        }
        
        String[] parts = ip.split("\\\\.");
        if (parts.length != 4) {
            return false;
        }
        
        for (String part : parts) {
            if (part.isEmpty() || part.length() > 3) {
                return false;
            }
            
            // Check for leading zeros
            if (part.length() > 1 && part.charAt(0) == '0') {
                return false;
            }
            
            try {
                int num = Integer.parseInt(part);
                if (num < 0 || num > 255) {
                    return false;
                }
            } catch (NumberFormatException e) {
                return false;
            }
        }
        return true;
    }
    
    public static boolean isValidIPv6(String ip) {
        if (ip == null || ip.isEmpty()) {
            return false;
        }
        
        String[] parts = ip.split(":");
        if (parts.length > 8 || parts.length < 3) {
            return false;
        }
        
        boolean hasDoubleColon = ip.contains("::");
        if (hasDoubleColon) {
            if (ip.indexOf("::") != ip.lastIndexOf("::")) {
                return false; // More than one "::"
            }
        }
        
        for (String part : parts) {
            if (part.isEmpty() && hasDoubleColon) {
                continue;
            }
            if (part.length() > 4) {
                return false;
            }
            if (!part.matches("[0-9a-fA-F]*")) {
                return false;
            }
        }
        return true;
    }
    
    public static String getIPVersion(String ip) {
        if (isValidIPv4(ip)) {
            return "IPv4";
        } else if (isValidIPv6(ip)) {
            return "IPv6";
        } else {
            return "Invalid";
        }
    }
    
    public static String normalizeIPv4(String ip) {
        if (!isValidIPv4(ip)) {
            return "Invalid IP";
        }
        return ip;
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid IPv4
        String test1 = "192.168.1.1";
        System.out.println("Test 1: " + test1);
        System.out.println("Valid IPv4: " + isValidIPv4(test1));
        System.out.println("IP Version: " + getIPVersion(test1));
        System.out.println();
        
        // Test case 2: Invalid IPv4 (out of range)
        String test2 = "256.168.1.1";
        System.out.println("Test 2: " + test2);
        System.out.println("Valid IPv4: " + isValidIPv4(test2));
        System.out.println("IP Version: " + getIPVersion(test2));
        System.out.println();
        
        // Test case 3: Valid IPv6
        String test3 = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
        System.out.println("Test 3: " + test3);
        System.out.println("Valid IPv6: " + isValidIPv6(test3));
        System.out.println("IP Version: " + getIPVersion(test3));
        System.out.println();
        
        // Test case 4: IPv6 with double colon
        String test4 = "2001:db8::8a2e:370:7334";
        System.out.println("Test 4: " + test4);
        System.out.println("Valid IPv6: " + isValidIPv6(test4));
        System.out.println("IP Version: " + getIPVersion(test4));
        System.out.println();
        
        // Test case 5: Invalid IP
        String test5 = "invalid.ip.address";
        System.out.println("Test 5: " + test5);
        System.out.println("Valid IPv4: " + isValidIPv4(test5));
        System.out.println("Valid IPv6: " + isValidIPv6(test5));
        System.out.println("IP Version: " + getIPVersion(test5));
        System.out.println();
    }
}
