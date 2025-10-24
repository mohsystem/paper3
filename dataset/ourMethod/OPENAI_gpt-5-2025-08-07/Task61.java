import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import java.io.StringReader;

public class Task61 {

    public static String getRootElement(String xml) {
        if (xml == null) {
            return "";
        }
        final int MAX_LEN = 1_000_000;
        if (xml.length() == 0 || xml.length() > MAX_LEN) {
            return "";
        }
        try {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            dbf.setNamespaceAware(true);
            // Secure processing and XXE protections
            dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
            dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            dbf.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            dbf.setXIncludeAware(false);
            dbf.setExpandEntityReferences(false);

            DocumentBuilder db = dbf.newDocumentBuilder();
            // Block any entity resolution
            db.setEntityResolver(new EntityResolver() {
                @Override
                public InputSource resolveEntity(String publicId, String systemId) {
                    return new InputSource(new StringReader(""));
                }
            });

            InputSource is = new InputSource(new StringReader(xml));
            Document doc = db.parse(is);
            Element root = doc.getDocumentElement();
            if (root == null) {
                return "";
            }
            String tag = root.getTagName();
            return tag == null ? "" : tag;
        } catch (ParserConfigurationException | SAXException | IllegalArgumentException | java.io.IOException e) {
            return "";
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "<root><child/></root>",
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?><a attr=\"1\">text</a>",
            "  <!--comment-->\n<store><item/></store>",
            "<ns:books xmlns:ns=\"urn:x\"><ns:book/></ns:books>",
            ""
        };
        for (String t : tests) {
            String res = getRootElement(t);
            System.out.println(res);
        }
    }
}