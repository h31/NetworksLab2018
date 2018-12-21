import java.io.*;
import java.net.*;
import java.util.logging.Level;
import java.util.logging.Logger;


/**
 * This is a handler of http connection
 */
public class HttpConnectionHandler implements Runnable {
    private Socket socket;
    private static Logger logger;


    /**
     * A little constructor
     * @param s socket for the connection
     */
    HttpConnectionHandler(Socket s) {
        socket = s;
        logger = Logger.getLogger(HttpConnectionHandler.class.getName());
    }

    /**
     * A point of communication with a client
     */
    public void run() {
        try {
            // Get the I/O streams for the socket
            InputStream in = socket.getInputStream();
            OutputStream out = socket.getOutputStream();

            // Create an empty request
            HttpRequest request = new HttpRequest();

            try {
                // Read request information
                request.read(new BufferedReader(new InputStreamReader(in)));

                // Get the request method
                String method = request.getMethod().toUpperCase();

                // Choose an handler for the method of the request
                switch (method) {
                    case "GET":
                        new HttpRequestHandler().get(request, out);
                    case "POST":
                        new HttpRequestHandler().post(request, out);
                        break;
                    case "HEAD":
                        new HttpRequestHandler().head(request, out);
                        break;
                    default:
                        out.write(new HttpResponse().sendError(HttpResponse.SC_NOT_IMPLEMENTED).getBytes());
                }

                logger.info(request.toString());
            } catch (Exception e) {
                out.write(new HttpResponse().sendError(HttpResponse.SC_BAD_REQUEST).getBytes());
                logger.log(Level.SEVERE, e.getMessage(), e);
            }
            out.flush();
            out.close();
            socket.close();
        } catch (IOException e) {
            logger.log(Level.SEVERE, e.getMessage(), e);
        }

    }

}