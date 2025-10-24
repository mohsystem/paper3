import java.util.regex.Pattern;

class Task118 {
    /**
     * Validates if the given string is a valid IPv4 address.
     *
     * @param ip The string to validate.
     * @return true if the string is a valid IPv4 address, false otherwise.
     */
    public static boolean isValidIP(String ip) {
        if (ip == null || ip.isEmpty()) {
            return false;
        }

        // The split method with a negative limit ensures that trailing empty strings are not discarded.
        // For example, "1.2.3." would split into ["1", "2", "3", ""], which will be correctly identified as invalid.
        String[] parts = ip.split("\\.", -1);

        if (parts.length != 4) {
            return false;
        }

        for (String part : parts) {
            // Each part must not be empty and must have at most 3 digits.
            if (part.isEmpty() || part.length() > 3) {
                return false;
            }

            // Each part must contain only digits.
            for (char c : part.toCharArray()) {
                if (!Character.isDigit(c)) {
                    return false;
                }
            }

            // A part cannot have leading zeros unless it is "0" itself.
            if (part.length() > 1 && part.startsWith("0")) {
                return false;
            }

            try {
                int num = Integer.parseInt(part);
                // The number must be in the range [0, 255].
                if (num < 0 || num > 255) {
                    return false;
                }
            } catch (NumberFormatException e) {
                // This should not happen due to the isDigit check, but serves as a safeguard.
                return false;
            }
        }

        return true;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "127.0.0.1",       // Valid
            "256.0.0.0",       // Invalid - range
            "192.168.01.1",    // Invalid - leading zero
            "1.2.3",           // Invalid - format/parts count
            "a.b.c.d"          // Invalid - non-numeric
        };

        for (String testCase : testCases) {
            System.out.println("IP: \"" + testCase + "\" is " + (isValidIP(testCase) ? "Valid" : "Invalid"));
        }
    }
}