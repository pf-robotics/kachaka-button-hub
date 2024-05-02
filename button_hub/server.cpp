#include "server.hpp"

#include <set>

#include "command_table.hpp"
#include "data.hpp"
#include "fetch_state.hpp"
#include "mutex.hpp"
#include "server_commands.hpp"
#include "server_info.hpp"
#include "settings.hpp"
#include "to_json.hpp"
#include "types.hpp"

namespace server {

static AsyncWebServer g_server(80);
static kb::Mutex g_ws_mutex;
static AsyncWebSocket g_ws("/ws");
static std::set<AsyncWebSocketClient*> g_ws_clients;
static int g_ws_client_count = 0;

void SendToWs(const String& msg) {
  const kb::LockGuard lock(g_ws_mutex);
  for (auto& client : g_ws_clients) {
    client->text(msg);
  }
}

static void SendAllToClient(AsyncWebSocketClient* client,
                            const RobotInfoHolder& robot_info,
                            const CommandTable& command_table) {
  const kb::LockGuard lock(g_ws_mutex);
  client->text(to_json::ConvertHubInfo(g_ws_client_count));
  client->text(to_json::ConvertRobotInfo(robot_info));
  client->text(to_json::ConvertSettings(g_settings));
  client->text(to_json::ConvertObservedButtons(
      command_table.GetObservedButtons(), command_table.GetButtonNames()));
  client->text(to_json::ConvertCommands(command_table.GetCommands()));
}

static void OnWebSocketEvent(AsyncWebSocket* server,
                             AsyncWebSocketClient* client, AwsEventType type,
                             void* arg, uint8_t* data, size_t len,
                             RobotInfoHolder& robot_info,
                             const CommandTable& command_table) {
  if (type == WS_EVT_CONNECT) {
    // client connected
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    {
      const kb::LockGuard lock(g_ws_mutex);
      g_ws_clients.insert(client);
      g_ws_client_count++;
    }
    SendAllToClient(client, robot_info, command_table);
    SendToWs(to_json::ConvertHubInfo(g_ws_client_count));
    if (g_settings.GetAutoRefetchOnUiLoad()) {
      fetch_state::FetchRobotInfoThrottled(&robot_info);
    }
    return;
  }
  if (type == WS_EVT_DISCONNECT) {
    // client disconnected
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
    int removed = 0;
    {
      const kb::LockGuard lock(g_ws_mutex);
      removed = g_ws_clients.erase(client);
      g_ws_client_count--;
    }
    if (removed != 1) {
      Serial.printf("ERROR: Failed to remove client: %d\n", removed);
    }
    SendToWs(to_json::ConvertHubInfo(g_ws_client_count));
    return;
  }
  if (type == WS_EVT_ERROR) {
    // error was received from the other end
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(),
                  *reinterpret_cast<uint16_t*>(arg), data);
    return;
  }
  if (type == WS_EVT_PONG) {
    // pong message was received (in response to a ping request maybe)
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len,
                  len ? reinterpret_cast<char*>(data) : "");
    return;
  }
  if (type == WS_EVT_DATA) {
    // data packet
    auto* info = reinterpret_cast<AwsFrameInfo*>(arg);
    if (info->final && info->index == 0 && info->len == len) {
      // the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(),
                    client->id(), (info->opcode == WS_TEXT) ? "text" : "binary",
                    info->len);
      if (info->opcode == WS_TEXT) {
        data[len] = 0;
        Serial.printf("%s\n", data);
      } else {
        for (size_t i = 0; i < info->len; i++) {
          Serial.printf("%02x ", data[i]);
        }
        Serial.printf("\n");
      }
      if (info->opcode == WS_TEXT) {
        client->text("I got your text message");
      } else {
        client->binary("I got your binary message");
      }
    } else {
      // message is comprised of multiple frames or the frame is split into
      // multiple packets
      Serial.println("ERROR: Message is comprised of multiple frames");
    }
    return;
  }
}

static void RegisterReadEntry(
    AsyncWebServer& server, const char* url, WebRequestMethodComposite method,
    std::function<void(AsyncWebServerRequest*)> handler) {
  server.on(
      url, method,
      [url, handler = std::move(handler)](AsyncWebServerRequest* request) {
        Serial.printf("%s %s\n", request->methodToString(), url);
        handler(request);
      });
}

static void RegisterWriteEntry(
    AsyncWebServer& server, const char* url, WebRequestMethodComposite method,
    std::function<void(AsyncWebServerRequest*, const String&)> handler) {
  server.on(
      url, method,
      [url, handler](AsyncWebServerRequest* request) {
        Serial.printf("%s %s\n", request->methodToString(), url);
        // body-less request is not supported
        if (request->hasArg("body")) {
          const String& body = request->arg("body");
          handler(request, body);
        }
      },
      nullptr,
      [url, handler = std::move(handler)](AsyncWebServerRequest* request,
                                          uint8_t* data, size_t len,
                                          size_t index, size_t total) {
        Serial.printf("%s %s (BODY: %zu, %zu, %zu)\n",
                      request->methodToString(), url, index, len, total);
        static String buffer;  // TODO: Use a buffer pool
        if (index == 0) {
          buffer = String(data, len);
        } else {
          buffer += String(data, len);
        }
        if (index + len == total) {
          handler(request, buffer);
        }
      });
}

static void RegisterGetAndPutEntry(
    AsyncWebServer& server, const char* url_path,
    std::function<void(AsyncWebServerRequest*)> get_handler,
    std::function<void(AsyncWebServerRequest*, const String&)> put_handler) {
  RegisterReadEntry(server, url_path, HTTP_GET, std::move(get_handler));
  RegisterWriteEntry(server, url_path, HTTP_PUT, std::move(put_handler));
}

static void SetWifiHandler(AsyncWebServer& server) {
  RegisterGetAndPutEntry(
      server, "/config/wifi",
      [](AsyncWebServerRequest* request) { HandleGetWiFi(request); },
      [](AsyncWebServerRequest* request, const String& body) {
        HandleSetWiFi(request, body);
      });
  RegisterReadEntry(server, "/ota/desired_hub_version", HTTP_GET,
                    [](AsyncWebServerRequest* request) {
                      HandleGetDesiredHubVersion(request);
                    });
  RegisterWriteEntry(server, "/ota/image_url_by_version", HTTP_GET,
                     [](AsyncWebServerRequest* request, const String& body) {
                       HandleGetOtaImageUrlByVersion(request, body);
                     });
  RegisterWriteEntry(
      server, "/ota/trigger_auto", HTTP_POST,
      [](AsyncWebServerRequest* request, const String& /* body */) {
        HandleStartAutoOta(request);
      });
  RegisterWriteEntry(server, "/ota/trigger_ota_by_url", HTTP_POST,
                     [](AsyncWebServerRequest* request, const String& body) {
                       HandleOtaByImageUrl(request, body);
                     });
}

static void SetAppHandler(AsyncWebServer& server, AsyncWebSocket& ws,
                          RobotInfoHolder& robot_info,
                          CommandTable& command_table) {
  ws.onEvent([&robot_info, &command_table](
                 AsyncWebSocket* server, AsyncWebSocketClient* client,
                 AwsEventType type, void* arg, uint8_t* data, size_t len) {
    OnWebSocketEvent(server, client, type, arg, data, len, robot_info,
                     command_table);
  });
  server.addHandler(&ws);

  RegisterGetAndPutEntry(
      server, "/config/robot_host",
      [](AsyncWebServerRequest* request) { HandleGetRobotHost(request); },
      [](AsyncWebServerRequest* request, const String& body) {
        HandleSetRobotHost(request, body);
      });
  RegisterGetAndPutEntry(
      server, "/config/beep_volume",
      [](AsyncWebServerRequest* request) { HandleGetBeepVolume(request); },
      [](AsyncWebServerRequest* request, const String& body) {
        HandleSetBeepVolume(request, body);
      });
  RegisterGetAndPutEntry(
      server, "/config/screen_brightness",
      [](AsyncWebServerRequest* request) {
        HandleGetScreenBrightness(request);
      },
      [](AsyncWebServerRequest* request, const String& body) {
        HandleSetScreenBrightness(request, body);
      });
  RegisterGetAndPutEntry(
      server, "/config/auto_ota_is_enabled",
      [](AsyncWebServerRequest* request) {
        HandleGetAutoOtaIsEnabled(request);
      },
      [](AsyncWebServerRequest* request, const String& body) {
        HandleSetAutoOtaIsEnabled(request, body);
      });
  RegisterGetAndPutEntry(
      server, "/config/auto_refetch_on_ui_load",
      [](AsyncWebServerRequest* request) {
        HandleGetAutoRefetchOnUiLoad(request);
      },
      [](AsyncWebServerRequest* request, const String& body) {
        HandleSetAutoRefetchOnUiLoad(request, body);
      });

  RegisterGetAndPutEntry(
      server, "/buttons",
      [&command_table](AsyncWebServerRequest* request) {
        HandleGetObservedButtons(request, command_table);
      },
      [&command_table](AsyncWebServerRequest* request, const String& body) {
        HandleSetButtonName(request, body, command_table);
      });
  RegisterWriteEntry(
      server, "/buttons", HTTP_DELETE,
      [&command_table](AsyncWebServerRequest* request, const String& body) {
        HandleDeleteButtonName(request, body, command_table);
      });

  RegisterWriteEntry(
      server, "/commands", HTTP_POST,
      [&command_table](AsyncWebServerRequest* request, const String& body) {
        HandlePostCommand(request, body, command_table);
      });
  RegisterGetAndPutEntry(
      server, "/commands",
      [&command_table](AsyncWebServerRequest* request) {
        HandleGetCommands(request, command_table);
      },
      [&command_table](AsyncWebServerRequest* request, const String& body) {
        HandlePutCommands(request, body, command_table);
      });
  // Ideally, it should be /commands/{id}, but since ASYNCWEBSERVER_REGEX must
  // be enabled in order to use path variables, we will proceed with the policy
  // of not using variables for now.
  RegisterWriteEntry(
      server, "/commands", HTTP_DELETE,
      [&command_table](AsyncWebServerRequest* request, const String& body) {
        HandleDeleteCommand(request, body, command_table);
      });

  RegisterReadEntry(server, "/reboot", HTTP_GET,
                    [](AsyncWebServerRequest* request) {
                      request->send(203);
                      delay(100);
                      ESP.restart();
                    });
  RegisterReadEntry(server, "/clear_all_data", HTTP_GET,
                    [&command_table](AsyncWebServerRequest* request) {
                      HandleClearAllData(request, command_table);
                    });
}

static void SetCommonSettings(AsyncWebServer& server) {
  const auto register_static_content =
      [&server](const char* url_path, const uint8_t* data, const size_t size,
                const uint8_t* gz_data, const size_t gz_size,
                const char* mime_type) {
        server.on(url_path, HTTP_GET,
                  [url_path, data, size, gz_data, gz_size,
                   mime_type](AsyncWebServerRequest* request) {
                    Serial.printf("GET %s\n", url_path);
                    bool use_gz = false;
                    if (gz_data && !data) {
                      use_gz = true;
                    } else if (data && !gz_data) {
                      use_gz = false;
                    } else {
                      AsyncWebHeader* enc =
                          request->getHeader("Accept-Encoding");
                      use_gz = enc && enc->value().indexOf("gzip") >= 0;
                    }
                    auto* p = new AsyncProgmemResponse(200, mime_type,
                                                       use_gz ? gz_data : data,
                                                       use_gz ? gz_size : size);
                    if (use_gz) {
                      p->addHeader("Content-Encoding", "gzip");
                    }
                    request->send(p);
                  });
      };
  register_static_content("/", data::index_html_data, data::index_html_size,
                          nullptr, 0, "text/html");
  register_static_content("/index.css", data::index_css_data,
                          data::index_css_size, nullptr, 0, "text/css");
  register_static_content("/index.js", data::index_js_data, data::index_js_size,
                          data::index_js_gz_data, data::index_js_gz_size,
                          "text/javascript");
  register_static_content("/license.txt", nullptr, 0, data::license_txt_gz_data,
                          data::license_txt_gz_size, "text/plain");

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->send(request->method() == HTTP_OPTIONS ? 200 : 404);
  });

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers",
                                       "Content-Type");

  server.begin();
  Serial.println("HTTP server started");
}

void SetupHttpServerForWiFiSetting() {
  SetWifiHandler(g_server);
  SetCommonSettings(g_server);
}

void SetupHttpServer(RobotInfoHolder& robot_info, CommandTable& command_table) {
  SetWifiHandler(g_server);
  SetAppHandler(g_server, g_ws, robot_info, command_table);
  SetCommonSettings(g_server);
}

void Stop() {
  const kb::LockGuard lock(g_ws_mutex);
  for (auto& client : g_ws_clients) {
    client->close();
  }
  g_ws_clients.clear();
  g_ws_client_count = 0;
  g_server.end();
}

}  // namespace server
