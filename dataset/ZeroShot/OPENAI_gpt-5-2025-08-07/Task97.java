import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.XMLConstants;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.xpath.XPathFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;

import java.io.StringWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.BufferedWriter;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.List;

public class Task97 {

    public static List<String> queryById(String xmlFilePath, String idValue) throws Exception {
        if (xmlFilePath == null || idValue == null) {
            throw new IllegalArgumentException("xmlFilePath and idValue must not be null");
        }
        Path p = Paths.get(xmlFilePath).normalize();
        if (Files.isSymbolicLink(p) || !Files.isRegularFile(p)) {
            throw new IOException("Invalid file: not a regular file or is a symbolic link");
        }

        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        secureDbf(dbf);
        Document doc;
        try (InputStream in = Files.newInputStream(p)) {
            DocumentBuilder db = dbf.newDocumentBuilder();
            doc = db.parse(in);
        }

        String expr = "/tag[@id=" + xpathLiteral(idValue) + "]";
        XPathFactory xpf = XPathFactory.newInstance();
        XPath xp = xpf.newXPath();

        NodeList nodes;
        try {
            nodes = (NodeList) xp.evaluate(expr, doc, XPathConstants.NODESET);
        } catch (XPathExpressionException e) {
            throw new IllegalArgumentException("Invalid XPath expression", e);
        }

        List<String> results = new ArrayList<>();
        for (int i = 0; i < nodes.getLength(); i++) {
            results.add(nodeToString(nodes.item(i)));
        }
        return results;
    }

    private static void secureDbf(DocumentBuilderFactory dbf) throws ParserConfigurationException {
        dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
        dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
        dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
        dbf.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
        dbf.setXIncludeAware(false);
        dbf.setExpandEntityReferences(false);
        dbf.setNamespaceAware(false);
        dbf.setValidating(false);
    }

    private static String nodeToString(Node node) throws Exception {
        TransformerFactory tf = TransformerFactory.newInstance();
        tf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        Transformer t = tf.newTransformer();
        t.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
        StringWriter sw = new StringWriter();
        t.transform(new DOMSource(node), new StreamResult(sw));
        return sw.toString();
    }

    private static String xpathLiteral(String s) {
        if (s.indexOf('\'') == -1) {
            return "'" + s + "'";
        }
        if (s.indexOf('"') == -1) {
            return "\"" + s + "\"";
        }
        StringBuilder sb = new StringBuilder("concat(");
        boolean first = true;
        int start = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '\'') {
                if (!first) sb.append(", ");
                sb.append("'").append(s, start, i).append("', \"'\"");
                start = i + 1;
                first = false;
            }
        }
        if (start <= s.length()) {
            if (!first) sb.append(", ");
            sb.append("'").append(s.substring(start)).append("'");
        }
        sb.append(")");
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        if (args.length >= 2) {
            List<String> out = queryById(args[0], args[1]);
            for (String s : out) {
                System.out.println(s);
            }
            return;
        }

        String sample = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
                "<root>\n" +
                "  <tag id=\"1\">Alpha</tag>\n" +
                "  <tag id=\"2\"><inner>Beta</inner></tag>\n" +
                "  <tag id=\"3\">Gamma</tag>\n" +
                "  <tag id=\"42\">FortyTwo</tag>\n" +
                "  <tag id=\"x\">X-Value</tag>\n" +
                "</root>\n";
        Path samplePath = Paths.get("sample97.xml");
        try (BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(Files.newOutputStream(samplePath, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING), StandardCharsets.UTF_8))) {
            bw.write(sample);
        }

        String[] testIds = {"1", "2", "3", "42", "nope"};
        for (String id : testIds) {
            List<String> res = queryById(samplePath.toString(), id);
            System.out.println("ID=" + id + " count=" + res.size());
            for (String s : res) {
                System.out.println(s);
            }
        }
    }
}