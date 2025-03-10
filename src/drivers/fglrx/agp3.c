/*
 * This file includes AGP 3.0 detection and enabling routines for
 * northbridges that are AGP 3.0 (8x speeds) compliant.  This is not 
 * a full implementation against the AGP 3.0 specification...yet.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JEFF HARTMANN, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * TODO: 
 * - Allocate more than order 0 pages to avoid too much linear map splitting.
 */

/* further general & copyright notes, for e.g. VIA KT400/AGP3, 
 * compareable to Linux kernel 2.5/2.6 tree */
/*
 * AGP v3 implementation.
 *
 * Copyright (C) 2002-2003 Dave Jones
 * Copyright (C) 1999 Jeff Hartmann
 * Copyright (C) 1999 Precision Insight, Inc.
 * Copyright (C) 1999 Xi Graphics, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JEFF HARTMANN, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <linux/version.h>
#ifdef MODVERSIONS
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
#include <linux/modversions.h>
#endif
#endif
#include <linux/config.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/pagemap.h>

//#include <linux/agp_backend.h>
#include "agp_backend.h"

#include "agp.h"

#if 0
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL and additional rights");
#endif
#endif

struct agp_3_0_dev {
	struct list_head list;
	u8 capndx;
	u32 maxbw;
	struct pci_dev *dev;
};

static int agp_3_0_dev_list_insert(struct list_head *head, struct list_head *new)
{
	struct agp_3_0_dev *cur, *n = list_entry(new, struct agp_3_0_dev, list);
	struct list_head *pos;

	list_for_each(pos, head) {
		cur = list_entry(pos, struct agp_3_0_dev, list);
		if(cur->maxbw > n->maxbw)
			break;
	}
	list_add_tail(new, pos);

	return 0;
}

static int agp_3_0_dev_list_sort(struct agp_3_0_dev *list, unsigned int ndevs)
{
	struct agp_3_0_dev *cur;
	struct pci_dev *dev;
	struct list_head *pos, *tmp, *head = &list->list, *start = head->next;
	u32 nistat;

	INIT_LIST_HEAD(head);

	for(pos = start; pos != head;) {
		cur = list_entry(pos, struct agp_3_0_dev, list);
		dev = cur->dev;

		pci_read_config_dword(dev, cur->capndx + 0x0c, &nistat);
		cur->maxbw = (nistat >> 16) & 0xff;

		tmp = pos;
		pos = pos->next;
		agp_3_0_dev_list_insert(head, tmp);
	}
	return 0;
}

/* 
 * Initialize all isochronous transfer parameters for an AGP 3.0 
 * node (i.e. a host bridge in combination with the adapters 
 * lying behind it...)
 */

static int agp_3_0_isochronous_node_enable(struct agp_3_0_dev *dev_list, unsigned int ndevs)
{
	/*
	 * Convenience structure to make the calculations clearer
	 * here.  The field names come straight from the AGP 3.0 spec.
	 */
	struct isoch_data {
		u32 maxbw;
		u32 n;
		u32 y;
		u32 l;
		u32 rq;
		struct agp_3_0_dev *dev;
	};

	struct pci_dev *td = agp_bridge.dev, *dev;
	struct list_head *head = &dev_list->list, *pos;
	struct agp_3_0_dev *cur;
	struct isoch_data *master, target;
	unsigned int cdev = 0;
	u32 mnistat, tnistat, tstatus, mcmd;
	u16 tnicmd, mnicmd;
	u8 mcapndx;
	u32 tot_bw = 0, tot_n = 0, tot_rq = 0, y_max, rq_isoch, rq_async;
	u32 step, rem, rem_isoch, rem_async;
	int ret = 0;

	/*
	 * We'll work with an array of isoch_data's (one for each
	 * device in dev_list) throughout this function.
	 */
	if((master = kmalloc(ndevs * sizeof(*master), GFP_KERNEL)) == NULL) {
		ret = -ENOMEM;
		goto get_out;
	}

	/*
	 * Sort the device list by maxbw.  We need to do this because the
	 * spec suggests that the devices with the smallest requirements
	 * have their resources allocated first, with all remaining resources
	 * falling to the device with the largest requirement.
	 *
	 * We don't exactly do this, we divide target resources by ndevs
	 * and split them amongst the AGP 3.0 devices.  The remainder of such
	 * division operations are dropped on the last device, sort of like
	 * the spec mentions it should be done.
	 *
	 * We can't do this sort when we initially construct the dev_list
	 * because we don't know until this function whether isochronous
	 * transfers are enabled and consequently whether maxbw will mean
	 * anything.
	 */
	if((ret = agp_3_0_dev_list_sort(dev_list, ndevs)) != 0)
		goto free_and_exit;

	pci_read_config_dword(td, agp_bridge.capndx + 0x0c, &tnistat);
	pci_read_config_dword(td, agp_bridge.capndx + 0x04, &tstatus);

	/* Extract power-on defaults from the target */
	target.maxbw = (tnistat >> 16) & 0xff;
	target.n     = (tnistat >> 8)  & 0xff;
	target.y     = (tnistat >> 6)  & 0x3;
	target.l     = (tnistat >> 3)  & 0x7;
	target.rq    = (tstatus >> 24) & 0xff;

	y_max = target.y;

	/*
	 * Extract power-on defaults for each device in dev_list.  Along
	 * the way, calculate the total isochronous bandwidth required
	 * by these devices and the largest requested payload size.
	 */
	list_for_each(pos, head) {
		cur = list_entry(pos, struct agp_3_0_dev, list);
		dev = cur->dev;

		mcapndx = cur->capndx;

		pci_read_config_dword(dev, cur->capndx + 0x0c, &mnistat);

		master[cdev].maxbw = (mnistat >> 16) & 0xff;
		master[cdev].n     = (mnistat >> 8)  & 0xff;
		master[cdev].y     = (mnistat >> 6)  & 0x3;
		master[cdev].dev   = cur;

		tot_bw += master[cdev].maxbw;
		y_max = max(y_max, master[cdev].y);

		cdev++;
	}

	/* Check if this configuration has any chance of working */
	if(tot_bw > target.maxbw) {
		printk(KERN_ERR PFX "isochronous bandwidth required "
			"by AGP 3.0 devices exceeds that which is supported by "
			"the AGP 3.0 bridge!\n");
		ret = -ENODEV;
		goto free_and_exit;
	}

	target.y = y_max;

	/*
	 * Write the calculated payload size into the target's NICMD
	 * register.  Doing this directly effects the ISOCH_N value
	 * in the target's NISTAT register, so we need to do this now
	 * to get an accurate value for ISOCH_N later.
	 */
	pci_read_config_word(td, agp_bridge.capndx + 0x20, &tnicmd);
	tnicmd &= ~(0x3 << 6);
	tnicmd |= target.y << 6;
	pci_write_config_word(td, agp_bridge.capndx + 0x20, tnicmd);

	/* Reread the target's ISOCH_N */
	pci_read_config_dword(td, agp_bridge.capndx + 0x0c, &tnistat);
	target.n = (tnistat >> 8) & 0xff;

	/* Calculate the minimum ISOCH_N needed by each master */
	for(cdev = 0; cdev < ndevs; cdev++) {
		master[cdev].y = target.y;
		master[cdev].n = master[cdev].maxbw / (master[cdev].y + 1);

		tot_n += master[cdev].n;
	}

	/* Exit if the minimal ISOCH_N allocation among the masters is more
	 * than the target can handle. */
	if(tot_n > target.n) {
		printk(KERN_ERR PFX "number of isochronous "
			"transactions per period required by AGP 3.0 devices "
			"exceeds that which is supported by the AGP 3.0 "
			"bridge!\n");
		ret = -ENODEV;
		goto free_and_exit;
	}

	/* Calculate left over ISOCH_N capability in the target.  We'll give
	 * this to the hungriest device (as per the spec) */
	rem  = target.n - tot_n;

	/* 
	 * Calculate the minimum isochronous RQ depth needed by each master.
	 * Along the way, distribute the extra ISOCH_N capability calculated
	 * above.
	 */
	for(cdev = 0; cdev < ndevs; cdev++) {
		/*
		 * This is a little subtle.  If ISOCH_Y > 64B, then ISOCH_Y
		 * byte isochronous writes will be broken into 64B pieces.
		 * This means we need to budget more RQ depth to account for
		 * these kind of writes (each isochronous write is actually
		 * many writes on the AGP bus).
		 */
		master[cdev].rq = master[cdev].n;
		if(master[cdev].y > 0x1) {
			master[cdev].rq *= (1 << (master[cdev].y - 1));
		}

		tot_rq += master[cdev].rq;

		if(cdev == ndevs - 1)
			master[cdev].n += rem;
	}

	/* Figure the number of isochronous and asynchronous RQ slots the
	 * target is providing. */
	rq_isoch = (target.y > 0x1) ? target.n * (1 << (target.y - 1)) : target.n;
	rq_async = target.rq - rq_isoch;

	/* Exit if the minimal RQ needs of the masters exceeds what the target
	 * can provide. */
	if(tot_rq > rq_isoch) {
		printk(KERN_ERR PFX "number of request queue slots "
			"required by the isochronous bandwidth requested by "
			"AGP 3.0 devices exceeds the number provided by the "
			"AGP 3.0 bridge!\n");
		ret = -ENODEV;
		goto free_and_exit;
	}

	/* Calculate asynchronous RQ capability in the target (per master) as
	 * well as the total number of leftover isochronous RQ slots. */
	step      = rq_async / ndevs;
	rem_async = step + (rq_async % ndevs);
	rem_isoch = rq_isoch - tot_rq;

	/* Distribute the extra RQ slots calculated above and write our
	 * isochronous settings out to the actual devices. */
	for(cdev = 0; cdev < ndevs; cdev++) {
		cur = master[cdev].dev;
		dev = cur->dev;

		mcapndx = cur->capndx;

		master[cdev].rq += (cdev == ndevs - 1)
		              ? (rem_async + rem_isoch) : step;

		pci_read_config_word(dev, cur->capndx + 0x20, &mnicmd);
		pci_read_config_dword(dev, cur->capndx + 0x08, &mcmd);

		mnicmd &= ~(0xff << 8);
		mnicmd &= ~(0x3  << 6);
		mcmd   &= ~(0xff << 24);

		mnicmd |= master[cdev].n  << 8;
		mnicmd |= master[cdev].y  << 6;
		mcmd   |= master[cdev].rq << 24;

		pci_write_config_dword(dev, cur->capndx + 0x08, mcmd);
		pci_write_config_word(dev, cur->capndx + 0x20, mnicmd);
	}

free_and_exit:
	kfree(master);

get_out:
	return ret;
}

/*
 * This function basically allocates request queue slots among the
 * AGP 3.0 systems in nonisochronous nodes.  The algorithm is
 * pretty stupid, divide the total number of RQ slots provided by the
 * target by ndevs.  Distribute this many slots to each AGP 3.0 device,
 * giving any left over slots to the last device in dev_list.
 */
static int agp_3_0_nonisochronous_node_enable(struct agp_3_0_dev *dev_list, unsigned int ndevs)
{
	struct agp_3_0_dev *cur;
	struct list_head *head = &dev_list->list, *pos;
	u32 tstatus, mcmd;
	u32 trq, mrq, rem;
	unsigned int cdev = 0;

	pci_read_config_dword(agp_bridge.dev, agp_bridge.capndx + 0x04, &tstatus);

	trq = (tstatus >> 24) & 0xff;
	mrq = trq / ndevs;

	rem = mrq + (trq % ndevs);

	for(pos = head->next; cdev < ndevs; cdev++, pos = pos->next) {
		cur = list_entry(pos, struct agp_3_0_dev, list);

		pci_read_config_dword(cur->dev, cur->capndx + 0x08, &mcmd);
		mcmd &= ~(0xff << 24);
		mcmd |= ((cdev == ndevs - 1) ? rem : mrq) << 24;
		pci_write_config_dword(cur->dev, cur->capndx + 0x08, mcmd);
	}

	return 0;
}

/*
 * Fully configure and enable an AGP 3.0 host bridge and all the devices
 * lying behind it.
 */
static int agp_3_0_node_enable(u32 mode, u32 minor)
{
	struct pci_dev *td = agp_bridge.dev, *dev;
	u8 bus_num, mcapndx;
	u32 isoch, arqsz, cal_cycle, tmp, rate;
#if 1 /* FGL */
    u32 sba, fw;
#endif
	u32 tstatus, tcmd, mcmd, mstatus, ncapid;
	u32 mmajor, mminor;
	u16 mpstat;
	struct agp_3_0_dev *dev_list, *cur;
	struct list_head *head, *pos;
	unsigned int ndevs = 0;
	int ret = 0;

	/* 
	 * Allocate a head for our AGP 3.0 device list (multiple AGP 3.0
	 * devices are allowed behind a single bridge). 
	 */
	if((dev_list = kmalloc(sizeof(*dev_list), GFP_KERNEL)) == NULL) {
		ret = -ENOMEM;
		goto get_out;
	}
	head = &dev_list->list;
	INIT_LIST_HEAD(head);

	/* 
	 * Find all the devices on this bridge's secondary bus and add them
	 * to dev_list. 
	 */
#if 1 /* FGL */
    {
        u8 header_type;
    	struct pci_dev *p2p = agp_bridge_pci2pci.dev;

        /* assign probed _uncertain_ value from bus scan as the default value */
        bus_num = agp_graphics_device_bus;

        /* if possible query the PCI-to-PCI (AGP) bus bridge */
#ifndef PCI_HEADER_TYPE_LAYOUT
#define PCI_HEADER_TYPE_LAYOUT 0x7f
#endif
#ifndef PCI_HEADER_TYPE_MULTIFUNCTION
#define PCI_HEADER_TYPE_MULTIFUNCTION 0x80
#endif
        if (p2p)
        {
    	    pci_read_config_byte(p2p, PCI_HEADER_TYPE, &header_type);
            if ((header_type & PCI_HEADER_TYPE_LAYOUT) == PCI_HEADER_TYPE_BRIDGE)
            {
	            pci_read_config_byte(p2p, PCI_SECONDARY_BUS, &bus_num);
            }
        }
    }
#else    
	pci_read_config_byte(td, PCI_SECONDARY_BUS, &bus_num);
#endif
	pci_for_each_dev(dev) {
		if(dev->bus->number == bus_num) {
#if 1 /* FGL */
            /* there are devices out there that do only expose */
            /* AGP caps on their pci function #0, and not on function #1 or higher. */
            /* strip such sub functions at this early point to make our life simpler. */

            // does this device have a CAP_LIST ?
    		pci_read_config_word(dev, PCI_STATUS, &mpstat);
	    	if((mpstat & PCI_STATUS_CAP_LIST) == 0)
            {
		    	mcapndx = 0;
            }
            else
            {
                // determine the CAP_INDEX to the AGP capability entry
        		pci_read_config_byte(dev, PCI_CAPABILITY_LIST, &mcapndx);
	        	if (mcapndx != 0x00) {
		        	do {
        				pci_read_config_dword(dev, mcapndx, &ncapid);
	        			if ((ncapid & 0xff) != 0x02)
		        			mcapndx = (ncapid >> 8) & 0xff;
			        }
			        while (((ncapid & 0xff) != 0x02) && (mcapndx != 0x00));
                }
    		}

            /* no AGP capability found */
    		if(mcapndx == 0)
                if(PCI_FUNC(dev->devfn)!=0) /* pci function value is >0 on the device */
                {
#if 0
        			printk(KERN_INFO PFX "device with function>0 "
	        			"found on the secondary bus of an AGP 3.0 "
		        		"bridge which has no AGP capability entry!\n");
#endif                
                    continue; /* ignore this single pci sub function */
                }
#endif /* FGL */

			if((cur = kmalloc(sizeof(*cur), GFP_KERNEL)) == NULL) {
				ret = -ENOMEM;
				goto free_and_exit;
			}
			
			cur->dev = dev;

			pos = &cur->list;
			list_add(pos, head);
			ndevs++;
		}
	}

	/* Extract some power-on defaults from the target */
	pci_read_config_dword(td, agp_bridge.capndx + 0x04, &tstatus);
	isoch     = (tstatus >> 17) & 0x1;
	arqsz     = (tstatus >> 13) & 0x7;
	cal_cycle = (tstatus >> 10) & 0x7;
	rate      = tstatus & 0x7;
#if 1 /* FGL */
    sba       = tstatus & (1<<9);
    fw        = tstatus & (1<<4);
#endif

	/*
	 * Take an initial pass through the devices lying behind our host
	 * bridge.  Make sure each one is actually an AGP 3.0 device, otherwise
	 * exit with an error message.  Along the way store the AGP 3.0
	 * cap_ptr for each device, the minimum supported cal_cycle, and the
	 * minimum supported data rate.
	 */
	list_for_each(pos, head) {
		cur = list_entry(pos, struct agp_3_0_dev, list);
		dev = cur->dev;
		
		pci_read_config_word(dev, PCI_STATUS, &mpstat);
		if((mpstat & PCI_STATUS_CAP_LIST) == 0)
			continue;

		pci_read_config_byte(dev, PCI_CAPABILITY_LIST, &mcapndx);
		if (mcapndx != 0x00) {
			do {
				pci_read_config_dword(dev, mcapndx, &ncapid);
				if ((ncapid & 0xff) != 0x02)
					mcapndx = (ncapid >> 8) & 0xff;
			}
			while (((ncapid & 0xff) != 0x02) && (mcapndx != 0x00));
		}

		if(mcapndx == 0) {
			printk(KERN_ERR PFX "woah!  Non-AGP device "
				"found on the secondary bus of an AGP 3.0 "
				"bridge!\n");
			ret = -ENODEV;
			goto free_and_exit;
		}

		mmajor = (ncapid >> 20) & 0xf;
		mminor = (ncapid >> 16) & 0xf;

		if(mmajor < 3) {
			printk(KERN_ERR PFX "woah!  AGP 2.0 device "
				"found on the secondary bus of an AGP 3.0 "
				"bridge operating with AGP 3.0 electricals!\n");
			ret = -ENODEV;
			goto free_and_exit;
		}

		cur->capndx = mcapndx;

		pci_read_config_dword(dev, cur->capndx + 0x04, &mstatus);

		if(((mstatus >> 3) & 0x1) == 0) {
			printk(KERN_ERR PFX "woah!  AGP 3.0 device "
				"not operating in AGP 3.0 mode found on the "
				"secondary bus of an AGP 3.0 bridge operating "
				"with AGP 3.0 electricals!\n");
			ret = -ENODEV;
			goto free_and_exit;
		}

		tmp = (mstatus >> 10) & 0x7;
		cal_cycle = min(cal_cycle, tmp);

		/* figure the lesser rate */
		tmp = mstatus & 0x7;
		if(tmp < rate) 
			rate = tmp;
			
#if 1 /* FGL */
        sba       &= mstatus & (1<<9);
        fw        &= mstatus & (1<<4);
#endif
	}		

	/* Turn rate into something we can actually write out to AGPCMD */
	switch(rate) {
	case 0x1:
	case 0x2:
		break;
	case 0x3:
		rate = 0x2;
	        break;
	default:
		printk(KERN_ERR PFX "woah!  Bogus AGP rate "
			"value found advertised behind an AGP 3.0 "
			"bridge!\n");
		ret = -ENODEV;
		goto free_and_exit;
	}

	/*
	 * Call functions to divide target resources amongst the AGP 3.0
	 * masters.  This process is dramatically different depending on
	 * whether isochronous transfers are supported.
	 */
	if(isoch != 0) {
		if((ret = agp_3_0_isochronous_node_enable(dev_list,
		                                          ndevs)) != 0)
			goto free_and_exit;
	} else {
		if((ret = agp_3_0_nonisochronous_node_enable(dev_list,
		                                             ndevs)) != 0)
			goto free_and_exit;
	}

	/*
	 * Set the calculated minimum supported cal_cycle and minimum
	 * supported transfer rate in the target's AGPCMD register.
	 * Also set the AGP_ENABLE bit, effectively 'turning on' the
	 * target (this has to be done _before_ turning on the masters).
	 */
	pci_read_config_dword(td, agp_bridge.capndx + 0x08, &tcmd);

	tcmd &= ~(0x7 << 10);
	tcmd &= ~0x7;

	tcmd |= cal_cycle << 10;
	tcmd |= 0x1 << 8;
	tcmd |= rate;
#if 1 /* FGL */
    tcmd |= sba; /* according to spec: set for software compatibilitiy */
    tcmd |= fw;
#endif

	pci_write_config_dword(td, agp_bridge.capndx + 0x08, tcmd);

	/*
	 * Set the target's advertised arqsz value, the minimum supported
	 * transfer rate, and the AGP_ENABLE bit in each master's AGPCMD
	 * register.
	 */
	list_for_each(pos, head) {
		cur = list_entry(pos, struct agp_3_0_dev, list);
		dev = cur->dev;

		mcapndx = cur->capndx;

		pci_read_config_dword(dev, cur->capndx + 0x08, &mcmd);

		mcmd &= ~(0x7 << 13);
		mcmd &= ~0x7;

		mcmd |= arqsz << 13;
		mcmd |= 0x1 << 8;
		mcmd |= rate;
#if 1 /* FGL */
        mcmd |= sba; /* according to spec: set for software compatibilitiy */
        mcmd |= fw;
#endif

		pci_write_config_dword(dev, cur->capndx + 0x08, mcmd);
	}

free_and_exit:
	/* Be sure to free the dev_list */
	for(pos = head->next; pos != head;) {
		cur = list_entry(pos, struct agp_3_0_dev, list);

		pos = pos->next;
		kfree(cur);
	}
	kfree(dev_list);

get_out:
	return ret;
}


/* 
 * Entry point to AGP 3.0 host bridge init.  Check to see if we 
 * have an AGP 3.0 device operating in 3.0 mode.  Call 
 * agp_3_0_node_enable or agp_generic_agp_enable if we don't 
 * (AGP 3.0 devices are required to operate as AGP 2.0 devices 
 * when not using 3.0 electricals.
 */
int agp_generic_agp_3_0_enable(u32 mode)
{
	u32 ncapid, major, minor, agp_3_0;

	pci_read_config_dword(agp_bridge.dev, agp_bridge.capndx, &ncapid);

	major = (ncapid >> 20) & 0xf;
	minor = (ncapid >> 16) & 0xf;

	printk(KERN_INFO PFX "Found an AGP %d.%d compliant device. \n",
			  major, minor);
	if(major >= 3) {
		pci_read_config_dword(agp_bridge.dev, agp_bridge.capndx + 0x4, &agp_3_0);
		/* 
		 * Check to see if we are operating in 3.0 mode 
		 */
		if((agp_3_0 >> 3) & 0x1) {
			agp_3_0_node_enable(mode, minor);
			return 0; /* success */
		}
	}
#if 1 /* FGL */
    agp_generic_agp_v2_enable(mode); // redirect to FGL coding
#else
	agp_generic_agp_enable(mode);
#endif

    return 0; /* success */
}
