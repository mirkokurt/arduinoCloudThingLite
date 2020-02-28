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

#include <ArduinoCloudThingLite.h>


/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoCloudThingLite::ArduinoCloudThingLite() :
  _numPrimitivesProperties(0),
  _numProperties(0),
  _isSyncMessage(false)
{}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void ArduinoCloudThingLite::begin() {
}

ArduinoCloudPropertyLite& ArduinoCloudThingLite::addPropertyReal(ArduinoCloudPropertyLite & property, String const & name, Permission const permission, int propertyIdentifier) {
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

void ArduinoCloudThingLite::readProperties(bool isSyncMessage) {
  _isSyncMessage = isSyncMessage;
  
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudPropertyLite * p = _property_list.get(i);
    p->iotReadProperty();
    updateProperty(p->name(),0);
  }
}

void ArduinoCloudThingLite::writeProperties() {
  
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudPropertyLite * p = _property_list.get(i);
    p->iotWriteProperty();
  }
}

bool ArduinoCloudThingLite::isPropertyInContainer(String const & name) {
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudPropertyLite * p = _property_list.get(i);
    if (p->name() == name) {
      return true;
    }
  }
  return false;
}

//retrieve property by name
ArduinoCloudPropertyLite * ArduinoCloudThingLite::getProperty(String const & name) {
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudPropertyLite * p = _property_list.get(i);
    if (p->name() == name) {
      return p;
    }
  }
  return NULL;
}

//retrieve property by identifier
ArduinoCloudPropertyLite * ArduinoCloudThingLite::getProperty(int const & pos) {
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudPropertyLite * p = _property_list.get(i);
    if (p->identifier() == pos) {
      return p;
    }
  }
  return NULL;
}

// this function updates the timestamps on the primitive properties that have been modified locally since last cloud synchronization
void ArduinoCloudThingLite::updateTimestampOnLocallyChangedProperties() {
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


void ArduinoCloudThingLite::updateProperty(String propertyName, unsigned long cloudChangeEventTime) {
  ArduinoCloudPropertyLite* property = getProperty(propertyName);
  if (property && property->isWriteableByCloud()) {
    property->setLastCloudChangeTimestamp(cloudChangeEventTime);
    if (_isSyncMessage) {
      property->execCallbackOnSync();
    } else {
      property->fromCloudToLocal();
      property->execCallbackOnChange();
    }
  }
}

// retrieve the property name by the identifier
String ArduinoCloudThingLite::getPropertyNameByIdentifier(int propertyIdentifier) {
  ArduinoCloudPropertyLite* property;
  if (propertyIdentifier > 255) {
    property = getProperty(propertyIdentifier & 255);
  } else {
    property = getProperty(propertyIdentifier);
  }
  return property->name();
}

void onAutoSync(ArduinoCloudPropertyLite & property) {
  if (property.getLastCloudChangeTimestamp() > property.getLastLocalChangeTimestamp()) {
    property.fromCloudToLocal();
    property.execCallbackOnChange();
  }
}

void onForceCloudSync(ArduinoCloudPropertyLite & property) {
  property.fromCloudToLocal();
  property.execCallbackOnChange();
}

void onForceDeviceSync(ArduinoCloudPropertyLite & /* property */) {
}
