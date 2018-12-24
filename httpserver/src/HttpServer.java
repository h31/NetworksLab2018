import java.io.*;
import java.net.*;
import java.text.DateFormat;
import java.util.Date;
import java.util.TimeZone;

class HttpServer extends Thread {
    private Socket s;

    public static void main(String args[]) {
        try {
            // сокет на локалхост, порт 80
            ServerSocket server = new ServerSocket(80, 0,
                    InetAddress.getByName("localhost"));

            System.out.println("server is started");

            // слушаем порт
            while (true) {
                // ждём нового подключения и в новый поток его
                new HttpServer(server.accept());
            }
        } catch (Exception e) {
            System.out.println("init error: " + e);
        }
    }

    public HttpServer(Socket s) {
        this.s = s;
        setDaemon(true);
        setPriority(NORM_PRIORITY);
        start();
    }

    public void run() {
        try {
            InputStream is = s.getInputStream();
            OutputStream os = s.getOutputStream();

            byte buf[] = new byte[64 * 2048];
            int r = is.read(buf);
            String request = "";
            if (r > 0)
                request = new String(buf, 0, r);

            String data = getData(request);
            System.out.println("DATA" + data);
            if (data == null) {
                String response = "HTTP/1.1 400 Bad Request\n";
                DateFormat df = DateFormat.getTimeInstance();
                df.setTimeZone(TimeZone.getTimeZone("GMT"));
                response = response + "Date: " + df.format(new Date()) + "\n";
                response = response
                        + "Connection: close\n"
                        + "Server: HttpServer\n"
                        + "Pragma: no-cache\n\n";
                os.write(response.getBytes());
                s.close();
                return;
            }
            // MIME по умолчанию - "text/plain"
            String mime = "text/plain";

            r = data.lastIndexOf(".");
            if (r > 0) {
                String ext = data.substring(r + 1);
                if (ext.equalsIgnoreCase("html"))
                    mime = "text/html";
                else if (ext.equalsIgnoreCase("htm"))
                    mime = "text/html";
                else if (ext.equalsIgnoreCase("gif"))
                    mime = "image/gif";
                else if (ext.equalsIgnoreCase("jpg"))
                    mime = "image/jpeg";
                else if (ext.equalsIgnoreCase("jpeg"))
                    mime = "image/jpeg";
                else if (ext.equalsIgnoreCase("bmp"))
                    mime = "image/x-xbitmap";
            }
            String subs = data.substring(0, 4);
            if (subs.equals("POST")) {
                String response = "HTTP/1.1 200 OK\n";
                DateFormat df = DateFormat.getTimeInstance();
                df.setTimeZone(TimeZone.getTimeZone("GMT"));
                response = response + "Content-Length: " + data.length() + "\n";
                response = response + "Content-Type: " + mime + "\n";
                response = response
                        + "Connection: close\n"
                        + "Server: HttpServer\n\n";
                response += data;
                os.write(response.getBytes());

                return;
            }
            if (subs.equals("HEAD")) {
                data = data.substring(4);
                File f = new File(data);
                String response = "HTTP/1.1 200 OK\n";
                DateFormat df = DateFormat.getTimeInstance();
                df.setTimeZone(TimeZone.getTimeZone("GMT"));
                response = response + "Last-Modified: " + df.format(new Date(f.lastModified())) + "\n";
                response = response + "Content-Length: " + f.length() + "\n";
                response = response + "Content-Type: " + mime + "\n";
                response = response
                        + "Connection: close\n"
                        + "Server: HttpServer\n\n";

                // вывод заголовка:
                os.write(response.getBytes());
                return;
            }


            // Если ничего не указано, то index.html
            File f = new File(data);
            boolean flag = !f.exists();
            if (!flag) if (f.isDirectory()) {
                if (data.lastIndexOf("" + File.separator) == data.length() - 1)
                    data = data + "index.html";
                else
                    data = data + File.separator + "index.html";
                f = new File(data);
                flag = !f.exists();
            }

            // "404 Not Found"
            if (flag) {
                String response = "HTTP/1.1 404 Not Found\n";
                DateFormat df = DateFormat.getTimeInstance();
                df.setTimeZone(TimeZone.getTimeZone("GMT"));
                response = response + "Date: " + df.format(new Date()) + "\n";
                response = response
                        + "Content-Type: text/plain\n"
                        + "Connection: close\n"
                        + "Server: HttpServer\n"
                        + "Pragma: noa-cache\n\n";

                response = response + "File " + data + " not found!";
                os.write(response.getBytes());
                s.close();

                // выход
                return;
            }

            // создаём ответ
            String response = "HTTP/1.1 200 OK\n";
            DateFormat df = DateFormat.getTimeInstance();
            df.setTimeZone(TimeZone.getTimeZone("GMT"));
            response = response + "Last-Modified: " + df.format(new Date(f.lastModified())) + "\n";
            response = response + "Content-Length: " + f.length() + "\n";
            response = response + "Content-Type: " + mime + "\n";
            response = response
                    + "Connection: close\n"
                    + "Server: HttpServer\n\n";

            // вывод заголовка:
            os.write(response.getBytes());

            // вывод файла:
            FileInputStream fis = new FileInputStream(data);
            r = 1;
            while (r > 0) {
                r = fis.read(buf);
                if (r > 0) os.write(buf, 0, r);
            }
            fis.close();

            s.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    protected String getData(String header) {
        String URI = extract(header, "GET ", " "), path;
        if (URI == null) {
            URI = extract(header, "POST ", " ");
            if (URI != null)
                return header + "\n" + "Query \n" + this.getPOST(URI);
            else {
                URI = extract(header, "HEAD ", " ");
                return getHEAD(URI);
            }
        }

        if (URI == null) return null;

        // парсинг URL, работает для GET запроса
        path = URI.toLowerCase();
        return parseURL(path, URI);
    }

    public String parseURL(String path, String URI) {
        if (path.indexOf("http://", 0) == 0) {
            URI = URI.substring(7);
            URI = URI.substring(URI.indexOf("/", 0));
        } else if (path.indexOf("/", 0) == 0)
            URI = URI.substring(1);
        int i = URI.indexOf("?");
        if (i > 0) URI = URI.substring(0, i);
        i = URI.indexOf("#");
        if (i > 0) URI = URI.substring(0, i);
        path = "C:\\Users\\User\\IdeaProjects\\httpserver\\src" + File.separator;
        char a;
        for (i = 0; i < URI.length(); i++) {
            a = URI.charAt(i);
            if (a == '/')
                path = path + File.separator;
            else
                path = path + a;
        }
        return path;
    }


    // Для получения вида запроса
    protected String extract(String str, String start, String end) {
        int s = str.indexOf("\n\n", 0), e;
        if (s < 0) s = str.indexOf("\r\n\r\n", 0);
        if (s > 0) str = str.substring(0, s);
        s = str.indexOf(start, 0) + start.length();
        if (s < start.length()) return null;
        e = str.indexOf(end, s);
        if (e < 0) e = str.length();
        return (str.substring(s, e)).trim();
    }

    // Обработка POST запроса
    public String getPOST(String URI) {
        System.out.println("I've got POST request " + URI + '\n');
        char a;
        String key = "";
        String val = "";
        String result = "";
        boolean isVal = false;
        int ind = URI.indexOf("?", 0);
        if (ind > 0) {
            for (int i = ind + 1; i < URI.length(); i++) {
                a = URI.charAt(i);
                if (a != '&') {
                    if ((a == '=')) {
                        isVal = !isVal;
                    } else {
                        if (isVal) {
                            val = val + a;
                        } else {
                            key = key + a;
                        }
                    }
                } else {
                    result += "It has Query params: \n"
                            + key + " = " + val + "\n";
                    key = "";
                    val = "";
                }
                if (i == URI.length() - 1) {
                    result += "It has Query params: \n"
                            + key + " = " + val + "\n";
                    System.out.println(result);
                }
            }
        }
        if (result.isEmpty()) {
            result += "is empty \n";
        }
        return result;
    }

    // Обработка HEAD запроса
    public String getHEAD(String URI) {
        String result = "";
        System.out.println("I'VE GOT HEAD REQUEST \n");
        result = URI.toLowerCase();
        result = parseURL(result, URI);
        result = "HEAD" + result;
        return result;
    }
}