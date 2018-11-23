#include "Client.h"
#include "Server.h"

DB *Server::db;

Client::Client() = default;

Client::~Client() {
    BOOST_LOG_TRIVIAL(debug) << "Client destruction";
    Server::deleteClient(this);
}

int Client::acceptConnection(int serverSock) {
    unsigned int clilen = sizeof(cli_addr);
    sockfd = accept(serverSock, (struct sockaddr *) &cli_addr, &clilen);
    return sockfd;
}

void Client::handler() {
    try {
        while (true) {
            //запрос логина и пароля
            char *login = readAll();
            char *pass = readAll();
            BOOST_LOG_TRIVIAL(info) << "Authorization attempt; login = " << login;
            BOOST_LOG_TRIVIAL(debug) << "password = " << pass;

            id = Server::db->findUser(login, pass, name, isClient);
            //отправка меню или сообщение об ошибке
            if (strcmp(id, "-1") == 0) {
                sendAll("-1");
                sendAll("Пользователь не найден");
                continue;
            }
            break;
        }
        sendAll("Добро пожаловать, " + name);

        if (isClient) {
            handleClient();
        } else {
            handleAdmin();
        }

        delete this;
    } catch (const runtime_error& ex) {
        BOOST_LOG_TRIVIAL(error) << ex.what();
        BOOST_LOG_TRIVIAL(error) << "Client " << name << " was unexpectedly disconnected";
        delete this;
    }
}

bool Client::operator==(const Client &client) {
    return cli_addr.sin_addr.s_addr == client.cli_addr.sin_addr.s_addr && cli_addr.sin_port == client.cli_addr.sin_port;
}

void Client::handleAdmin() {
    while (true) {
        string menu = "\nВы вошли в систему как администратор. Выберете дальнейшее действие\n"
                      "1. Отображение истории показаний счетчика для заданного абонента;\n"
                      "2. Отображение счетчиков и Абонентов, которые не вносили показания на заданное число;\n"
                      "3. Выход из системы;\n";
        sendAll(menu);

        char *num = readAll();
        if (!isdigit(*num)) {
            sendAll("Неверный ввод, введите число");
            continue;
        }

        int option = atoi(num);
        switch (option) {
            case 1: {
                sendAll("1. Отображение истории показаний счетчика для заданного абонента");
                sendAll("Введите номер Абонента и через пробел число месяцев для просмотра (0 для просмотра всей истории)");
                char *loginAndMonth = readAll();
                //валидируем
                string temp(loginAndMonth);
                string month = temp.substr(temp.find(' ') + 1, temp.length() - temp.find(' '));
                string login = temp.substr(0, temp.find(' '));

                if(login.length() <= 30 && login.length() > 0 && month.length() > 0 && month.length() <= 3 && isdigit(*month.c_str())){
                    const char* clientId = Server::db->getClientItByName(login.c_str());
                    if(strcmp(clientId, "-1") == 0){
                        sendAll("Клиент не найден");
                    } else {
                        string *result = new string("");
                        if (Server::db->getData(clientId, month.c_str(), result) != -1) {
                            //достаем данные
                            sendAll(*result);
                        } else {
                            sendAll("На сервере возникли неполадки, пожалуйста попробуйте позже");
                        }
                        delete result;
                    }
                } else {
                    sendAll("Неверный формат ввода");
                }
                break;
            }
            case 2: {
                sendAll("2. Отображение счетчиков и Абонентов, которые не вносили показания на заданное число");
                sendAll("Введите мясяц и год в формате ММ.ГГГГ");
                string monthAndYear = readAll();
                string monthStr = monthAndYear.substr(0, monthAndYear.find('.'));
                string yearStr = monthAndYear.substr(monthAndYear.find('.') + 1,
                                                     monthAndYear.length() - monthAndYear.find('.'));
                if (validateMonthAndYear(monthStr, yearStr) != -1) {
                    string *result = new string("");
                    if (Server::db->getDefaulters(monthStr.c_str(), yearStr.c_str(), result) != -1) {
                        //достаем данные
                        sendAll(*result);
                    } else {
                        sendAll("На сервере возникли неполадки, пожалуйста попробуйте позже");
                    }
                    delete result;
                } else {
                    sendAll("Неверный формат ввода");
                }
                break;
            }
            case 3: {
                BOOST_LOG_TRIVIAL(debug) << "Client " << name << "is disconnected";
                sendAll("-1");
                return;
            }
            default: {
                sendAll("Неверный ввод, введите число в заданном пределе");
                continue;
            }
        }
    }
}

void Client::handleClient() {
    while (true) {
        string menu = "\nВы вошли в систему как пользователь. Выберете дальнейшее действие\n"
                      "1. Фиксация показаний счетчиков на заданное число;\n"
                      "2. Отображение истории показаний счетчика;\n"
                      "3. Выход из системы;\n";
        sendAll(menu);

        char *num = readAll();
        if (!isdigit(*num)) {
            sendAll("Неверный ввод, введите число");
            continue;
        }

        int option = atoi(num);
        switch (option) {
            case 1: {
                sendAll("1. Фиксация показаний счетчиков на заданное число");
                sendAll("Введите месяц и год показаний в формате ММ.ГГГГ, а затем через пробел показания счетчика, 6 цифр");
                //валидируем
                string meterData = readAll();
                string data = meterData.substr(meterData.find(' ') + 1, meterData.length() - meterData.find(' '));

                string monthAndYear = meterData.substr(0, meterData.find(' '));
                string monthStr = monthAndYear.substr(0, monthAndYear.find('.'));
                string yearStr = monthAndYear.substr(monthAndYear.find('.') + 1,
                                                     monthAndYear.length() - monthAndYear.find('.'));
                if (validateMonthAndYear(monthStr, yearStr) != -1 && isdigit(*data.c_str()) && data.length() == 6) {
                    //сохраняем
                    if (Server::db->saveData(id, monthStr, yearStr, data.c_str()) != -1) {
                        sendAll("Показания успешно сохранены");
                    } else {
                        sendAll("На сервере возникли неполадки, пожалуйста попробуйте позже");
                    }
                } else {
                    sendAll("Неверный формат ввода");
                }
                break;
            }
            case 2: {
                sendAll("2. Отображение истории показаний счетчика");
                sendAll("Введите количество месяцев для просмотра. Введите 0 для просморта всей истории.");
                const char* monthToShow = readAll();

                if (isdigit(*monthToShow) && atoi(monthToShow) >= 0) {
                    string *result = new string("");
                    if (Server::db->getData(id, monthToShow, result) != -1) {
                        //достаем данные
                        sendAll(*result);
                    } else {
                        sendAll("На сервере возникли неполадки, пожалуйста попробуйте позже");
                    }
                    delete result;
                } else {
                    sendAll("Неверный формат ввода");
                }

                break;
            }
            case 3: {
                BOOST_LOG_TRIVIAL(debug) << "Client " << name << "was disconnected";
                sendAll("-1");
                return;
            }
            default: {
                sendAll("Неверный ввод, введите число в заданном пределе");
                continue;
            }
        }
    }
}

int Client::validateMonthAndYear(string monthStr, string yearStr) {
    if (!isdigit(*monthStr.c_str()) || !isdigit(*yearStr.c_str()) || monthStr.length() != 2 || yearStr.length() != 4) {
        return -1;
    }
    int month = stoi(monthStr);
    int year = stoi(yearStr);

    time_t now = time(NULL);
    struct tm *currentTime = localtime(&now);
    int currentYear = currentTime->tm_year + 1900;
    if (month < 1 || month > 12 || year < 2000 || year > currentYear) {
        return -1;
    }

    struct tm timeToValidate{};
    timeToValidate.tm_year = year - 1900;
    timeToValidate.tm_mon = month - 1;
    double seconds = difftime(now, mktime(&timeToValidate));
    if (seconds < 0) {
        return -1;
    }
}
