import Foundation
import Socket

enum HTTPStatusCode: Int {
    case ok = 200
    case badRequest = 400
    case notFound = 404
}

enum HTTPMethod: String {
    case get
    case post
    case head
    case put
    case delete
}

typealias HTTPHandler = (_ queries: [String: String], _ headers: [String: String], _ body: String,
    _ response: (_ code: HTTPStatusCode, _ headers: [String: String], _ body: String?) -> ()) -> ()

class HTTPServer {
    
    private let port: Int
    
    init(port: Int) {
        self.port = port
    }
    
    private let logger = Logger(name: "HTTP_SERVER", enabled: true)
    
    private var routes: [(method: HTTPMethod, path: String, handler: HTTPHandler)] = []
    
    private var serverSocket: Socket!
    private let bufferSize = 4096
    
    private let socketBarrierQueue = DispatchQueue(label: "com.goloviznin.eldar.second_task.socketBarrierQueue")
    private var connectedSockets: [Socket] = []
    
    func run() {
        do {
            try serverSocket = Socket.create(family: .inet)
            try serverSocket.listen(on: port)
            
            logger.log("Listening on port: \(serverSocket.listeningPort)")
            
            while true {
                let clientSocket = try serverSocket.acceptClientConnection()
                
                logger.log("Accepted connection from: \(clientSocket.remoteHostname)")
                
                handle(clientSocket: clientSocket)
            }
        } catch let error as NSError {
            logger.log(error.localizedDescription)
        }
    }
    
    func addHandler(method: HTTPMethod, path: String, handler: @escaping (HTTPHandler)) {
        routes.append((method: method, path: path, handler: handler))
    }
    
    deinit {
        serverSocket.close()
        socketBarrierQueue.sync {
            for socket in connectedSockets {
                socket.close()
            }
        }
        while !connectedSockets.isEmpty {
            Thread.sleep(forTimeInterval: 1_000)
        }
    }
    
}

// MARK: - Private `HttpServer` implementation

private extension HTTPServer {
    
    func handle(clientSocket: Socket) {
        socketBarrierQueue.sync { [unowned self] in
            self.connectedSockets.append(clientSocket)
        }
        
        DispatchQueue.global().async { [unowned self] in
            var buffer = Data(capacity: self.bufferSize)
            
            guard let _ = try? clientSocket.read(into: &buffer) else {
                self.logger.log("Can't read from: \(clientSocket.remoteHostname)")
                self.close(clientSocket: clientSocket)
                return
            }
            
            let requestString = String(data: buffer, encoding: .utf8)!
            
            guard let request = self.parseRequest(request: requestString) else {
                self.sendBadRequest(clientSocket: clientSocket)
                return
            }
            
            guard let route = self.routes.first(where: { method, path, _ in request.method == method && request.path == path }) else {
                self.sendBadRequest(clientSocket: clientSocket)
                return
            }
            
            route.handler(request.queries, request.headers, request.body ?? "") { status, headers, body in
                self.sendResponse(clientSocket: clientSocket, status: status, headers: headers, body: body ?? "")
            }
        }
    }
    
    func sendBadRequest(clientSocket: Socket) {
        sendResponse(clientSocket: clientSocket, status: .badRequest, headers: [:], body: "BAD REQUEST")
    }
    
    func sendResponse(clientSocket: Socket, status: HTTPStatusCode, headers: [String: String], body: String) {
        var response = ""
        
        response += "HTTP/1.1 \(status.rawValue) "
        switch status {
        case .ok:
            response += "OK"
            
        case .badRequest:
            response += "Bad Request"
            
        case .notFound:
            response += "Not Found"
        }
        response += "\r\n"
        
        for headerPair in headers {
            response += "\(headerPair.key): \(headerPair.value)\r\n"
        }
        response += "\r\n"
        
        response += body
        
        let _ = try? clientSocket.write(from: response)
        close(clientSocket: clientSocket)
    }
    
    func close(clientSocket: Socket) {
        clientSocket.close()
        
        socketBarrierQueue.sync { [unowned self] in
            guard let index = self.connectedSockets.firstIndex(where: { $0 === clientSocket }) else {
                return
            }
            
            self.connectedSockets.remove(at: index)
        }
    }
    
    func parseRequest(request: String) -> (method: HTTPMethod, path: String, queries: [String: String], headers: [String: String], body: String?)? {
        guard let requestLineIndex = request.range(of: " HTTP")?.lowerBound else {
            return nil
        }
        
        let requestLine = String(request[..<requestLineIndex])
        
        guard let methodIndex = requestLine.firstIndex(of: " ") else {
            return nil
        }
        
        let stringMethod = String(requestLine[..<methodIndex]).lowercased()
        
        guard let method = HTTPMethod(rawValue: stringMethod) else {
            return nil
        }
        
        guard let pathStartIndex = requestLine.firstIndex(of: "/") else {
            return nil
        }
        
        var path: String
        var queries: [String: String] = [:]
        if let queryStartIndex = requestLine.firstIndex(of: "?") {
            path = String(requestLine[pathStartIndex ..< queryStartIndex])
            let queriesString = String(requestLine[requestLine.index(after: queryStartIndex) ..< requestLine.endIndex])
            queries = queriesString.split(separator: "&").reduce(into: [String: String](), { dict, pair in
                let splittedPair = pair.split(separator: "=")
                guard let key = splittedPair.first?.removingPercentEncoding, let value = splittedPair.last?.removingPercentEncoding else {
                    return
                }
                dict[key] = value
            })
        } else {
            path = String(requestLine[pathStartIndex ..< requestLine.endIndex])
        }
        
        var headers: [String: String] = [:]
        if let headersStartIndex = request.range(of: "\r\n")?.upperBound,
            let headersEndIndex = request.range(of: "\r\n\r\n")?.lowerBound {
            let headersString = String(request[headersStartIndex ..< headersEndIndex])
            headers = headersString.components(separatedBy: "\r\n").reduce(into: [String: String]()) { dict, pair in
                let splittedPair = pair.components(separatedBy: ": ")
                guard let key = splittedPair.first?.removingPercentEncoding, let value = splittedPair.last?.removingPercentEncoding else {
                    return
                }
                dict[key] = value
            }
        }
        
        var body: String?
        if let bodyStartIndex = request.range(of: "\r\n\r\n")?.upperBound {
            body = String(request[bodyStartIndex ..< request.endIndex])
        }
        
        return (method: method, path: path, queries: queries, headers: headers, body: body)
    }
    
}
