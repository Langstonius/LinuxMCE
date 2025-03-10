/*******************************************
 libudev example.

 This example prints out properties of
 each of the hidraw devices. It then
 creates a monitor which will report when
 hidraw devices are connected or removed
 from the system.

 This code is meant to be a teaching
 resource. It can be used for anyone for
 any reason, including embedding into
 a commercial product.

 The document describing this file, and
 updated versions can be found at:
    http://www.signal11.us/oss/udev/

 Alan Ott
 Signal 11 Software
 2010-05-22 - Initial Revision
 2010-05-27 - Monitoring initializaion
              moved to before enumeration.
*******************************************/
//#include <string>

#include <ctype.h>
#include <string.h>

#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>

#include "udi-helper.h"

/*
static void print_attribute(struct udev_device *device, const char *name)
{
	const char *value = udev_device_get_sysattr_value (device, name);
	if (value != NULL)
		printf("    {%s}==\"%s\"\n", name, value);
}
*/
static void print_all_attributes(struct udev_device *device)
{
        struct udev_list_entry *sysattr;

        udev_list_entry_foreach(sysattr, udev_device_get_sysattr_list_entry(device)) {
                const char *name;
                const char *value;
                size_t len;

                name = udev_list_entry_get_name(sysattr);
                value = udev_device_get_sysattr_value(device, name);
                if (value == NULL)
                        continue;

                /* skip any values that look like a path */
                if (value[0] == '/')
                        continue;

                /* skip nonprintable attributes */
                len = strlen(value);
                while (len > 0 && isprint(value[len-1]))
                        len--;
                if (len > 0)
                        continue;

                printf("    {%s}==\"%s\"\n", name, value);
        }
        printf("\n");
}


static void print_property(struct udev_device *device, const char *name)
{
	const char *value = udev_device_get_property_value (device, name);
	if (value != NULL)
		printf("    {%s}==\"%s\"\n", name, value);
}

static void print_record(struct udev_device *device)
{
        const char *str;
        struct udev_list_entry *list_entry;

        printf("P: %s\n", udev_device_get_devpath(device));
        printf("D: %s\n", udev_device_get_syspath(device));

        str = udev_device_get_devnode(device);
        if (str != NULL)
                printf("N: %s\n", str + strlen("/dev/"));

        udev_list_entry_foreach(list_entry, udev_device_get_devlinks_list_entry(device))
                printf("S: %s\n", udev_list_entry_get_name(list_entry) + strlen("/dev/"));

        udev_list_entry_foreach(list_entry, udev_device_get_properties_list_entry(device))
                printf("E: %s=%s\n",
                       udev_list_entry_get_name(list_entry),
                       udev_list_entry_get_value(list_entry));
        printf("\n");
}

int main (void)
{
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev, *dev2;

   	struct udev_monitor *mon;
	int fd;

	udi_helper_map_table_new();

	/* Create the udev object */
	udev = udev_new();
	if (!udev) {
		printf("Can't create udev\n");
		exit(1);
	}

	/* This section sets up a monitor which will report events when
	   devices attached to the system change.  Events include "add",
	   "remove", "change", "online", and "offline".

	   This section sets up and starts the monitoring. Events are
	   polled for (and delivered) later in the file.

	   It is important that the monitor be set up before the call to
	   udev_enumerate_scan_devices() so that events (and devices) are
	   not missed.  For example, if enumeration happened first, there
	   would be no event generated for a device which was attached after
	   enumeration but before monitoring began.

	   Note that a filter is added so that we only get events for
	   "hidraw" devices. */

	/* Set up a monitor to monitor hidraw devices */
	mon = udev_monitor_new_from_netlink(udev, "udev");
/*	udev_monitor_filter_add_match_subsystem_devtype(mon, "hidraw", NULL);
	udev_monitor_filter_add_match_subsystem_devtype(mon, "tty", NULL);
	udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
	udev_monitor_filter_add_match_subsystem_devtype(mon, "pnp", NULL);
	udev_monitor_filter_add_match_subsystem_devtype(mon, "sound", NULL);
	udev_monitor_filter_add_match_subsystem_devtype(mon, "serial", NULL);
	udev_monitor_filter_add_match_subsystem_devtype(mon, "bluetooth_hci", NULL);
	udev_monitor_filter_add_match_subsystem_devtype(mon, "volume", NULL);
	udev_monitor_filter_add_match_subsystem_devtype(mon, "storage", NULL);
	udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL);
*/
	udev_monitor_enable_receiving(mon);
	/* Get the file descriptor (fd) for the monitor.
	   This fd will get passed to select() */
	fd = udev_monitor_get_fd(mon);

	/* Create a list of the devices in the '....' subsystem. */
	enumerate = udev_enumerate_new(udev);

	udev_enumerate_add_nomatch_subsystem(enumerate, "bdi");
	udev_enumerate_add_nomatch_subsystem(enumerate, "vc");
	udev_enumerate_add_nomatch_subsystem(enumerate, "workqueue");
	udev_enumerate_add_nomatch_subsystem(enumerate, "vtconsole");
	udev_enumerate_add_nomatch_subsystem(enumerate, "thermal");
	udev_enumerate_add_nomatch_subsystem(enumerate, "misc");
	udev_enumerate_add_nomatch_subsystem(enumerate, "mem");
	udev_enumerate_add_nomatch_subsystem(enumerate, "event_source");
	udev_enumerate_add_nomatch_subsystem(enumerate, "dmi");
	udev_enumerate_add_nomatch_subsystem(enumerate, "machinecheck");
	udev_enumerate_add_nomatch_subsystem(enumerate, "clocksource");
	udev_enumerate_add_nomatch_subsystem(enumerate, "clockevents");
	udev_enumerate_add_nomatch_subsystem(enumerate, "cpu");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "platform");
	udev_enumerate_add_nomatch_subsystem(enumerate, "rtc");
	udev_enumerate_add_nomatch_subsystem(enumerate, "regulator");
	udev_enumerate_add_nomatch_subsystem(enumerate, "drm");
	udev_enumerate_add_nomatch_subsystem(enumerate, "acpi");
	udev_enumerate_add_nomatch_subsystem(enumerate, "mdio_bus");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "pci_bus");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "pnp");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "serio");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "scsi_generic");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "scsi_host");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "scsi_device");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "scsi_disk");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "scsi");
	udev_enumerate_add_nomatch_subsystem(enumerate, "ata_port");
	udev_enumerate_add_nomatch_subsystem(enumerate, "ata_device");
	udev_enumerate_add_nomatch_subsystem(enumerate, "ata_link");
	udev_enumerate_add_nomatch_subsystem(enumerate, "bsg");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "pci");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "power_supply");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "hid");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "hidraw");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "usb");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "usb-serial");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "usbmon");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "usbmisc");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "pci_express");
	udev_enumerate_add_nomatch_subsystem(enumerate, "i2c");
//	udev_enumerate_add_nomatch_subsystem(enumerate, "input");
	udev_enumerate_add_nomatch_subsystem(enumerate, "ac97");

/*
	udev_enumerate_add_match_subsystem(enumerate, "hidraw");
	udev_enumerate_add_match_subsystem(enumerate, "tty");
	udev_enumerate_add_match_subsystem(enumerate, "block");
	udev_enumerate_add_match_subsystem(enumerate, "pnp");
	udev_enumerate_add_match_subsystem(enumerate, "sound");
	udev_enumerate_add_match_subsystem(enumerate, "serial");
	udev_enumerate_add_match_subsystem(enumerate, "bluetooth_hci");
	udev_enumerate_add_match_subsystem(enumerate, "volume");
	udev_enumerate_add_match_subsystem(enumerate, "storage");
	udev_enumerate_add_match_subsystem(enumerate, "usb");
*/
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);
	/* For each item enumerated, print out its information.
	   udev_list_entry_foreach is a macro which expands to
	   a loop. The loop will be executed for each member in
	   devices, setting dev_list_entry to a list entry
	   which contains the device's path in /sys. */
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path;

		/* Get the filename of the /sys entry for the device
		   and create a udev_device object (dev) representing it */
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);

		/* The device pointed to by dev contains information about
		   the hidraw device. In order to get information about the
		   USB device, get the parent device with the
		   subsystem/devtype pair of "usb"/"usb_device". This will
		   be several levels up the tree, but the function will find
		   it.*/
		dev2 = udev_device_get_parent_with_subsystem_devtype(
		       dev,
		       "usb",
		       "usb_device");

		if (!dev2)
			dev2 = udev_device_get_parent_with_subsystem_devtype(
			dev,
			"pci",
			NULL);

		if (!dev2)
			dev2 = udev_device_get_parent(dev);

//		const char *subsystem = udev_device_get_subsystem(dev);
//	       	const char *devpath = udev_device_get_devpath(dev);

//	if ( strstr(devpath, "/virtual") == NULL && (strcmp(subsystem, "tty") != 0 || ( strcmp(subsystem, "tty") == 0 && dev2 ))  ) { // ignore non-usb tty devices - listserialports.sh will find them.
		/* usb_device_get_devnode() returns the path to the device node itself in /dev. */
		printf("-------------------------------------------------------\n");
		printf("Device Node Path: %s\n", udev_device_get_devnode(dev));
		const char *p = udi_helper_compute_udi(dev);
		if (p) {
			printf("   UDI: %s\n", p);
		}
		printf("   Subsystem: %s\n", udev_device_get_subsystem(dev));
		printf("   Devtype: %s\n", udev_device_get_devtype(dev));
		printf("   Dev Path: %s\n", udev_device_get_devpath(dev));
		if (udev_device_has_property(dev, "ID_USB_INTERFACE_NUM")) print_property(dev, "ID_USB_INTERFACE_NUM");
		print_property(dev, "ID_SERIAL");
		print_property(dev, "SUBSYSTEM");

		print_record(dev);

		print_all_attributes(dev);

//while (dev2){
		if (dev2) {
			printf(" PARENT:\n");
			const char *p = udi_helper_compute_udi(dev2);
			if (p) {
				printf("   UDI.: %s\n", p);
			}
			printf("   Subsystem: %s\n", udev_device_get_subsystem(dev2));
			printf("   Devtype2: %s\n", udev_device_get_devtype(dev2));
			print_property(dev2, "ID_SERIAL");

			/* From here, we can call get_sysattr_value() for each file
			   in the device's /sys entry. The strings passed into these
			   functions (idProduct, idVendor, serial, etc.) correspond
			   directly to the files in the /sys directory which
			   represents the USB device. Note that USB strings are
			   Unicode, UCS2 encoded, but the strings returned from
			   udev_device_get_sysattr_value() are UTF-8 encoded. */
			printf("  VID/PID: %s %s\n",
			        udev_device_get_sysattr_value(dev2,"idVendor") ? udev_device_get_sysattr_value(dev2,"idVendor") : udev_device_get_sysattr_value(dev2,"vendor"),
			        udev_device_get_sysattr_value(dev2,"idProduct") ? udev_device_get_sysattr_value(dev2,"idProduct") : udev_device_get_sysattr_value(dev2,"device"));
			printf("  %s\n  %s\n",
			        udev_device_get_sysattr_value(dev2,"manufacturer"),
			        udev_device_get_sysattr_value(dev,"product"));
			printf("  serial: %s\n",
			         udev_device_get_sysattr_value(dev2, "serial"));
		}

		udev_device_unref(dev);
	}
//	}
//		if (dev) udev_device_unref(dev);
	/* Free the enumerator object */
	udev_enumerate_unref(enumerate);

	/* Begin polling for udev events. Events occur when devices
	   attached to the system are added, removed, or change state.
	   udev_monitor_receive_device() will return a device
	   object representing the device which changed and what type of
	   change occured.

	   The select() system call is used to ensure that the call to
	   udev_monitor_receive_device() will not block.

	   The monitor was set up earler in this file, and monitoring is
	   already underway.

	   This section will run continuously, calling usleep() at the end
	   of each pass. This is to demonstrate how to use a udev_monitor
	   in a non-blocking way. */
	while (!1) {
		/* Set up the call to select(). In this case, select() will
		   only operate on a single file descriptor, the one
		   associated with our udev_monitor. Note that the timeval
		   object is set to 0, which will cause select() to not
		   block. */
		fd_set fds;
		struct timeval tv;
		int ret;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		ret = select(fd+1, &fds, NULL, NULL, &tv);

		/* Check if our file descriptor has received data. */
		if (ret > 0 && FD_ISSET(fd, &fds)) {
			printf("\nselect() says there should be data\n");


			/* Make the call to receive the device.
			   select() ensured that this will not block. */
			dev = udev_monitor_receive_device(mon);

			/* The device pointed to by dev contains information about
			   the hidraw device. In order to get information about the
			   USB device, get the parent device with the
			   subsystem/devtype pair of "usb"/"usb_device". This will
			   be several levels up the tree, but the function will find
			   it.*/
			dev2 = udev_device_get_parent_with_subsystem_devtype(
			       dev,
			       "usb",
			       "usb_device");

			const char *subsystem = udev_device_get_subsystem(dev);
		       	const char *devpath = udev_device_get_devpath(dev);

			if ( strstr(devpath, "/virtual") == NULL && (strcmp(subsystem, "tty") != 0 || ( strcmp(subsystem, "tty") == 0 && dev2 ))  ) { // ignore non-usb tty devices - listserialports.sh will find them.
				/* usb_device_get_devnode() returns the path to the device node itself in /dev. */
				printf("-------------------------------------------------------\n");
				printf("Device Node Path: %s\n", udev_device_get_devnode(dev));
				const char *p = udi_helper_compute_udi(dev);
				if (p) {
					printf("   UDI: %s\n", p);
				}
				printf("   Subsystem: %s\n", udev_device_get_subsystem(dev));
				printf("   Devtype: %s\n", udev_device_get_devtype(dev));
				printf("   Dev Path: %s\n", udev_device_get_devpath(dev));
				printf("   Action: %s\n",udev_device_get_action(dev));
				print_property(dev, "ID_SERIAL");
				print_property(dev, "SUBSYSTEM");

				if (dev2) {
					/* From here, we can call get_sysattr_value() for each file
					   in the device's /sys entry. The strings passed into these
					   functions (idProduct, idVendor, serial, etc.) correspond
					   directly to the files in the /sys directory which
					   represents the USB device. Note that USB strings are
					   Unicode, UCS2 encoded, but the strings returned from
					   udev_device_get_sysattr_value() are UTF-8 encoded. */
					printf("  VID/PID: %s %s\n",
					        udev_device_get_sysattr_value(dev2,"idVendor") ? udev_device_get_sysattr_value(dev2,"idVendor") : udev_device_get_sysattr_value(dev2,"vendor"),
					        udev_device_get_sysattr_value(dev2,"idProduct") ? udev_device_get_sysattr_value(dev2,"idProduct") : udev_device_get_sysattr_value(dev2,"device"));
					printf("  %s\n  %s\n",
					        udev_device_get_sysattr_value(dev2,"manufacturer"),
					        udev_device_get_sysattr_value(dev,"product"));
					printf("  serial: %s\n",
					         udev_device_get_sysattr_value(dev2, "serial"));

					udev_device_unref(dev2);
				}


				print_record(dev);

				print_all_attributes(dev);


			}
			else {
				printf("No Device from receive_device(). An error occured.\n");
			}
		}
		usleep(250*1000);
		printf(".");
		fflush(stdout);
	}


	udev_unref(udev);

	return 0;
}


