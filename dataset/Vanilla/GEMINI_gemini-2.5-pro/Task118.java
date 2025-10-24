import java.util.Scanner;

public class Task118 {

    public String validateIPAddress(String IP) {
        if (IP == null || IP.isEmpty()) {
            return "Neither";
        }

        if (IP.contains(".")) {
            return validateIPv4(IP);
        } else if (IP.contains(":")) {
            return validateIPv6(IP);
        } else {
            return "Neither";
        }
    }

    private String validateIPv4(String ip) {
        if (ip.endsWith(".")) {
            return "Neither";
        }
        String[] parts = ip.split("\\.");
        if (parts.length != 4) {
            return "Neither";
        }

        for (String part : parts) {
            if (part.isEmpty() || (part.length() > 1 && part.startsWith("0"))) {
                return "Neither";
            }
            if (part.length() > 3) {
                return "Neither";
            }
            for (char c : part.toCharArray()) {
                if (!Character.isDigit(c)) {
                    return "Neither";
                }
            }
            try {
                int num = Integer.parseInt(part);
                if (num < 0 || num > 255) {
                    return "Neither";
                }
            } catch (NumberFormatException e) {
                return "Neither";
            }
        }
        return "IPv4";
    }

    private String validateIPv6(String ip) {
         if (ip.endsWith(":")) {
            return "Neither";
        }
        String[] parts = ip.split(":");
        if (parts.length != 8) {
            return "Neither";
        }

        for (String part : parts) {
            if (part.isEmpty() || part.length() > 4) {
                return "Neither";
            }
            for (char c : part.toCharArray()) {
                if (!Character.isDigit(c) && (c < 'a' || c > 'f') && (c < 'A' || c > 'F')) {
                    return "Neither";
                }
            }
        }
        return "IPv6";
    }

    public static void main(String[] args) {
        Task118 validator = new Task118();
        String[] testCases = {
            "172.16.254.1",
            "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
            "256.256.256.256",
            "192.168.01.1",
            "2001:0db8:85a3::8a2e:0370:7334"
        };

        for (String ip : testCases) {
            System.out.println("IP: " + ip + " -> " + validator.validateIPAddress(ip));
        }
    }
}