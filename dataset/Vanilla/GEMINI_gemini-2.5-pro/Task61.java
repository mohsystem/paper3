import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import java.io.StringReader;

public class Task61 {

    /**
     * Parses an XML string to find the root element's name.
     * @param xmlString The string containing the XML document.
     * @return The name of the root element, or an error message if parsing fails.
     */
    public static String getRootElement(String xmlString) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            InputSource is = new InputSource(new StringReader(xmlString));
            Document doc = builder.parse(is);
            return doc.getDocumentElement().getNodeName();
        } catch (Exception e) {
            // In case of a parsing error, return an informative message.
            return "Error parsing XML";
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "<root><child/></root>",
            "<bookstore category=\"COOKING\"><book/></bookstore>",
            "<?xml version=\"1.0\"?><note><to>Tove</to></note>",
            "  <data>  </data>  ",
            "<employees><employee><id>1</id></employee></employees>"
        };

        System.out.println("Java Test Cases:");
        for (String xml : testCases) {
            System.out.println("Input: " + xml);
            System.out.println("Root Element: " + getRootElement(xml));
            System.out.println("---");
        }
    }
}