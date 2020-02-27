//
// This file is part of ArduinoCloudThing
//
// Copyright 2019 ARDUINO SA (http://www.arduino.cc/)
//
// This software is released under the GNU General Public License version 3,
// which covers the main part of ArduinoCloudThing.
// The terms of this license can be found at:
// https://www.gnu.org/licenses/gpl-3.0.en.html
//
// You can be released from the requirements of the above licenses by purchasing
// a commercial license. Buying such a license is mandatory if you want to modify or
// otherwise use the software for commercial activities involving the Arduino
// software without disclosing the source code of your own applications. To purchase
// a commercial license, send an email to license@arduino.cc.
//

#include "ArduinoCloudProperty.h"

#ifdef ARDUINO_ARCH_SAMD
  #include <RTCZero.h>
  extern RTCZero rtc;
#endif

static unsigned long getTimestamp() {
  #ifdef ARDUINO_ARCH_SAMD
  return rtc.getEpoch();
  #else
#pragma message "No RTC available on this architecture - ArduinoIoTCloud will not keep track of local change timestamps ."
  return 0;
  #endif
}

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/
ArduinoCloudProperty::ArduinoCloudProperty()
  :   _name(""),
      _min_delta_property(0.0f),
      _min_time_between_updates_millis(0),
      _permission(Permission::Read),
      _update_callback_func(nullptr),
      _sync_callback_func(nullptr),
      _has_been_updated_once(false),
      _has_been_modified_in_callback(false),
      _last_updated_millis(0),
      _update_interval_millis(0),
      _last_local_change_timestamp(0),
      _last_cloud_change_timestamp(0),
      _map_data_list(nullptr),
      _identifier(0),
      _attributeIdentifier(0)
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/
void ArduinoCloudProperty::init(String const name, Permission const permission) {
  _name = name;
  _permission = permission;
}

ArduinoCloudProperty & ArduinoCloudProperty::onUpdate(UpdateCallbackFunc func) {
  _update_callback_func = func;
  return (*this);
}

ArduinoCloudProperty & ArduinoCloudProperty::onSync(SyncCallbackFunc func) {
  _sync_callback_func = func;
  return (*this);
}

ArduinoCloudProperty & ArduinoCloudProperty::publishOnChange(float const min_delta_property, unsigned long const min_time_between_updates_millis) {
  _update_policy = UpdatePolicy::OnChange;
  _min_delta_property = min_delta_property;
  _min_time_between_updates_millis = min_time_between_updates_millis;
  return (*this);
}

ArduinoCloudProperty & ArduinoCloudProperty::publishEvery(unsigned long const seconds) {
  _update_policy = UpdatePolicy::TimeInterval;
  _update_interval_millis = (seconds * 1000);
  return (*this);
}

void ArduinoCloudProperty::iotReadProperty(){
  iotReadProperty();
}

void ArduinoCloudProperty::iotReadPropertyReal(bool& value, String attributeName) {
value = WiFi.iotReadPropertyBool(attributeName.c_str());
}

void ArduinoCloudProperty::iotReadPropertyReal(int& value, String attributeName) {
  value = WiFi.iotReadPropertyInt(attributeName.c_str());
}

void ArduinoCloudProperty::iotReadPropertyReal(float& value, String attributeName) {
  value = WiFi.iotReadPropertyFloat(attributeName.c_str());
}

void ArduinoCloudProperty::iotReadPropertyReal(String& value, String attributeName) {
  value = WiFi.iotReadPropertyString(attributeName.c_str());
}

void ArduinoCloudProperty::iotWriteProperty(){
  iotWriteProperty();
}

void ArduinoCloudProperty::iotWritePropertyReal(bool& value, String attributeName) {
  WiFi.iotWritePropertyBool(attributeName.c_str(), *value);
}

void ArduinoCloudProperty::iotWritePropertyReal(int& value, String attributeName) {
  WiFi.iotWritePropertyInt(attributeName.c_str(), *value);
}

void ArduinoCloudProperty::iotWritePropertyReal(float& value, String attributeName) {
  WiFi.iotWritePropertyFloat(attributeName.c_str(), *value);
}

void ArduinoCloudProperty::iotReadPropertyReal(String& value, String attributeName) {
  WiFi.iotWritePropertyString(attributeName.c_str(), *value);
}


bool ArduinoCloudProperty::shouldBeUpdated() {
  if (!_has_been_updated_once) {
    return true;
  }

  if (_has_been_modified_in_callback) {
    _has_been_modified_in_callback = false;
    return true;
  }

  if (_update_policy == UpdatePolicy::OnChange) {
    return (isDifferentFromCloud() && ((millis() - _last_updated_millis) >= (_min_time_between_updates_millis)));
  } else if (_update_policy == UpdatePolicy::TimeInterval) {
    return ((millis() - _last_updated_millis) >= _update_interval_millis);
  } else {
    return false;
  }
}

void ArduinoCloudProperty::execCallbackOnChange() {
  if (_update_callback_func != NULL) {
    _update_callback_func();
  }
  if (!isDifferentFromCloud()) {
    _has_been_modified_in_callback = true;
  }
}

void ArduinoCloudProperty::execCallbackOnSync() {
  if (_sync_callback_func != NULL) {
    _sync_callback_func(*this);
  }
}

String ArduinoCloudProperty::getAttributeName(String propertyName, char separator) {
  int colonPos;
  String attributeName = "";
  (colonPos = propertyName.indexOf(separator)) != -1 ? attributeName = propertyName.substring(colonPos + 1) : "";
  return attributeName;
}

void ArduinoCloudProperty::updateLocalTimestamp() {
  if (isReadableByCloud()) {
    _last_local_change_timestamp = getTimestamp();
  }
}

void ArduinoCloudProperty::setLastCloudChangeTimestamp(unsigned long cloudChangeEventTime) {
  _last_cloud_change_timestamp = cloudChangeEventTime;
}

void ArduinoCloudProperty::setLastLocalChangeTimestamp(unsigned long localChangeTime) {
  _last_local_change_timestamp = localChangeTime;
}

unsigned long ArduinoCloudProperty::getLastCloudChangeTimestamp() {
  return _last_cloud_change_timestamp;
}

unsigned long ArduinoCloudProperty::getLastLocalChangeTimestamp() {
  return _last_local_change_timestamp;
}

void ArduinoCloudProperty::setIdentifier(int identifier) {
  _identifier = identifier;
}
