public class Task118 {
    /**
     * Validates if the given string is a valid IPv4 address.
     * A valid IPv4 address consists of four octets separated by dots.
     * Each octet must be a number between 0 and 255.
     * Leading zeros are not allowed, except for the number "0" itself.
     *
     * @param ip The string to validate.
     * @return true if the string is a valid IPv4 address, false otherwise.
     */
    public static boolean isValidIP(String ip) {
        if (ip == null || ip.isEmpty()) {
            return false;
        }

        // A valid IP address should not end with a dot.
        // The split method has tricky behavior with trailing delimiters,
        // so we check this case explicitly for clarity.
        if (ip.endsWith(".")) {
            return false;
        }

        String[] parts = ip.split("\\.");

        if (parts.length != 4) {
            return false;
        }

        for (String part : parts) {
            // Each part must not be empty (e.g., from "1..2.3.4").
            if (part.isEmpty()) {
                return false;
            }

            // A part cannot have leading zeros unless it is just "0".
            if (part.length() > 1 && part.startsWith("0")) {
                return false;
            }
            
            // Check for any non-digit characters.
            for (char c : part.toCharArray()) {
                if (!Character.isDigit(c)) {
                    return false;
                }
            }

            try {
                int num = Integer.parseInt(part);
                if (num < 0 || num > 255) {
                    return false;
                }
            } catch (NumberFormatException e) {
                // This case should be caught by the isDigit check, but is here for robustness.
                return false;
            }
        }
        return true;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "192.168.1.1",      // Valid
            "255.255.255.255",  // Valid
            "0.0.0.0",          // Valid
            "1.2.3.4",          // Valid
            "192.168.1.256",    // Invalid: octet > 255
            "192.168.01.1",     // Invalid: leading zero
            "192.168.1",        // Invalid: too few parts
            "a.b.c.d",          // Invalid: non-numeric parts
            "1.2.3.4.",         // Invalid: trailing dot
            ".1.2.3.4",         // Invalid: leading dot
            "1..2.3.4"          // Invalid: consecutive dots
        };

        for (String ip : testCases) {
            System.out.println("IP: " + ip + " is " + (isValidIP(ip) ? "valid" : "invalid"));
        }
    }
}