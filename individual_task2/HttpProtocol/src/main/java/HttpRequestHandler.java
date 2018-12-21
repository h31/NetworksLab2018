import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.net.URISyntaxException;
import java.nio.file.Files;

/**
 * This class is responsible for executing the query
 */
public class HttpRequestHandler {

    /**
     * This is a GET request handler
     * @param request - received request
     * @param out - output stream
     * @throws IOException
     */
    public void get(HttpRequest request, OutputStream out) throws IOException {
        HttpResponse response = new HttpResponse();

        String uri = request.getRequestURI().substring(1);
        //Get file from resources folder
        try {
            ClassLoader classLoader = getClass().getClassLoader();
            File file = new File(classLoader.getResource(uri).toURI());
            if(request.getHeader("Accept").contains("image/*")){
                response.setContentType("image/png");

                out.write(response.sendResponseHeader(200, file.length()).getBytes());
                Files.copy(file.toPath(), out);
                return;
            }
            out.write(response.send(HttpResponse.SC_OK, Files.readAllBytes(file.toPath())).getBytes());
        } catch (NullPointerException e) {
            e.getStackTrace();
            out.write(response.sendError(HttpResponse.SC_NOT_FOUND).getBytes());
        } catch (URISyntaxException e) {
            e.printStackTrace();
            out.write(response.sendError(HttpResponse.SC_INTERNAL_ERROR).getBytes());
        }
    }

    /**
     * This is a HEAD request handler
     * @param request - received request
     * @param out - output stream
     * @throws IOException
     */
    public void head(HttpRequest request, OutputStream out) throws IOException {
        HttpResponse response = new HttpResponse();

        String uri = request.getRequestURI().substring(1);
        //Get file from resources folder
        try {
            ClassLoader classLoader = getClass().getClassLoader();
            File file = new File(classLoader.getResource(uri).toURI());
            if(request.getHeader("Accept").contains("image/*")){
                response.setContentType("image/png");

                out.write(response.sendResponseHeader(200, file.length()).getBytes());
                return;
            }
            out.write(response.sendResponseHeader(HttpResponse.SC_OK, 0).getBytes());
        } catch (NullPointerException e) {
            e.getStackTrace();
            out.write(response.sendResponseHeader(HttpResponse.SC_NOT_FOUND, 0).getBytes());
        } catch (URISyntaxException e) {
            e.printStackTrace();
            out.write(response.sendResponseHeader(HttpResponse.SC_INTERNAL_ERROR, 0).getBytes());
        }
    }

    /**
     * This is a POST request handler
     * @param request - received request
     * @param out - output stream
     * @throws IOException
     */
    public void post(HttpRequest request, OutputStream out) throws IOException {
        if(request.getRequestURI().equals("/analyzeForm") &&
                request.getQueryPost().getValue("login").equals("admin") &&
                request.getQueryPost().getValue("password").equals("password")){
            out.write(new HttpResponse().sendOK().getBytes());
        } else {
            out.write(new HttpResponse().sendError(404).getBytes());
        }
    }
}
