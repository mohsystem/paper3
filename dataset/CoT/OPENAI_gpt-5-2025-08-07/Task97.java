import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.*;
import javax.xml.transform.*;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import org.w3c.dom.*;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

public class Task97 {
    public static List<String> queryById(String xmlPath, String idValue) throws Exception {
        Objects.requireNonNull(xmlPath, "xmlPath");
        Objects.requireNonNull(idValue, "idValue");
        Path p = Paths.get(xmlPath).normalize().toAbsolutePath();
        if (!Files.exists(p) || !Files.isRegularFile(p)) {
            throw new FileNotFoundException("XML file not found: " + p);
        }
        byte[] data = Files.readAllBytes(p);
        String content = new String(data, StandardCharsets.UTF_8);
        if (content.contains("<!DOCTYPE")) {
            throw new SecurityException("DOCTYPE is disallowed for security reasons.");
        }

        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setNamespaceAware(false);
        dbf.setXIncludeAware(false);
        dbf.setExpandEntityReferences(false);
        dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        try {
            dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
        } catch (ParserConfigurationException ignored) {}
        try {
            dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
        } catch (ParserConfigurationException ignored) {}
        try {
            dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
        } catch (ParserConfigurationException ignored) {}

        DocumentBuilder db = dbf.newDocumentBuilder();
        try (InputStream is = new ByteArrayInputStream(content.getBytes(StandardCharsets.UTF_8))) {
            Document doc = db.parse(is);
            XPathFactory xpf = XPathFactory.newInstance();
            XPath xpath = xpf.newXPath();
            String safeLiteral = buildXPathLiteral(idValue);
            String expr = "/tag[@id=" + safeLiteral + "]";
            NodeList nodes = (NodeList) xpath.evaluate(expr, doc, XPathConstants.NODESET);
            List<String> result = new ArrayList<>();
            for (int i = 0; i < nodes.getLength(); i++) {
                result.add(nodeToString(nodes.item(i)));
            }
            return result;
        }
    }

    private static String buildXPathLiteral(String s) {
        if (!s.contains("'")) {
            return "'" + s + "'";
        } else if (!s.contains("\"")) {
            return "\"" + s + "\"";
        } else {
            StringBuilder sb = new StringBuilder("concat(");
            String[] parts = s.split("\"", -1);
            for (int i = 0; i < parts.length; i++) {
                if (i > 0) sb.append(", '\"', ");
                sb.append("'").append(parts[i].replace("'", "''")).append("'");
            }
            sb.append(")");
            return sb.toString();
        }
    }

    private static String nodeToString(Node node) throws Exception {
        TransformerFactory tf = TransformerFactory.newInstance();
        tf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        Transformer transformer = tf.newTransformer();
        transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
        StringWriter writer = new StringWriter();
        transformer.transform(new DOMSource(node), new StreamResult(writer));
        return writer.toString();
    }

    private static void writeFile(String path, String content) throws IOException {
        Files.write(Paths.get(path), content.getBytes(StandardCharsets.UTF_8));
    }

    public static void main(String[] args) throws Exception {
        if (args.length >= 2) {
            String xmlPath = args[0];
            String idVal = args[1];
            List<String> res = queryById(xmlPath, idVal);
            for (String s : res) {
                System.out.println(s);
            }
            return;
        }

        String[] files = {
            "sample97_1.xml",
            "sample97_2.xml",
            "sample97_3.xml",
            "sample97_4.xml",
            "sample97_5.xml"
        };
        String[] contents = {
            "<tag id=\"1\">Alpha</tag>",
            "<tag id=\"abc\">Bravo</tag>",
            "<tag id=\"he said &quot;hi&quot; and 'yo'\">Charlie</tag>",
            "<tag id=\"no-match\">Delta</tag>",
            "<tag id=\"5\"><child>Echo</child></tag>"
        };
        for (int i = 0; i < files.length; i++) writeFile(files[i], contents[i]);

        List<String> r1 = queryById(files[0], "1");
        List<String> r2 = queryById(files[1], "abc");
        List<String> r3 = queryById(files[2], "he said \"hi\" and 'yo'");
        List<String> r4 = queryById(files[3], "x");
        List<String> r5 = queryById(files[4], "5");

        System.out.println("Test1: " + r1);
        System.out.println("Test2: " + r2);
        System.out.println("Test3: " + r3);
        System.out.println("Test4: " + r4);
        System.out.println("Test5: " + r5);
    }
}