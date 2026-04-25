#ifndef LEGO_SERVER_HPP
#define LEGO_SERVER_HPP
#include <algorithm>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "EzTypes.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"
#include "Socket.h"
using namespace ez;
extern int PORT4SSL;
extern int PORT4;
extern int PORT6SSL;
extern int PORT6;
extern size_t BUFFER_SIZE;
extern int BACKLOG_QUEUE;
extern uint32_t DEFAULT_DECISIONS_INODE;

class Lego_Server {
 public:
  // Constructor y Destructor
  Lego_Server(const char* host, const char* lego_ip, int queue_capacity,
              ez::FileSystem&, std::string);
  ~Lego_Server();

  int cicloServer();

  void initializeFileSystem(std::string);
  void populateFileSystem();

 private:
  std::atomic<bool> stopFlag = false;
  VSocket* server_socket = nullptr;
  std::string host;
  int ipVer;
  int queue_capacity;
  std::mutex ezy_mutex;
  ez::FileSystem& ezy_file;
  std::vector<std::jthread> threads;
  std::string errorBody =
      "<h1>404 Not Found</h1><p>La figura o parte no existe en el "
      "sistema.</p>";
  std::string badReq =
      "<h1>400 Bad Request</h1><p>Este servidor requiere HTTP/1.1</p>";
  // FileSystem ezy_file;

  static void handleSignal(int signal);
  void acceptLoop(std::stop_token stop_token, VSocket* server, int);
  void connectionHandling(std::stop_token stop_token, VSocket* client);
  std::string build_HttpResponse(std::string, uint32_t length,
                                 std::string type_conect,
                                 std::string contentType, std::string& content);
  int BindPortAvailable(VSocket* socket, int startPort);
  void populateFigure(std::string directory, std::string figuras1,
                      std::string figuras2);
  std::pair<std::string, int> parseLegoRequest(const std::string& stringData);
  void populateLogFile();
  void registrarEvento(Entidad e, TipoLog t, const std::string& m);
  int HTTPver(const std::string& stringData,
              const std::string& versionEsperada);
  void replace(std::string& target, const std::string& placeholder,
               const std::string& value);
  std::string buildFigurePage(
      const std::string& figureName,
      std::string content);  // std::stop_token stop_token
  const std::string LOG_TEMPLATE = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Lego Server Logs</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      background-color: #f5f0e8;
      font-family: 'Segoe UI', sans-serif;
      color: #2c2c2c;
      padding: 2rem;
      max-width: 900px;
      margin: 0 auto;
    }

    h1 {
      font-size: 2.5rem;
      color: #d62828;
      border-bottom: 4px solid #d62828;
      padding-bottom: 0.5rem;
      margin-bottom: 1.5rem;
    }

    table {
      width: 100%;
      border-collapse: collapse;
      background: #fff;
      border-radius: 6px;
      overflow: hidden;
      box-shadow: 0 2px 8px rgba(0,0,0,0.1);
    }

    thead {
      background: #d62828;
      color: white;
    }

    th {
      padding: 0.8rem 1rem;
      text-align: left;
      font-size: 0.9rem;
      text-transform: uppercase;
      letter-spacing: 0.05em;
      background: #d62828;
      color: white;
    }

    td {
      padding: 0.6rem 1rem;
      border-bottom: 1px solid #eee;
      background: #fff;
    }

    tr:last-child td { border-bottom: none; }

    tr:nth-child(even) td { background: #faf7f2; }

    td:first-child {
      font-weight: bold;
      color: #f77f00;
      width: 100px;
    }
  </style>
</head>
<body>
  <h1>Logs</h1>
  <table>
    <thead>
      <tr>
        <th>Type</th>
        <th>Description</th>
      </tr>
    </thead>
    <tbody>
      {{logContent}}
    </tbody>
  </table>
</body>
</html>)HTML";

  const std::string FIGURES_TEMPLATE = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Lego Server Figure</title>
  <style>
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }

    body {
      background-color: #f5f0e8;
      font-family: 'Segoe UI', sans-serif;
      color: #2c2c2c;
      padding: 2rem;
      max-width: 800px;
      margin: 0 auto;
    }

    h1 {
      font-size: 2.5rem;
      text-transform: capitalize;
      color: #d62828;
      border-bottom: 4px solid #d62828;
      padding-bottom: 0.5rem;
      margin-bottom: 1.5rem;
    }

    h2 {
      font-size: 1.2rem;
      color: #f77f00;
      margin: 1.2rem 0 0.5rem;
      text-transform: uppercase;
      letter-spacing: 0.05em;
    }

    h3 {
      font-size: 1.5rem;
      color: #000;
      margin: 1.4;
      padding: 1rem;
    }

    ul {
      list-style: none;
      padding: 0;
    }

    li {
      background: #fff;
      border-left: 4px solid #f77f00;
      padding: 0.5rem 1rem;
      margin-bottom: 0.4rem;
      border-radius: 4px;
      box-shadow: 0 1px 3px rgba(0,0,0,0.08);
    }

    .total {
      margin-top: 2rem;
      background: #d62828;
      color: white;
      padding: 0.8rem 1.2rem;
      border-radius: 6px;
      font-size: 1.1rem;
      font-weight: bold;
    }
  </style>
</head>
<body>
  <h1>{{figureName}}</h1>
  {{figurePartList}}
  <h2 class="total">{{totalPieceQuantity}}</h2>
</body>
</html>)HTML";
};

#endif  // LEGO_SERVER_HPP
