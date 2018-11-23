#include "Client.h"

Client::Client(char *hostname, char *port) {
    uint16_t portno = (uint16_t) atoi(port);
    struct hostent *server;

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        closeAndExit("ERROR opening socket", 1);
    }

    server = gethostbyname(hostname);

    if (server == NULL) {
        closeAndExit("ERROR, no such host\n", 0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        closeAndExit("ERROR connecting", 1);
    }
}

Client::~Client() {
    cout << "Client destuction\n";
}

void Client::handler() {

    //запрос логина и пароля
    while (!doLogin()) {

    }

    cin.ignore();
    while (true) {
        BOOST_LOG_TRIVIAL(info) << readAll();
        string option;
        getline(cin, option);
        sendAll(option);
        char *response = readAll();
        if (strcmp(response, "-1") == 0) {
            BOOST_LOG_TRIVIAL(info) << "Соединение с сервером было разорвано";
            break;
        }
        BOOST_LOG_TRIVIAL(info) << response;
    }

    closeAndExit("", 0);
}

void Client::closeAndExit(string errorMsg, int errorCode) {
    cerr << errorMsg << "\n";
    delete this;
    exit(errorCode);
}

bool Client::validate(char *str) {
    return regex_match(str, regex("[a-zA-Z0-9_]{4,19}$"));
}

bool Client::doLogin() {
    char *login = new char[256]();
    char *pass = new char[256]();
    BOOST_LOG_TRIVIAL(info) << "Введите логин и пароль\n" << "Логин: ";
    cin >> login;
    if (validate(login)) {
        BOOST_LOG_TRIVIAL(debug) << "Login valid";
    } else {
        BOOST_LOG_TRIVIAL(info) << "Login not valid";
        return false;
    }

    BOOST_LOG_TRIVIAL(info) << "\nПароль: ";
    cin >> pass;
    if (validate(pass)) {
        BOOST_LOG_TRIVIAL(debug) << "Password valid";
    } else {
        BOOST_LOG_TRIVIAL(info) << "Password not valid";
        return false;
    }

    sendAll(login);
    delete[] login;
    sendAll(pass);
    delete[] pass;

    char *response = readAll();
    if (atoi(response) == -1) {
        BOOST_LOG_TRIVIAL(info) << readAll();
        return false;
    }

    BOOST_LOG_TRIVIAL(info) << response;
    return true;

}
