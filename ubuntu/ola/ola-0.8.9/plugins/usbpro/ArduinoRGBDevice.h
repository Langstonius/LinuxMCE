/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * ArduinoRGBDevice.h
 * The Arduino RGB Mixer device.
 * Copyright (C) 2010 Simon Newton
 */

#ifndef PLUGINS_USBPRO_ARDUINORGBDEVICE_H_
#define PLUGINS_USBPRO_ARDUINORGBDEVICE_H_

#include <string>
#include "ola/DmxBuffer.h"
#include "plugins/usbpro/UsbDevice.h"
#include "plugins/usbpro/ArduinoWidget.h"

namespace ola {
namespace plugin {
namespace usbpro {


/*
 * An Arduino RGB Mixer Device
 */
class ArduinoRGBDevice: public UsbDevice {
  public:
    ArduinoRGBDevice(ola::network::SelectServerInterface *ss,
                     ola::AbstractPlugin *owner,
                     const string &name,
                     UsbWidget *widget,
                     uint16_t esta_id,
                     uint16_t device_id,
                     uint32_t serial);

    string DeviceId() const { return m_device_id; }

  private:
    string m_device_id;
};


/*
 * A single Output port per device
 */
class ArduinoRGBOutputPort: public BasicOutputPort {
  public:
    ArduinoRGBOutputPort(ArduinoRGBDevice *parent,
                         UsbWidget *widget,
                         uint16_t esta_id,
                         uint32_t serial);

    string Description() const { return m_description; }

    bool WriteDMX(const DmxBuffer &buffer, uint8_t priority) {
      return m_widget.SendDMX(buffer);
      (void) priority;
    }

    void HandleRDMRequest(const ola::rdm::RDMRequest *request,
                          ola::rdm::RDMCallback *callback) {
      return m_widget.SendRDMRequest(request, callback);
    }

    void RunFullDiscovery() {
      ola::rdm::RDMDiscoveryCallback *callback = ola::NewSingleCallback(
          static_cast<BasicOutputPort*>(this),
          &ArduinoRGBOutputPort::NewUIDList);
      m_widget.RunFullDiscovery(callback);
    }

    void RunIncrementalDiscovery() {
      ola::rdm::RDMDiscoveryCallback *callback = ola::NewSingleCallback(
          static_cast<BasicOutputPort*>(this),
          &ArduinoRGBOutputPort::NewUIDList);
      m_widget.RunIncrementalDiscovery(callback);
    }

  private:
    ArduinoWidget m_widget;
    string m_description;
};
}  // usbpro
}  // plugin
}  // ola
#endif  // PLUGINS_USBPRO_ARDUINORGBDEVICE_H_
