import java.io.*;
import java.net.*;
import java.util.*;

/**
 * Common class for http request and response
 */
public abstract class HttpMessage {
    protected byte[] content;
    NameValueMap headers;

    /**
     * This is default Constructor
     */
    HttpMessage() {
        content = null;
        headers = NameValueMap.createNameValueMap();
    }


    /**
     * This method for getting request (or response) content (body)
     * @return content
     */
    public byte[] getContent() {
        return content;
    }


    /**
     * Method returns request (or response) content length
     * @return status of operation, -1 - exception
     */
    int getContentLength() {
        try {
            return Integer.parseInt(getHeader("Content-Length").trim());
        } catch (Exception e) {
            return -1;
        }
    }


    /**
     * @param name from pair name=value
     * @return value of the header
     */
    String getHeader(String name) {
        return headers.getValue(name);
    }

    /**
     * @return names of the headers
     */
    List<String> getHeaderNames() {
        return headers.getNames();
    }


    /**
     * This method is used for reading data from some input stream
     * @param in - input stream
     * @throws IOException
     * @throws URISyntaxException
     */
    public abstract void read(BufferedReader in) throws IOException, URISyntaxException;


    /**
     * This method is used for reading request (or response) content (body)
     * @param in - input stream
     * @throws IOException
     */
    public void readContent(BufferedReader in) throws IOException {
        int contentLength = getContentLength();

        if (contentLength > 0) {
            char[] charContent = new char[contentLength];
            if (in.read(charContent) != -1) {
                content = new String(charContent).getBytes();
            }
        }
    }


    /**
     * This is constructor for request (or response) content
     * @param content
     */
    public void setContent(byte[] content) {
        this.content = content;
        //setContentLength(content.length);
    }


    /**
     * Method for setting length of the request (response) content
     * @param contentLength int
     */
    public void setContentLength(int contentLength) {
        setHeader("Content-Length", Integer.toString(contentLength));
    }

    /**
     * Method for setting length of the request (response) content
     * @param contentLength long
     */
    public void setContentLength(long contentLength) {
        setHeader("Content-Length", Long.toString(contentLength));
    }

    /**
     * Thos method sets type of the content
     * @param type of the content
     */
    public void setContentType(String type) {
        setHeader("Content-Type", type);
    }


    /**
     * Set header into namevalue map
     * @param name name
     * @param value value
     */
    public void setHeader(String name, String value) {
        headers.put(name, value);
    }


    /**
     * Constructor for  the headers
     * @param headers
     */
    public void setHeaders(NameValueMap headers) {
        this.headers = headers;
    }
}