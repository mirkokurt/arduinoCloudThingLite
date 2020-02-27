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

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#include <ArduinoCloudThing.h>


/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoCloudThing::ArduinoCloudThing() :
  _numPrimitivesProperties(0),
  _numProperties(0),
  _isSyncMessage(false)
{}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void ArduinoCloudThing::begin() {
}

ArduinoCloudProperty& ArduinoCloudThing::addPropertyReal(ArduinoCloudProperty & property, String const & name, Permission const permission, int propertyIdentifier) {
  property.init(name, permission);
  if (isPropertyInContainer(name)) {
    return (*getProperty(name));
  } else {
    if (property.isPrimitive()) {
      _numPrimitivesProperties++;
    }
    _numProperties++;
    addProperty(&property, propertyIdentifier);
    return (property);
  }

}

void ArduinoCloudThing::readProperties(bool isSyncMessage) {
  _isSyncMessage = isSyncMessage;
  
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudProperty * p = _property_list.get(i);
    p->iotReadProperty();
    updateProperty(p->name(),0);
  }
}

void ArduinoCloudThing::writeProperties() {
  _isSyncMessage = isSyncMessage;
  
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudProperty * p = _property_list.get(i);
    p->iotWriteProperty();
  }
}

bool ArduinoCloudThing::isPropertyInContainer(String const & name) {
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudProperty * p = _property_list.get(i);
    if (p->name() == name) {
      return true;
    }
  }
  return false;
}

//retrieve property by name
ArduinoCloudProperty * ArduinoCloudThing::getProperty(String const & name) {
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudProperty * p = _property_list.get(i);
    if (p->name() == name) {
      return p;
    }
  }
  return NULL;
}

//retrieve property by identifier
ArduinoCloudProperty * ArduinoCloudThing::getProperty(int const & pos) {
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudProperty * p = _property_list.get(i);
    if (p->identifier() == pos) {
      return p;
    }
  }
  return NULL;
}

// this function updates the timestamps on the primitive properties that have been modified locally since last cloud synchronization
void ArduinoCloudThing::updateTimestampOnLocallyChangedProperties() {
  if (_numPrimitivesProperties == 0) {
    return;
  } else {
    for (int i = 0; i < _property_list.size(); i++) {
      CloudWrapperBase * p = (CloudWrapperBase *)_property_list.get(i);
      if (p->isPrimitive() && p->isChangedLocally() && p->isReadableByCloud()) {
        p->updateLocalTimestamp();
      }
    }
  }
}


void ArduinoCloudThing::updateProperty(String propertyName, unsigned long cloudChangeEventTime) {
  ArduinoCloudProperty* property = getProperty(propertyName);
  if (property && property->isWriteableByCloud()) {
    property->setLastCloudChangeTimestamp(cloudChangeEventTime);
    property->setAttributesFromCloud(&_map_data_list);
    if (_isSyncMessage) {
      property->execCallbackOnSync();
    } else {
      property->fromCloudToLocal();
      property->execCallbackOnChange();
    }
  }
}

// retrieve the property name by the identifier
String ArduinoCloudThing::getPropertyNameByIdentifier(int propertyIdentifier) {
  ArduinoCloudProperty* property;
  if (propertyIdentifier > 255) {
    property = getProperty(propertyIdentifier & 255);
  } else {
    property = getProperty(propertyIdentifier);
  }
  return property->name();
}

bool ArduinoCloudThing::ifNumericConvertToDouble(CborValue * value_iter, double * numeric_val) {

  if (cbor_value_is_integer(value_iter)) {
    int64_t val = 0;
    if (cbor_value_get_int64(value_iter, &val) == CborNoError) {
      *numeric_val = static_cast<double>(val);
      return true;
    }
  } else if (cbor_value_is_double(value_iter)) {
    double val = 0.0;
    if (cbor_value_get_double(value_iter, &val) == CborNoError) {
      *numeric_val = val;
      return true;
    }
  } else if (cbor_value_is_float(value_iter)) {
    float val = 0.0;
    if (cbor_value_get_float(value_iter, &val) == CborNoError) {
      *numeric_val = static_cast<double>(val);
      return true;
    }
  } else if (cbor_value_is_half_float(value_iter)) {
    uint16_t val = 0;
    if (cbor_value_get_half_float(value_iter, &val) == CborNoError) {
      *numeric_val = static_cast<double>(convertCborHalfFloatToDouble(val));
      return true;
    }
  }

  return false;
}

/* Source Idea from https://tools.ietf.org/html/rfc7049 : Page: 50 */
double ArduinoCloudThing::convertCborHalfFloatToDouble(uint16_t const half_val) {
  int exp = (half_val >> 10) & 0x1f;
  int mant = half_val & 0x3ff;
  double val;
  if (exp == 0) {
    val = ldexp(mant, -24);
  } else if (exp != 31) {
    val = ldexp(mant + 1024, exp - 25);
  } else {
    val = mant == 0 ? INFINITY : NAN;
  }
  return half_val & 0x8000 ? -val : val;
}

void onAutoSync(ArduinoCloudProperty & property) {
  if (property.getLastCloudChangeTimestamp() > property.getLastLocalChangeTimestamp()) {
    property.fromCloudToLocal();
    property.execCallbackOnChange();
  }
}

void onForceCloudSync(ArduinoCloudProperty & property) {
  property.fromCloudToLocal();
  property.execCallbackOnChange();
}

void onForceDeviceSync(ArduinoCloudProperty & /* property */) {
}
