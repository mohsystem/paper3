import java.net.*;
import java.util.*;

public class Task110 {

    public static boolean validateURL(String url) {
        if (url == null || url.isEmpty()) return false;
        // Reject whitespace
        for (int i = 0; i < url.length(); i++) {
            if (Character.isWhitespace(url.charAt(i))) return false;
        }

        String lower = url.toLowerCase(Locale.ROOT);
        String scheme;
        int schemeEnd = lower.indexOf("://");
        if (schemeEnd <= 0) return false;
        scheme = lower.substring(0, schemeEnd);
        if (!(scheme.equals("http") || scheme.equals("https") || scheme.equals("ftp"))) return false;

        int pos = schemeEnd + 3;
        int n = url.length();

        // Parse host (and optional port)
        String host = null;
        int port = -1;

        if (pos >= n) return false;

        if (url.charAt(pos) == '[') {
            int close = url.indexOf(']', pos + 1);
            if (close < 0) return false;
            host = url.substring(pos + 1, close);
            if (!isValidIPv6(host)) return false;
            pos = close + 1;
            if (pos < n && url.charAt(pos) == ':') {
                int portStart = ++pos;
                int portEnd = pos;
                while (portEnd < n && Character.isDigit(url.charAt(portEnd))) portEnd++;
                if (portStart == portEnd) return false;
                if (!isValidPort(url.substring(portStart, portEnd))) return false;
                port = Integer.parseInt(url.substring(portStart, portEnd));
                pos = portEnd;
            }
        } else {
            int hostStart = pos;
            while (pos < n) {
                char c = url.charAt(pos);
                if (c == ':' || c == '/' || c == '?' || c == '#') break;
                pos++;
            }
            if (pos == hostStart) return false;
            host = url.substring(hostStart, pos);
            if (host.indexOf(':') >= 0) return false; // IPv6 must be in brackets
            if (!(isValidIPv4(host) || isValidDomain(host) || host.equalsIgnoreCase("localhost"))) return false;

            if (pos < n && url.charAt(pos) == ':') {
                int portStart = ++pos;
                int portEnd = pos;
                while (portEnd < n && Character.isDigit(url.charAt(portEnd))) portEnd++;
                if (portStart == portEnd) return false;
                if (!isValidPort(url.substring(portStart, portEnd))) return false;
                port = Integer.parseInt(url.substring(portStart, portEnd));
                pos = portEnd;
            }
        }

        // Remaining part path/query/fragment - just ensure no whitespace
        while (pos < n) {
            if (Character.isWhitespace(url.charAt(pos))) return false;
            pos++;
        }
        return true;
    }

    private static boolean isValidPort(String s) {
        try {
            int p = Integer.parseInt(s);
            return p >= 1 && p <= 65535;
        } catch (Exception e) {
            return false;
        }
    }

    private static boolean isHex(char c) {
        return (c >= '0' && c <= '9') ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F');
    }

    private static boolean isValidIPv4(String s) {
        String[] parts = s.split("\\.", -1);
        if (parts.length != 4) return false;
        for (String p : parts) {
            if (p.length() == 0 || p.length() > 3) return false;
            for (int i = 0; i < p.length(); i++) {
                if (!Character.isDigit(p.charAt(i))) return false;
            }
            int val = Integer.parseInt(p);
            if (val < 0 || val > 255) return false;
            // Disallow leading zeros like "01" except "0"
            if (p.length() > 1 && p.charAt(0) == '0') return false;
        }
        return true;
    }

    private static boolean isAlnum(char c) {
        return (c >= '0' && c <= '9') ||
               (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z');
    }

    private static boolean isValidDomain(String host) {
        if (host.length() == 0 || host.length() > 253) return false;
        if (host.startsWith(".") || host.endsWith(".")) return false;
        String[] labels = host.split("\\.", -1);
        for (String label : labels) {
            if (label.length() < 1 || label.length() > 63) return false;
            if (!isAlnum(label.charAt(0)) || !isAlnum(label.charAt(label.length() - 1))) return false;
            for (int i = 0; i < label.length(); i++) {
                char c = label.charAt(i);
                if (!(isAlnum(c) || c == '-')) return false;
            }
        }
        return true;
    }

    private static boolean isValidIPv6(String s) {
        if (s == null || s.isEmpty()) return false;

        String t = s;
        int maxHextets = 8;

        if (t.indexOf('.') >= 0) {
            int lastColon = t.lastIndexOf(':');
            if (lastColon < 0) return false;
            String ipv4 = t.substring(lastColon + 1);
            if (!isValidIPv4(ipv4)) return false;
            t = t.substring(0, lastColon);
            maxHextets = 6;
        }

        String[] dc = t.split("::", -1);
        if (dc.length > 2) return false;

        int hextetCount = 0;
        for (String side : dc) {
            if (side.isEmpty()) continue;
            String[] hs = side.split(":", -1);
            for (String h : hs) {
                if (h.length() < 1 || h.length() > 4) return false;
                for (int i = 0; i < h.length(); i++) if (!isHex(h.charAt(i))) return false;
                hextetCount++;
            }
        }

        if (dc.length == 2) {
            return hextetCount < maxHextets;
        } else {
            return hextetCount == maxHextets;
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "https://example.com",
            "http://sub.domain.co.uk:8080/path?query=1#frag",
            "ftp://192.168.1.1/resource",
            "https://[2001:db8::1]/index.html",
            "http://-invalid-.com"
        };
        for (String t : tests) {
            System.out.println(t + " -> " + validateURL(t));
        }
    }
}