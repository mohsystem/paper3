import java.util.Arrays;

public class Task118 {

    /**
     * Validates if a given string is a valid IPv4 address.
     *
     * An IPv4 address is valid if it consists of four octets separated by dots,
     * where each octet is a decimal number from 0 to 255.
     * Leading zeros are not allowed (e.g., "01" is invalid), except for the number "0" itself.
     *
     * @param ip The string to validate.
     * @return true if the string is a valid IPv4 address, false otherwise.
     */
    public static boolean isValidIPv4(String ip) {
        // Rule #1: Ensure all input is validated.
        if (ip == null || ip.isEmpty()) {
            return false;
        }

        // The limit argument -1 ensures that trailing empty strings are not discarded.
        // This correctly handles cases like "1.2.3."
        String[] octets = ip.split("\\.", -1);

        if (octets.length != 4) {
            return false;
        }

        for (String octetStr : octets) {
            if (octetStr.isEmpty() || octetStr.length() > 3) {
                return false;
            }

            // Check for leading zeros, e.g., "01", "007". "0" is allowed.
            if (octetStr.length() > 1 && octetStr.startsWith("0")) {
                return false;
            }

            // Rule #6: Handle potential exceptions from parsing.
            try {
                // Check if all characters are digits and parse.
                int octetValue = Integer.parseInt(octetStr);
                if (octetValue < 0 || octetValue > 255) {
                    return false;
                }
            } catch (NumberFormatException e) {
                // The string contained non-digit characters.
                return false;
            }
        }

        return true;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "192.168.1.1",       // Valid
            "256.0.0.1",         // Invalid range
            "192.168.01.1",      // Invalid leading zero
            "192.168.1.a",       // Invalid character
            "1.2.3"              // Invalid structure (too few parts)
        };

        for (String ip : testCases) {
            System.out.println("Is \"" + ip + "\" a valid IPv4 address? " + isValidIPv4(ip));
        }

        System.out.println("\n--- Additional Test Cases ---");
        String[] moreTestCases = {
            "0.0.0.0",           // Valid
            "255.255.255.255",   // Valid
            "1.2.3.4.5",         // Invalid structure (too many parts)
            "1..2.3.4",          // Invalid structure (empty octet)
            "1.2.3.",            // Invalid structure (trailing dot)
            "",                  // Invalid empty string
            null,                // Invalid null
            "123.45.67.89"       // Valid
        };

        for (String ip : moreTestCases) {
             System.out.println("Is \"" + ip + "\" a valid IPv4 address? " + isValidIPv4(ip));
        }
    }
}