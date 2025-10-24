
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task118 {
    
    /**
     * Validates and processes IPv4 addresses
     * Prevents injection attacks and ensures strict validation
     */
    public static boolean validateIPv4(String ip) {
        if (ip == null || ip.isEmpty()) {
            return false;
        }
        
        // Prevent excessively long inputs (DoS prevention)
        if (ip.length() > 15) {
            return false;
        }
        
        // Strict regex pattern for IPv4 validation
        String ipv4Pattern = "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";
        
        if (!Pattern.matches(ipv4Pattern, ip)) {
            return false;
        }
        
        // Additional validation: split and check each octet
        String[] octets = ip.split("\\\\.");
        
        if (octets.length != 4) {
            return false;
        }
        
        for (String octet : octets) {
            try {
                int value = Integer.parseInt(octet);
                if (value < 0 || value > 255) {
                    return false;
                }
                // Reject leading zeros (except for "0" itself)
                if (octet.length() > 1 && octet.startsWith("0")) {
                    return false;
                }
            } catch (NumberFormatException e) {
                return false;
            }
        }
        
        return true;
    }
    
    /**
     * Validates and processes IPv6 addresses
     * Prevents injection attacks and ensures strict validation
     */
    public static boolean validateIPv6(String ip) {
        if (ip == null || ip.isEmpty()) {
            return false;
        }
        
        // Prevent excessively long inputs (DoS prevention)
        if (ip.length() > 39) {
            return false;
        }
        
        // Strict regex pattern for IPv6 validation
        String ipv6Pattern = "^([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$|" +
                            "^::([0-9a-fA-F]{1,4}:){0,6}[0-9a-fA-F]{1,4}$|" +
                            "^([0-9a-fA-F]{1,4}:){1}:([0-9a-fA-F]{1,4}:){0,5}[0-9a-fA-F]{1,4}$|" +
                            "^([0-9a-fA-F]{1,4}:){2}:([0-9a-fA-F]{1,4}:){0,4}[0-9a-fA-F]{1,4}$|" +
                            "^([0-9a-fA-F]{1,4}:){3}:([0-9a-fA-F]{1,4}:){0,3}[0-9a-fA-F]{1,4}$|" +
                            "^([0-9a-fA-F]{1,4}:){4}:([0-9a-fA-F]{1,4}:){0,2}[0-9a-fA-F]{1,4}$|" +
                            "^([0-9a-fA-F]{1,4}:){5}:([0-9a-fA-F]{1,4}:){0,1}[0-9a-fA-F]{1,4}$|" +
                            "^([0-9a-fA-F]{1,4}:){6}:[0-9a-fA-F]{1,4}$|" +
                            "^::$";
        
        return Pattern.matches(ipv6Pattern, ip);
    }
    
    /**
     * Determines IP version and validates accordingly
     * Returns: "Valid IPv4", "Valid IPv6", or "Invalid IP"
     */
    public static String processIPAddress(String ip) {
        if (ip == null || ip.isEmpty()) {
            return "Invalid IP";
        }
        
        // Sanitize input - remove whitespace
        ip = ip.trim();
        
        // Prevent buffer overflow and DoS attacks
        if (ip.length() > 100) {
            return "Invalid IP";
        }
        
        if (validateIPv4(ip)) {
            return "Valid IPv4";
        } else if (validateIPv6(ip)) {
            return "Valid IPv6";
        } else {
            return "Invalid IP";
        }
    }
    
    /**
     * Normalizes IPv4 address (removes leading zeros safely)
     */
    public static String normalizeIPv4(String ip) {
        if (!validateIPv4(ip)) {
            return null;
        }
        
        String[] octets = ip.split("\\\\.");
        StringBuilder normalized = new StringBuilder();
        
        for (int i = 0; i < octets.length; i++) {
            int value = Integer.parseInt(octets[i]);
            normalized.append(value);
            if (i < octets.length - 1) {
                normalized.append(".");
            }
        }
        
        return normalized.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("=== IP Address Validation Tests ===\\n");
        
        // Test Case 1: Valid IPv4
        String test1 = "192.168.1.1";
        System.out.println("Test 1: " + test1);
        System.out.println("Result: " + processIPAddress(test1));
        System.out.println("IPv4 Valid: " + validateIPv4(test1));
        System.out.println("Normalized: " + normalizeIPv4(test1));
        System.out.println();
        
        // Test Case 2: Invalid IPv4 (out of range)
        String test2 = "256.168.1.1";
        System.out.println("Test 2: " + test2);
        System.out.println("Result: " + processIPAddress(test2));
        System.out.println("IPv4 Valid: " + validateIPv4(test2));
        System.out.println();
        
        // Test Case 3: Valid IPv6
        String test3 = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
        System.out.println("Test 3: " + test3);
        System.out.println("Result: " + processIPAddress(test3));
        System.out.println("IPv6 Valid: " + validateIPv6(test3));
        System.out.println();
        
        // Test Case 4: Invalid IP (malicious input attempt)
        String test4 = "192.168.1.1; DROP TABLE users;";
        System.out.println("Test 4: " + test4);
        System.out.println("Result: " + processIPAddress(test4));
        System.out.println("IPv4 Valid: " + validateIPv4(test4));
        System.out.println();
        
        // Test Case 5: Edge case (localhost)
        String test5 = "127.0.0.1";
        System.out.println("Test 5: " + test5);
        System.out.println("Result: " + processIPAddress(test5));
        System.out.println("IPv4 Valid: " + validateIPv4(test5));
        System.out.println("Normalized: " + normalizeIPv4(test5));
        System.out.println();
    }
}
