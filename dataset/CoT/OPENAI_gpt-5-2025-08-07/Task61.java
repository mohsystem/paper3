import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;

public class Task61 {

    // Node structure to represent XML tree (simplified)
    static class Node {
        String name;
        List<Node> children = new ArrayList<>();
        Node(String name) { this.name = name; }
    }

    // Public API: returns the root element name of the XML document
    public static String getRootElementName(String xml) {
        if (xml == null) return "";
        ParseResult result = parseXML(xml);
        return result.rootName == null ? "" : result.rootName;
    }

    // Internal structure for parse results
    private static class ParseResult {
        String rootName;
        Node rootNode;
    }

    // Parse XML to build a basic structure and identify the root
    private static ParseResult parseXML(String xml) {
        ParseResult res = new ParseResult();
        char[] a = xml.toCharArray();
        int n = a.length;
        int i = 0;

        Deque<Node> stack = new ArrayDeque<>();

        while (i < n) {
            int lt = indexOf(a, '<', i);
            if (lt < 0) break;
            i = lt;

            if (i + 1 >= n) break;

            // Processing instruction
            if (startsWith(a, i, "<?")) {
                int j = indexOfSeq(a, i + 2, '?', '>');
                if (j < 0) break;
                i = j + 1;
                continue;
            }

            // Comment
            if (startsWith(a, i, "<!--")) {
                int j = indexOfSeq(a, i + 4, '-', '-', '>');
                if (j < 0) break;
                i = j + 1;
                continue;
            }

            // CDATA
            if (startsWith(a, i, "<![CDATA[")) {
                int j = indexOfSeq(a, i + 9, ']', ']', '>');
                if (j < 0) break;
                i = j + 1;
                continue;
            }

            // DOCTYPE or other declaration
            if (startsWith(a, i, "<!")) {
                int j = skipDeclaration(a, i + 2);
                if (j < 0) break;
                i = j + 1;
                continue;
            }

            // Closing tag
            if (a[i + 1] == '/') {
                int j = i + 2;
                j = skipSpaces(a, j);
                int start = j;
                while (j < n && isNameChar(a[j])) j++;
                // name = new String(a, start, j - start); // can be used for validation
                j = skipToTagEnd(a, j);
                if (j < 0) break;
                if (!stack.isEmpty()) stack.pop();
                i = j + 1;
                continue;
            }

            // Opening tag
            int j = i + 1;
            j = skipSpaces(a, j);
            int start = j;
            while (j < n && isNameChar(a[j])) j++;
            if (j == start) { // invalid tag name
                i++;
                continue;
            }
            String name = new String(a, start, j - start);
            Node node = new Node(name);
            if (stack.isEmpty()) {
                res.rootName = name;
                res.rootNode = node;
            } else {
                stack.peek().children.add(node);
            }

            // Scan attributes and detect self-close
            boolean selfClose = false;
            int k = j;
            char quote = 0;
            while (k < n) {
                char c = a[k];
                if (quote != 0) {
                    if (c == quote) quote = 0;
                    k++;
                    continue;
                } else {
                    if (c == '\'' || c == '"') {
                        quote = c;
                        k++;
                        continue;
                    }
                    if (c == '/') {
                        if (k + 1 < n && a[k + 1] == '>') {
                            selfClose = true;
                            k += 2;
                            break;
                        }
                    }
                    if (c == '>') {
                        k++;
                        break;
                    }
                    k++;
                }
            }
            if (!selfClose) stack.push(node);
            i = k;
        }

        return res;
    }

    private static boolean startsWith(char[] a, int i, String s) {
        if (i + s.length() > a.length) return false;
        for (int k = 0; k < s.length(); k++) if (a[i + k] != s.charAt(k)) return false;
        return true;
    }

    private static int indexOf(char[] a, char ch, int from) {
        for (int i = Math.max(0, from); i < a.length; i++) if (a[i] == ch) return i;
        return -1;
        return -1;
    }

    // Find sequence x,y,z in order (consecutive)
    private static int indexOfSeq(char[] a, int from, char x, char y, char z) {
        for (int i = Math.max(0, from); i + 2 < a.length; i++) {
            if (a[i] == x && a[i + 1] == y && a[i + 2] == z) return i + 2;
        }
        return -1;
    }

    // For "?>" pair
    private static int indexOfSeq(char[] a, int from, char x, char y) {
        for (int i = Math.max(0, from); i + 1 < a.length; i++) {
            if (a[i] == x && a[i + 1] == y) return i + 1;
        }
        return -1;
    }

    private static int skipSpaces(char[] a, int i) {
        while (i < a.length) {
            char c = a[i];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') i++;
            else break;
        }
        return i;
    }

    private static boolean isNameChar(char c) {
        return Character.isLetterOrDigit(c) || c == '_' || c == '-' || c == ':' || c == '.';
    }

    // Skip until matching '>' for declaration (handles internal subset brackets [] depth)
    private static int skipDeclaration(char[] a, int i) {
        int n = a.length;
        int depth = 0;
        for (int k = i; k < n; k++) {
            char c = a[k];
            if (c == '[') depth++;
            else if (c == ']') { if (depth > 0) depth--; }
            else if (c == '>' && depth == 0) return k;
        }
        return -1;
    }

    // Skip to the end of a tag '>' (attributes with quotes handled)
    private static int skipToTagEnd(char[] a, int i) {
        int n = a.length;
        char quote = 0;
        for (int k = i; k < n; k++) {
            char c = a[k];
            if (quote != 0) {
                if (c == quote) quote = 0;
                continue;
            } else {
                if (c == '\'' || c == '"') { quote = c; continue; }
                if (c == '>') return k;
            }
        }
        return -1;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "<root><child/></root>",
            "<?xml version='1.0'?><!-- comment --><a id='1'><b></b><c/></a>",
            "  \n <root>text<inner>more</inner></root> ",
            "<x><y></x>",
            "<!DOCTYPE note SYSTEM 'Note.dtd'><note><![CDATA[<notatag>]]><to>T</to></note>"
        };
        for (String t : tests) {
            System.out.println(getRootElementName(t));
        }
    }
}