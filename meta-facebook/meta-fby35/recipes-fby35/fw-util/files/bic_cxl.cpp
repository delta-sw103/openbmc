#include <cstdio>
#include <syslog.h>
#include <unistd.h>
#include <openbmc/pal.h>
#include "bic_cxl.h"
#ifdef BIC_SUPPORT
#include <facebook/bic.h>
#include <openbmc/pal.h>

using namespace std;

#define MAX_RETRY 30

int CxlComponent::update_internal(const std::string& image, int fd, bool force) {
  int ret, retry_count;
  uint8_t status;

  try {
    cerr << "Checking if the server is ready..." << endl;
    server.ready_except();
    expansion.ready();
  } catch(const std::exception& err) {
    cout << err.what() << endl;
    return FW_STATUS_NOT_SUPPORTED;
  }

  if (image.empty() && fd < 0) {
    cerr << "File or fd is required." << endl;
    return FW_STATUS_NOT_SUPPORTED;
  }
  for (retry_count = 0; retry_count < MAX_RETRY; retry_count++) {
    ret = pal_get_server_power(slot_id, &status);
    cerr << "Server power status: " << (int) status << endl;
    if ((ret == 0) && (status == SERVER_POWER_OFF)) {
      break;
    } else {
      uint8_t power_cmd = (force ? SERVER_POWER_OFF : SERVER_GRACEFUL_SHUTDOWN);
      if ((retry_count == 0) || force) {
        cerr << "Powering off the server (cmd " << ((int) power_cmd) << ")..." << endl;
        pal_set_server_power(slot_id, power_cmd);
      }
      sleep(2);
    }
  }
  if (retry_count == MAX_RETRY) {
    cerr << "Failed to Power Off Server " << slot_id << ". Stopping the update!" << endl;
    return -1;
  }

  if (!image.empty()) {
    ret = bic_update_fw(slot_id, fw_comp, (char *)image.c_str(), force);
  } else {
    ret = bic_update_fw_fd(slot_id, fw_comp, fd, force);
  }

  if (ret != 0) {
    cerr << "CXL update failed. ret = " << ret << endl;
  }
  sleep(1);

  cerr << "Power-cycling the server..." << endl;
  pal_set_server_power(slot_id, SERVER_POWER_CYCLE);
  return ret;
}

int CxlComponent::update(const string image) {
  return update_internal(image, -1, false /* force */);
}

int CxlComponent::update(int fd, bool force) {
  return update_internal("", fd, force);
}

int CxlComponent::fupdate(const string image) {
  return update_internal(image, -1, true /* force */);
}

int CxlComponent::get_ver_str(string& s) {
  uint8_t fruid = 0;
  int ret = 0;

  ret = pal_get_fru_id((char *)_fru.c_str(), &fruid);
  if ( ret < 0 ) {
    syslog(LOG_WARNING, "Failed to get fru id");
    throw runtime_error("Error in getting fru id");
  }
  throw runtime_error("Not support to get version of CXL");

  return FW_STATUS_NOT_SUPPORTED;
}

int CxlComponent::print_version() {
  string ver("");
  try {
    server.ready_except();
    expansion.ready();
    if ( get_ver_str(ver) < 0 ) {
      throw runtime_error("Error in getting the version of CXL");
    }
    cout << "CXL Version: " << ver << endl;
  } catch(const runtime_error& err) {
    cout << "CXL Version: NA (" << err.what() << ")" << endl;
  }

  return FW_STATUS_SUCCESS;
}

void CxlComponent::get_version(json& j) {
  string ver("");
  try {
    server.ready_except();
    expansion.ready();
    if ( get_ver_str(ver) < 0 ) {
      throw runtime_error("Error in getting the version of CXL");
    }
    j["VERSION"] = ver;
  } catch(const runtime_error& err) {
    if ( string(err.what()).find("empty") != string::npos )
      j["VERSION"] = "not_present";
    else
      j["VERSION"] = "error_returned";
  }

  return;
}

#endif
