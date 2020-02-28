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

#ifndef ARDUINO_CLOUD_THING_LITE_H_
#define ARDUINO_CLOUD_THING_LITE_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "ArduinoCloudPropertyLite.h"
#include "lib/LinkedList/LinkedList.h"
#include "types/CloudBool.h"
#include "types/CloudFloat.h"
#include "types/CloudInt.h"
#include "types/CloudString.h"
//#include "types/CloudLocation.h"
//#include "types/CloudColor.h"
#include "types/CloudWrapperBase.h"

//#include "types/automation/CloudColoredLight.h"
//#include "types/automation/CloudContactSensor.h"
//#include "types/automation/CloudDimmedLight.h"
//#include "types/automation/CloudLight.h"
//#include "types/automation/CloudMotionSensor.h"
//#include "types/automation/CloudSmartPlug.h"
//#include "types/automation/CloudSwitch.h"
//#include "types/automation/CloudTemperature.h"
//#include "types/automation/CloudTelevision.h"


/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static bool const ON  = true;
static bool const OFF = false;

static long const ON_CHANGE = -1;
static long const SECONDS   = 1;
static long const MINUTES   = 60;
static long const HOURS     = 3600;
static long const DAYS      = 86400;

/******************************************************************************
   SYNCHRONIZATION CALLBACKS
 ******************************************************************************/

void onAutoSync(ArduinoCloudPropertyLite & property);
#define MOST_RECENT_WINS onAutoSync
void onForceCloudSync(ArduinoCloudPropertyLite & property);
#define CLOUD_WINS onForceCloudSync
void onForceDeviceSync(ArduinoCloudPropertyLite & property);
#define DEVICE_WINS onForceDeviceSync // The device property value is already the correct one. The cloud property value will be synchronized at the next update cycle.

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class ArduinoCloudThingLite {

  public:
    ArduinoCloudThingLite();

    void begin();
    //if propertyIdentifier is different from -1, an integer identifier is associated to the added property to be use instead of the property name when the parameter lightPayload is true in the encode method
    ArduinoCloudPropertyLite   & addPropertyReal(ArduinoCloudPropertyLite   & property, String const & name, Permission const permission, int propertyIdentifier = -1);

    bool isPropertyInContainer(String const & name);

    void updateTimestampOnLocallyChangedProperties();
    void updateProperty(String propertyName, unsigned long cloudChangeEventTime);
    String getPropertyNameByIdentifier(int propertyIdentifier);

    void readProperties(bool isSyncMessage = false);
    void writeProperties();

  private:
    LinkedList<ArduinoCloudPropertyLite *>   _property_list;
    /* Keep track of the number of primitive properties in the Thing. If 0 it allows the early exit in updateTimestampOnLocallyChangedProperties() */
    int                                  _numPrimitivesProperties;
    int                                  _numProperties;
    /* Indicates the if the message received to be decoded is a response to the getLastValues inquiry */
    bool                                 _isSyncMessage;
    
    inline void addProperty(ArduinoCloudPropertyLite   * property_obj, int propertyIdentifier) {
      if (propertyIdentifier != -1) {
        property_obj->setIdentifier(propertyIdentifier);
      } else {
        // if property identifier is -1, an incremental value will be assigned as identifier.
        property_obj->setIdentifier(_numProperties);
      }
      _property_list.add(property_obj);
    }
    ArduinoCloudPropertyLite * getProperty(String const & name);
    ArduinoCloudPropertyLite * getProperty(int const & identifier);

};

#endif /* ARDUINO_CLOUD_THING_H_ */
