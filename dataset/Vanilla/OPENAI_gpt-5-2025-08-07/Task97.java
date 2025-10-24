import java.io.*;
import java.nio.file.*;
import java.util.*;

public class Task97 {

    public static String executeXPathLike(String xmlFilePath, String idValue) {
        try {
            String content = Files.readString(Path.of(xmlFilePath));
            StringBuilder result = new StringBuilder();
            int pos = 0;
            while (true) {
                int start = content.indexOf("<tag", pos);
                if (start == -1) break;
                int gt = content.indexOf(">", start);
                if (gt == -1) break;
                boolean selfClosing = gt > start && content.charAt(gt - 1) == '/';
                String startTagAttrs = content.substring(start + 4, gt);
                String attrVal = extractAttrValue(startTagAttrs, "id");
                if (attrVal != null && attrVal.equals(idValue)) {
                    if (selfClosing) {
                        String snippet = content.substring(start, gt + 1);
                        if (result.length() > 0) result.append("\n");
                        result.append(snippet);
                    } else {
                        int endIdx = findMatchingEnd(content, gt + 1);
                        if (endIdx != -1) {
                            String snippet = content.substring(start, endIdx);
                            if (result.length() > 0) result.append("\n");
                            result.append(snippet);
                        }
                    }
                }
                pos = start + 1;
            }
            return result.toString();
        } catch (IOException e) {
            return "";
        }
    }

    private static int findMatchingEnd(String s, int from) {
        int depth = 1;
        int search = from;
        while (depth > 0) {
            int nextOpen = s.indexOf("<tag", search);
            int nextClose = s.indexOf("</tag>", search);
            if (nextClose == -1) return -1;
            if (nextOpen != -1 && nextOpen < nextClose) {
                int gt2 = s.indexOf(">", nextOpen);
                if (gt2 == -1) return -1;
                boolean selfClosing = gt2 > nextOpen && s.charAt(gt2 - 1) == '/';
                if (!selfClosing) {
                    depth++;
                }
                search = gt2 + 1;
            } else {
                depth--;
                search = nextClose + 6;
            }
        }
        return search;
    }

    private static String extractAttrValue(String attrs, String name) {
        // Supports quoted attribute values: id="..." or id='...'
        int idx = indexOfWord(attrs, name);
        while (idx != -1) {
            int j = idx + name.length();
            while (j < attrs.length() && Character.isWhitespace(attrs.charAt(j))) j++;
            if (j < attrs.length() && attrs.charAt(j) == '=') {
                j++;
                while (j < attrs.length() && Character.isWhitespace(attrs.charAt(j))) j++;
                if (j < attrs.length()) {
                    char q = attrs.charAt(j);
                    if (q == '"' || q == '\'') {
                        int k = attrs.indexOf(q, j + 1);
                        if (k != -1) {
                            return attrs.substring(j + 1, k);
                        }
                    } else {
                        // unquoted until whitespace
                        int k = j;
                        while (k < attrs.length() && !Character.isWhitespace(attrs.charAt(k))) k++;
                        return attrs.substring(j, k);
                    }
                }
            }
            idx = indexOfWord(attrs, name, idx + 1);
        }
        return null;
    }

    private static int indexOfWord(String s, String w) {
        return indexOfWord(s, w, 0);
    }

    private static int indexOfWord(String s, String w, int from) {
        int i = from - 1;
        while (true) {
            i = s.indexOf(w, i + 1);
            if (i == -1) return -1;
            boolean leftOk = (i == 0) || !Character.isLetterOrDigit(s.charAt(i - 1));
            int end = i + w.length();
            boolean rightOk = (end >= s.length()) || !Character.isLetterOrDigit(s.charAt(end));
            if (leftOk && rightOk) return i;
        }
    }

    private static void writeSampleFile(String path) throws IOException {
        String xml = ""
            + "<root>\n"
            + "  <tag id=\"1\">Content one</tag>\n"
            + "  <tag id=\"2\">\n"
            + "     Two\n"
            + "     <child>Alpha</child>\n"
            + "  </tag>\n"
            + "  <tag id=\"3\"/>\n"
            + "  <other id=\"1\">Not a tag</other>\n"
            + "  <tag id=\"nested\">\n"
            + "     <tag id=\"inner\">Inner content</tag>\n"
            + "  </tag>\n"
            + "  <tag id=\"spaces\">  spaced  </tag>\n"
            + "</root>\n";
        Files.writeString(Path.of(path), xml);
    }

    public static void main(String[] args) throws Exception {
        if (args.length >= 2) {
            String idVal = args[0];
            String xmlPath = args[1];
            String xpath = "/tag[@id='" + idVal + "']";
            String out = executeXPathLike(xmlPath, idVal);
            System.out.println(out);
            return;
        }
        String samplePath = "sample_task97.xml";
        writeSampleFile(samplePath);
        String[] tests = new String[] {"1", "2", "3", "missing", "nested"};
        for (String id : tests) {
            String xpath = "/tag[@id='" + id + "']";
            String res = executeXPathLike(samplePath, id);
            System.out.println("XPath: " + xpath);
            System.out.println(res);
            System.out.println("-----");
        }
    }
}