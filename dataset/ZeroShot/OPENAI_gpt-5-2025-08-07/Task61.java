import java.util.*;

public class Task61 {

    public static String getRootElementName(String xml) {
        if (xml == null) return "";
        int n = xml.length();
        int i = 0;

        // Skip BOM if present
        if (i < n && xml.charAt(i) == '\uFEFF') i++;

        while (i < n) {
            // Skip whitespace
            while (i < n) {
                char c = xml.charAt(i);
                if (c == ' ' || c == '\t' || c == '\r' || c == '\n') i++;
                else break;
            }
            if (i >= n) break;

            // Processing Instruction
            if (startsWithAt(xml, i, "<?")) {
                int end = xml.indexOf("?>", i + 2);
                if (end == -1) return "";
                i = end + 2;
                continue;
            }

            // Comment
            if (startsWithAt(xml, i, "<!--")) {
                int end = xml.indexOf("-->", i + 4);
                if (end == -1) return "";
                i = end + 3;
                continue;
            }

            // DOCTYPE or other declaration
            if (startsWithAt(xml, i, "<!")) {
                // Special-case CDATA (though typically not in prolog), skip safely
                if (startsWithAt(xml, i, "<![CDATA[")) {
                    int end = xml.indexOf("]]>", i + 9);
                    if (end == -1) return "";
                    i = end + 3;
                    continue;
                }
                // General declaration skip with quote/bracket awareness
                i += 2; // after "<!"
                int depth = 0;
                boolean inQuote = false;
                char quoteCh = 0;
                while (i < n) {
                    char ch = xml.charAt(i);
                    if (inQuote) {
                        if (ch == quoteCh) inQuote = false;
                    } else {
                        if (ch == '"' || ch == '\'') {
                            inQuote = true;
                            quoteCh = ch;
                        } else if (ch == '[') {
                            depth++;
                        } else if (ch == ']') {
                            if (depth > 0) depth--;
                        } else if (ch == '>' && depth == 0) {
                            i++;
                            break;
                        }
                    }
                    i++;
                }
                continue;
            }

            // Start tag
            if (xml.charAt(i) == '<') {
                if (i + 1 < n && xml.charAt(i + 1) == '/') {
                    // Unexpected close tag before any start tag; invalid XML for our purposes
                    return "";
                }
                int j = i + 1;
                while (j < n) {
                    char ch = xml.charAt(j);
                    if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '/' || ch == '>') {
                        break;
                    }
                    j++;
                }
                if (j == i + 1) return "";
                return xml.substring(i + 1, j);
            }

            // Skip any other characters until next '<'
            i++;
        }
        return "";
    }

    private static boolean startsWithAt(String s, int idx, String pat) {
        int n = s.length();
        int m = pat.length();
        if (idx < 0 || idx + m > n) return false;
        for (int k = 0; k < m; k++) {
            if (s.charAt(idx + k) != pat.charAt(k)) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "<root></root>",
            "<?xml version=\"1.0\"?><root attr='1'><child/></root>",
            "<!-- comment --><!DOCTYPE note [<!ELEMENT note ANY>]><note><to>T</to></note>",
            "<x:Root xmlns:x='urn:x'><x:child/></x:Root>",
            "\uFEFF  <?pi test?><root/>"
        };
        for (String t : tests) {
            System.out.println(getRootElementName(t));
        }
    }
}