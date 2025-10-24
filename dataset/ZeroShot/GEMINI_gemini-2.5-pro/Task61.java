import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import java.io.StringReader;

public class Task61 {

    /**
     * Parses the provided XML string to find the root element's name.
     * This implementation uses JAXP (Java API for XML Processing) and is configured
     * to be secure against XXE (XML External Entity) attacks by disabling DTDs
     * and external entity resolution.
     *
     * @param xmlString The XML content as a string.
     * @return The name of the root element, or an error message if parsing fails.
     */
    public static String getRootElementName(String xmlString) {
        try {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            
            // Secure processing: Disable DTDs and external entities to prevent XXE attacks.
            // This is the OWASP recommended configuration.
            dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
            dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            dbf.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            dbf.setXIncludeAware(false);
            dbf.setExpandEntityReferences(false);
            
            DocumentBuilder builder = dbf.newDocumentBuilder();
            
            InputSource is = new InputSource(new StringReader(xmlString.trim()));
            Document doc = builder.parse(is);
            
            return doc.getDocumentElement().getNodeName();
        } catch (Exception e) {
            // If the parser is configured to disallow DTDs, it will throw an exception
            // for XML with a DOCTYPE declaration. This is expected and secure behavior.
            return "Error parsing XML: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "<root><child/></root>",
            "<notes date=\"2023-10-27\"><note>text</note></notes>",
            "<?xml version=\"1.0\"?><book><title>XML</title></book>",
            " <!-- comment --><data> <item/> </data>",
            "<!DOCTYPE note SYSTEM \"Note.dtd\"><note><to>Tove</to></note>" // This will fail as DTDs are disallowed
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input XML: " + testCases[i]);
            String rootElement = getRootElementName(testCases[i]);
            System.out.println("Root Element: " + rootElement);
            System.out.println();
        }
    }
}