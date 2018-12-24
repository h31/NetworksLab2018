let server = HTTPServer(port: 80)

server.addHandler(method: .get, path: "/") { queries, headers, body, response in
    response(.ok,
             ["1": "2"],
             """
            <html>
              <head>
                <title>Welcome</title>
              </head>
              <body>
                <p>He<strong>ll</strong>0.</p>
              </body>
            </html>
            """
    )
}

server.addHandler(method: .post, path: "/convert") { queries, headers, body, response in
    guard headers["Content-Type"] == "application/xml" else {
        response(.badRequest, [:], "")
        return
    }
    
    var yaml: String
    do {
        yaml = try XMLToYAMLConverter.convertToYAML(fromXML: body)
    } catch _ {
        response(.badRequest, [:], "")
        return
    }
    
    response(.ok, [:], yaml)
}



server.run()
