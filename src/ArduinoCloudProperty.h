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

#ifndef ARDUINO_CLOUD_PROPERTY_HPP_
#define ARDUINO_CLOUD_PROPERTY_HPP_

#ifdef HOST_BUILD
  #define substring(...) substr(__VA_ARGS__)
  #define indexOf(x) find(x)
#endif

/******************************************************************************
   INCLUDE
 ******************************************************************************/


#include <Arduino.h>
// in order to allow <functional> to define its own max and min functions
#undef max
#undef min
#include <functional>

#include "lib/LinkedList/LinkedList.h"

#define appendAttributesToCloud() appendAttributesToCloudReal(CborEncoder *encoder)
#define appendAttribute(x) appendAttributeReal(x, getAttributeName(#x, '.'), encoder)
#define setAttribute(x) setAttributeReal(x, getAttributeName(#x, '.'))
#define readProperty(x) iotReadPropertyReal(x, getAttributeName(#x, '.'))
#define writeProperty(x) iotWritePropertyReal(x, getAttributeName(#x, '.'))

enum class Permission {
  Read, Write, ReadWrite
};

enum class Type {
  Bool, Int, Float, String
};

enum class UpdatePolicy {
  OnChange, TimeInterval
};

typedef void(*UpdateCallbackFunc)(void);

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class ArduinoCloudProperty {
    typedef void(*SyncCallbackFunc)(ArduinoCloudProperty &property);
  public:
    ArduinoCloudProperty();
    void init(String const name, Permission const permission);

    /* Composable configuration of the ArduinoCloudProperty class */
    ArduinoCloudProperty & onUpdate(UpdateCallbackFunc func);
    ArduinoCloudProperty & onSync(SyncCallbackFunc func);
    ArduinoCloudProperty & publishOnChange(float const min_delta_property, unsigned long const min_time_between_updates_millis = 0);
    ArduinoCloudProperty & publishEvery(unsigned long const seconds);

    inline String name() const {
      return _name;
    }
    inline int identifier() const {
      return _identifier;
    }
    inline bool   isReadableByCloud() const {
      return (_permission == Permission::Read) || (_permission == Permission::ReadWrite);
    }
    inline bool   isWriteableByCloud() const {
      return (_permission == Permission::Write) || (_permission == Permission::ReadWrite);
    }

    //read from NINA
    void iotReadProperty() = 0;
    void iotReadPropertyReal(bool& value, String attributeName = "");
    void iotReadPropertyReal(int& value, String attributeName = "");
    void iotReadPropertyReal(float& value, String attributeName = "");
    void iotReadPropertyReal(String& value, String attributeName = "");

    //write to NINA
    void iotWriteProperty() = 0;
    void iotWritePropertyReal(bool& value, String attributeName = "");
    void iotWritePropertyReal(int& value, String attributeName = "");
    void iotWritePropertyReal(float& value, String attributeName = "");
    void iotWritePropertyReal(String& value, String attributeName = "");

    bool shouldBeUpdated();
    void execCallbackOnChange();
    void execCallbackOnSync();
    void setLastCloudChangeTimestamp(unsigned long cloudChangeTime);
    void setLastLocalChangeTimestamp(unsigned long localChangeTime);
    unsigned long getLastCloudChangeTimestamp();
    unsigned long getLastLocalChangeTimestamp();
    void setIdentifier(int identifier);

    void updateLocalTimestamp();

    virtual bool isDifferentFromCloud() = 0;
    virtual void fromCloudToLocal() = 0;
    virtual void fromLocalToCloud() = 0;
    virtual void appendAttributesToCloudReal(CborEncoder *encoder) = 0;
    virtual void setAttributesFromCloud() = 0;
    virtual bool isPrimitive() {
      return false;
    };
  protected:
    /* Variables used for UpdatePolicy::OnChange */
    String             _name;
    float              _min_delta_property;
    unsigned long      _min_time_between_updates_millis;

  private:
    Permission         _permission;
    UpdateCallbackFunc _update_callback_func;
    void (*_sync_callback_func)(ArduinoCloudProperty &property);

    UpdatePolicy       _update_policy;
    bool               _has_been_updated_once,
                       _has_been_modified_in_callback;
    /* Variables used for UpdatePolicy::TimeInterval */
    unsigned long      _last_updated_millis,
             _update_interval_millis;
    /* Variables used for reconnection sync*/
    unsigned long      _last_local_change_timestamp;
    unsigned long      _last_cloud_change_timestamp;
    LinkedList<CborMapData *> * _map_data_list;
    /* Store the identifier of the property in the array list */
    int                _identifier;
    int                _attributeIdentifier;
    
};

/******************************************************************************
   PROTOTYPE FREE FUNCTIONs
 ******************************************************************************/

inline bool operator == (ArduinoCloudProperty const & lhs, ArduinoCloudProperty const & rhs) {
  return (lhs.name() == rhs.name());
}

#endif /* ARDUINO_CLOUD_PROPERTY_HPP_ */
