import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import java.io.StringReader;

public class Task61 {
    public static String getRootElement(String xml) {
        if (xml == null) return null;
        try {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            try {
                dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", false);
            } catch (Exception ignored) {}
            try {
                dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
            } catch (Exception ignored) {}
            try {
                dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            } catch (Exception ignored) {}
            dbf.setXIncludeAware(false);
            dbf.setExpandEntityReferences(false);
            dbf.setNamespaceAware(true);

            DocumentBuilder db = dbf.newDocumentBuilder();
            InputSource is = new InputSource(new StringReader(xml));
            Document doc = db.parse(is);
            if (doc.getDocumentElement() != null) {
                return doc.getDocumentElement().getNodeName();
            }
        } catch (Exception e) {
            return null;
        }
        return null;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "<root></root>",
            "<?xml version=\"1.0\"?><a><b/></a>",
            "  \n<!-- comment -->\n<ns:catalog attr='1'><item/></ns:catalog>",
            "<!DOCTYPE note [<!ELEMENT note (to)>]><note><to>Tove</to></note>",
            "<unclosed"
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println("Test " + (i+1) + ": " + getRootElement(tests[i]));
        }
    }
}