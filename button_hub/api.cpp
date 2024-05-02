#include "api.hpp"

#include <Arduino.h>
#include <optional>
#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "ip_resolver.hpp"
#include "kachaka-api.pb.h"
#include "src/sh2lib/sh2lib.h"

namespace api {

static String g_host;
static int g_port;

// TODO: Remove these global variables. sh2lib should be modified to allow
// passing a context to its callbacks.
static constexpr int kDefaultTimeoutMsec = 30 * 1000;
static constexpr int kSendBufferSize = 2048;
static uint8_t g_send_buffer[kSendBufferSize];
static size_t g_send_size;
static bool g_request_finished = false;
static ResultCode g_result_code = ResultCode::kOk;

static String g_get_robot_version_response;
static std::vector<Shelf> g_get_shelves_response;
static std::vector<Location> g_get_locations_response;

const char* ResultCodeToString(ResultCode code) {
  return code == api::ResultCode::kOk             ? "OK"
         : code == api::ResultCode::kNotConnected ? "Not connected"
         : code == api::ResultCode::kEncodeFailed ? "Encode failed"
         : code == api::ResultCode::kTimeout      ? "Timeout"
                                                  : "(Unknown)";
}

static int OnSendData(struct sh2lib_handle* /* handle */, char* buf,
                      const size_t length, uint32_t* data_flags) {
  (*data_flags) |= NGHTTP2_DATA_FLAG_EOF;

  if (g_send_size <= length) {
    memcpy(buf, g_send_buffer, g_send_size);
    return static_cast<int>(g_send_size);
  }
  return 0;
}

static bool DecodeString(pb_istream_t* stream, const pb_field_t* /* field */,
                         void** arg) {
  const size_t left = stream->bytes_left;
  char buf[left + 1] = {0};
  if (!pb_read(stream, reinterpret_cast<uint8_t*>(buf), left)) {
    return false;
  }
  **reinterpret_cast<String**>(arg) = String(buf, left);
  return true;
}

static bool DecodeShelf(pb_istream_t* stream, const pb_field_t* /* field */,
                        void** arg) {
  Shelf& out = **reinterpret_cast<Shelf**>(arg);
  kachaka_api_Shelf shelf{};
  shelf.id.funcs.decode = DecodeString;
  shelf.id.arg = &out.id;
  shelf.name.funcs.decode = DecodeString;
  shelf.name.arg = &out.name;

  return pb_decode(stream, kachaka_api_Shelf_fields, &shelf);
}

static bool DecodeLocation(pb_istream_t* stream, const pb_field_t* /* field */,
                           void** arg) {
  Location& out = **reinterpret_cast<Location**>(arg);
  kachaka_api_Location location{};
  location.id.funcs.decode = DecodeString;
  location.id.arg = &out.id;
  location.name.funcs.decode = DecodeString;
  location.name.arg = &out.name;

  return pb_decode(stream, kachaka_api_Location_fields, &location);
}

static bool DecodeRepeatedShelf(pb_istream_t* stream, const pb_field_t* field,
                                void** arg) {
  std::vector<Shelf>* out = *reinterpret_cast<std::vector<Shelf>**>(arg);
  Shelf shelf;
  Shelf* shelf_ptr = &shelf;
  if (!DecodeShelf(stream, field, reinterpret_cast<void**>(&shelf_ptr))) {
    return false;
  }
  out->push_back(std::move(shelf));
  return true;
}

static bool DecodeRepeatedLocation(pb_istream_t* stream,
                                   const pb_field_t* field, void** arg) {
  std::vector<Location>* out = *reinterpret_cast<std::vector<Location>**>(arg);
  Location location;
  Location* location_ptr = &location;
  if (!DecodeLocation(stream, field, reinterpret_cast<void**>(&location_ptr))) {
    return false;
  }
  out->push_back(std::move(location));
  return true;
}

static bool EncodeString(pb_ostream_t* stream, const pb_field_t* field,
                         void* const* arg) {
  return pb_encode_tag_for_field(stream, field) &&
         pb_encode_string(stream, static_cast<const pb_byte_t*>(*arg),
                          strlen(static_cast<const char*>(*arg)));
}

static void CheckFlags(const int flags) {
  if (flags == DATA_RECV_FRAME_COMPLETE || flags == DATA_RECV_RST_STREAM) {
    g_request_finished = true;
  }
}

static int HandleGetRobotVersionResponse(struct sh2lib_handle* /* handle */,
                                         const char* data, size_t len,
                                         int flags) {
  Serial.printf(" <- GetRobotVersionResponse (len=%d)\n", len);
  CheckFlags(flags);

  if (len > 0) {
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(&data[5]), len - 5);

    kachaka_api_GetRobotVersionResponse response =
        kachaka_api_GetRobotVersionResponse_init_zero;
    response.version.funcs.decode = DecodeString;
    response.version.arg = &g_get_robot_version_response;

    const int status = pb_decode(
        &stream, kachaka_api_GetRobotVersionResponse_fields, &response);
    if (!status) {
      Serial.printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
      return 1;
    }
  }

  return 0;
}

static int HandleStartCommandResponse(struct sh2lib_handle* /* handle */,
                                      const char* data, size_t len, int flags) {
  Serial.printf(" <- StartCommandResponse (len=%d)\n", len);
  CheckFlags(flags);

  if (len > 0) {
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(&data[5]), len - 5);

    String command_id;

    kachaka_api_StartCommandResponse response =
        kachaka_api_StartCommandResponse_init_zero;
    response.command_id.funcs.decode = DecodeString;
    response.command_id.arg = &command_id;

    const int status =
        pb_decode(&stream, kachaka_api_StartCommandResponse_fields, &response);
    if (!status) {
      Serial.printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
      return 1;
    }
    Serial.printf("response = {success=%d, error_code=%d, command_id=\"%s\"}\n",
                  response.result.success, response.result.error_code,
                  command_id.c_str());
  }

  return 0;
}

static int HandleGetShelvesResponse(struct sh2lib_handle* /* handle */,
                                    const char* data, size_t len, int flags) {
  Serial.printf(" <- GetShelvesResponse (len=%d)\n", len);
  CheckFlags(flags);

  if (len > 0) {
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(&data[5]), len - 5);

    kachaka_api_GetShelvesResponse response =
        kachaka_api_GetShelvesResponse_init_zero;
    response.shelves.funcs.decode = DecodeRepeatedShelf;
    response.shelves.arg = &g_get_shelves_response;

    const int status =
        pb_decode(&stream, kachaka_api_GetShelvesResponse_fields, &response);
    if (!status) {
      Serial.printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
      return 1;
    }
  }

  return 0;
}

static int HandleGetLocationsResponse(struct sh2lib_handle* /* handle */,
                                      const char* data, size_t len, int flags) {
  Serial.printf(" <- GetLocationsResponse (len=%d)\n", len);
  CheckFlags(flags);

  if (len > 0) {
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(&data[5]), len - 5);

    kachaka_api_GetLocationsResponse response =
        kachaka_api_GetLocationsResponse_init_zero;
    response.locations.funcs.decode = DecodeRepeatedLocation;
    response.locations.arg = &g_get_locations_response;

    const int status =
        pb_decode(&stream, kachaka_api_GetLocationsResponse_fields, &response);
    if (!status) {
      Serial.printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
      return 1;
    }
  }

  return 0;
}

static int HandleProceedResponse(struct sh2lib_handle* /* handle */,
                                 const char* data, size_t len, int flags) {
  Serial.printf(" <- HandleProceedResponse (len=%d)\n", len);
  CheckFlags(flags);

  if (len > 0) {
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(&data[5]), len - 5);

    kachaka_api_StartCommandResponse response =
        kachaka_api_StartCommandResponse_init_zero;

    const int status =
        pb_decode(&stream, kachaka_api_StartCommandResponse_fields, &response);
    if (!status) {
      Serial.printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
      return 1;
    }
    Serial.printf("response = {success=%d, error_code=%d}\n",
                  response.result.success, response.result.error_code);
  }

  return 0;
}

static int HandleCancelCommandResponse(struct sh2lib_handle* /* handle */,
                                       const char* data, size_t len,
                                       int flags) {
  Serial.printf(" <- HandleCancelCommandResponse (len=%d)\n", len);
  CheckFlags(flags);

  if (len > 0) {
    pb_istream_t stream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(&data[5]), len - 5);

    kachaka_api_CancelCommandResponse response =
        kachaka_api_CancelCommandResponse_init_zero;

    const int status =
        pb_decode(&stream, kachaka_api_StartCommandResponse_fields, &response);
    if (!status) {
      Serial.printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
      return 1;
    }
    Serial.printf("response = {success=%d, error_code=%d}\n",
                  response.result.success, response.result.error_code);
  }

  return 0;
}

static bool EncodeProtoBufMessage(uint8_t* buffer, const int buffer_size,
                                  size_t* out_size, const pb_msgdesc_t* fields,
                                  const void* message) {
  pb_ostream_t stream = pb_ostream_from_buffer(&buffer[5], buffer_size - 5);

  const bool status = pb_encode(&stream, fields, message);
  if (!status) {
    Serial.printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
    return false;
  }

  // gRPC header (5 bytes)
  buffer[0] = 0;  // no compression
  const uint32_t pb_size_bigendian = htonl(stream.bytes_written);
  memcpy(&buffer[1], &pb_size_bigendian, sizeof(pb_size_bigendian));

  *out_size = 5 + stream.bytes_written;
  return true;
}

static void SendGrpcRequestAndWait(
    struct sh2lib_handle* hd, const char* service,
    sh2lib_frame_data_recv_cb_t response_callback) {
  char path[64];
  char len[8];

  Serial.printf("--> %s\n", service);

  snprintf(path, sizeof(path), "/kachaka_api.KachakaApi/%s", service);
  snprintf(len, sizeof(len), "%d", g_send_size);

  const nghttp2_nv nva[] = {
      SH2LIB_MAKE_NV(":method", "POST"),
      SH2LIB_MAKE_NV(":scheme", "https"),
      SH2LIB_MAKE_NV(":authority", hd->hostname),
      SH2LIB_MAKE_NV(":path", path),
      SH2LIB_MAKE_NV("te", "trailers"),
      SH2LIB_MAKE_NV("Content-Type", "application/grpc"),
      SH2LIB_MAKE_NV("content-length", len),
  };

  g_request_finished = false;
  sh2lib_do_putpost_with_nv(hd, nva, sizeof(nva) / sizeof(nva[0]), OnSendData,
                            response_callback);

  while (!g_request_finished) {
    if (sh2lib_execute(hd) != ESP_OK) {
      Serial.println("Error in execute");
      break;
    }
    delay(20);
  }
}

struct Service {
  const char* service_name;
  sh2lib_frame_data_recv_cb_t response_callback;
  TaskHandle_t parent_task_handle;
};

static void SendGrpcTask(void* param) {
  const Service* service = reinterpret_cast<Service*>(param);

  String ip_or_host = ip_resolver::GetIpAddressIfPossible(g_host, true);

  String uri = "https://" + ip_or_host + ":" + String(g_port);

  sh2lib_handle hd{};
  sh2lib_config_t config = {
      .uri = uri.c_str(),
      .cacert_buf = nullptr,
      .cacert_bytes = 0,
      .crt_bundle_attach = nullptr,
  };

  if (sh2lib_connect(&config, &hd) != ESP_OK) {
    Serial.println("Error connecting to HTTP2 server");

    g_result_code = ResultCode::kNotConnected;
  } else {
    Serial.println("Connected to HTTP2 server");

    SendGrpcRequestAndWait(&hd, service->service_name,
                           service->response_callback);
  }

  xTaskNotifyGive(service->parent_task_handle);
  sh2lib_free(&hd);
  vTaskDelete(nullptr);
}

static bool EncodeSendAndWait(Service& service, const pb_msgdesc_t* fields,
                              const void* request) {
  g_result_code = ResultCode::kOk;

  if (!EncodeProtoBufMessage(g_send_buffer, sizeof(g_send_buffer), &g_send_size,
                             fields, request)) {
    g_result_code = ResultCode::kEncodeFailed;
    return false;
  }

  TaskHandle_t task = nullptr;
  xTaskCreate(SendGrpcTask, "SendGrpcTask", 10 * 1024,
              reinterpret_cast<void*>(&service), 5, &task);
  const int retv = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(kDefaultTimeoutMsec));
  if (g_result_code == ResultCode::kNotConnected) {
    return false;
  }
  if (retv != pdTRUE) {
    Serial.printf("Timeout waiting for %s\n", service);
    g_result_code = ResultCode::kTimeout;
    return false;
  }
  return retv == pdPASS;
}

void SetRobotHost(String host, const int port) {
  g_host = std::move(host);
  g_port = port;
}

std::pair<ResultCode, String> GetRobotVersion() {
  static Service service = {"GetRobotVersion", HandleGetRobotVersionResponse};
  service.parent_task_handle = xTaskGetCurrentTaskHandle();

  kachaka_api_GetRequest request = kachaka_api_GetRequest_init_zero;
  request.metadata.cursor = 0;

  g_get_robot_version_response.clear();
  if (!EncodeSendAndWait(service, kachaka_api_GetRequest_fields, &request)) {
    return {g_result_code, {}};
  }

  return {g_result_code, std::move(g_get_robot_version_response)};
}

static void FillCommandCommon(kachaka_api_StartCommandRequest& request,
                              const bool cancel_all, const char* tts_on_success,
                              const bool deferrable, const char* title) {
  request.cancel_all = cancel_all;
  if (tts_on_success) {
    request.tts_on_success.funcs.encode = EncodeString;
    request.tts_on_success.arg = const_cast<char*>(tts_on_success);
  }
  request.deferrable = deferrable;
  if (title) {
    request.title.funcs.encode = EncodeString;
    request.title.arg = const_cast<char*>(title);
  }
}

ResultCode ReturnHome(const bool cancel_all, const char* tts_on_success,
                      const bool deferrable, const char* title) {
  static Service service = {"StartCommand", HandleStartCommandResponse};
  service.parent_task_handle = xTaskGetCurrentTaskHandle();

  kachaka_api_StartCommandRequest request =
      kachaka_api_StartCommandRequest_init_zero;
  request.has_command = true;
  request.command.which_command = kachaka_api_Command_return_home_command_tag;
  FillCommandCommon(request, cancel_all, tts_on_success, deferrable, title);

  EncodeSendAndWait(service, kachaka_api_StartCommandRequest_fields, &request);
  return g_result_code;
}

ResultCode MoveToLocation(const char* location_id, const bool cancel_all,
                          const char* tts_on_success, const bool deferrable,
                          const char* title) {
  static Service service = {"StartCommand", HandleStartCommandResponse};
  service.parent_task_handle = xTaskGetCurrentTaskHandle();

  kachaka_api_StartCommandRequest request =
      kachaka_api_StartCommandRequest_init_zero;
  request.has_command = true;
  request.command.which_command =
      kachaka_api_Command_move_to_location_command_tag;
  request.command.command.move_to_location_command.target_location_id.funcs
      .encode = EncodeString;
  request.command.command.move_to_location_command.target_location_id.arg =
      const_cast<char*>(location_id);
  FillCommandCommon(request, cancel_all, tts_on_success, deferrable, title);

  EncodeSendAndWait(service, kachaka_api_StartCommandRequest_fields, &request);
  return g_result_code;
}

ResultCode Speak(const char* text, const bool cancel_all,
                 const char* tts_on_success, const bool deferrable,
                 const char* title) {
  static Service service = {"StartCommand", HandleStartCommandResponse};
  service.parent_task_handle = xTaskGetCurrentTaskHandle();

  kachaka_api_StartCommandRequest request =
      kachaka_api_StartCommandRequest_init_zero;
  request.has_command = true;
  request.command.which_command = kachaka_api_Command_speak_command_tag;
  request.command.command.speak_command.text.funcs.encode = EncodeString;
  request.command.command.speak_command.text.arg = const_cast<char*>(text);
  FillCommandCommon(request, cancel_all, tts_on_success, deferrable, title);

  EncodeSendAndWait(service, kachaka_api_StartCommandRequest_fields, &request);
  return g_result_code;
}

ResultCode MoveShelf(const char* shelf_id, const char* location_id,
                     const bool cancel_all, const char* tts_on_success,
                     const bool deferrable, const char* title) {
  static Service service = {"StartCommand", HandleStartCommandResponse};
  service.parent_task_handle = xTaskGetCurrentTaskHandle();

  kachaka_api_StartCommandRequest request =
      kachaka_api_StartCommandRequest_init_zero;
  request.has_command = true;
  request.command.which_command = kachaka_api_Command_move_shelf_command_tag;
  auto& cmd = request.command.command.move_shelf_command;
  cmd.target_shelf_id.funcs.encode = EncodeString;
  cmd.target_shelf_id.arg = const_cast<char*>(shelf_id);
  cmd.destination_location_id.funcs.encode = EncodeString;
  cmd.destination_location_id.arg = const_cast<char*>(location_id);
  FillCommandCommon(request, cancel_all, tts_on_success, deferrable, title);

  EncodeSendAndWait(service, kachaka_api_StartCommandRequest_fields, &request);
  return g_result_code;
}

ResultCode ReturnShelf(const char* shelf_id, const bool cancel_all,
                       const char* tts_on_success, const bool deferrable,
                       const char* title) {
  static Service service = {"StartCommand", HandleStartCommandResponse};
  service.parent_task_handle = xTaskGetCurrentTaskHandle();

  kachaka_api_StartCommandRequest request =
      kachaka_api_StartCommandRequest_init_zero;
  request.has_command = true;
  request.command.which_command = kachaka_api_Command_return_shelf_command_tag;
  request.command.command.return_shelf_command.target_shelf_id.funcs.encode =
      EncodeString;
  request.command.command.return_shelf_command.target_shelf_id.arg =
      const_cast<char*>(shelf_id);
  FillCommandCommon(request, cancel_all, tts_on_success, deferrable, title);

  EncodeSendAndWait(service, kachaka_api_StartCommandRequest_fields, &request);
  return g_result_code;
}

ResultCode Lock(const double duration_sec, const char* title) {
  static Service service = {"StartCommand", HandleStartCommandResponse};
  service.parent_task_handle = xTaskGetCurrentTaskHandle();

  kachaka_api_StartCommandRequest request =
      kachaka_api_StartCommandRequest_init_zero;
  request.has_command = true;
  request.command.which_command = kachaka_api_Command_lock_command_tag;
  request.command.command.lock_command.duration_sec = duration_sec;
  FillCommandCommon(request, false, nullptr, false, title);

  EncodeSendAndWait(service, kachaka_api_StartCommandRequest_fields, &request);
  return g_result_code;
}

std::pair<ResultCode, std::vector<Shelf>> GetShelves() {
  static Service service = {"GetShelves", HandleGetShelvesResponse};
  service.parent_task_handle = xTaskGetCurrentTaskHandle();

  kachaka_api_GetRequest request = kachaka_api_GetRequest_init_zero;
  request.metadata.cursor = 0;

  g_get_shelves_response.clear();
  if (!EncodeSendAndWait(service, kachaka_api_GetRequest_fields, &request)) {
    return {g_result_code, {}};
  }
  return {g_result_code, std::move(g_get_shelves_response)};
}

std::pair<ResultCode, std::vector<Location>> GetLocations() {
  static Service service = {"GetLocations", HandleGetLocationsResponse};
  service.parent_task_handle = xTaskGetCurrentTaskHandle();

  kachaka_api_GetRequest request = kachaka_api_GetRequest_init_zero;
  request.metadata.cursor = 0;

  g_get_locations_response.clear();
  if (!EncodeSendAndWait(service, kachaka_api_GetRequest_fields, &request)) {
    return {g_result_code, {}};
  }
  return {g_result_code, std::move(g_get_locations_response)};
}

ResultCode Proceed() {
  static Service service = {"Proceed", HandleProceedResponse};
  service.parent_task_handle = xTaskGetCurrentTaskHandle();

  kachaka_api_EmptyRequest request = kachaka_api_EmptyRequest_init_zero;

  EncodeSendAndWait(service, kachaka_api_EmptyRequest_fields, &request);
  return g_result_code;
}

ResultCode CancelCommand() {
  static Service service = {"CancelCommand", HandleCancelCommandResponse};
  service.parent_task_handle = xTaskGetCurrentTaskHandle();

  kachaka_api_EmptyRequest request = kachaka_api_EmptyRequest_init_zero;

  EncodeSendAndWait(service, kachaka_api_EmptyRequest_fields, &request);
  return g_result_code;
}

}  // namespace api
