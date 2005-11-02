/*
 * $Id: compat.h,v 1.21 2005/10/06 14:38:52 mchehab Exp $
 */

#ifndef _COMPAT_H
#define _COMPAT_H

#include <linux/i2c-id.h>
#include <linux/pm.h>
#include <linux/version.h>
#include <linux/utsname.h>
#include <linux/sched.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,18)
# define minor(x) MINOR(x)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
# define need_resched() (current->need_resched)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,19)
# define BUG_ON(condition) do { if ((condition)!=0) BUG(); } while(0)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23)
# define irqreturn_t void
# define IRQ_RETVAL(foobar)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
# define strlcpy(dest,src,len) strncpy(dest,src,(len)-1)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
# define iminor(inode) minor(inode->i_rdev)
#endif

#if defined(I2C_ADAP_CLASS_TV_ANALOG) && !defined(I2C_CLASS_TV_ANALOG)
# define  I2C_CLASS_TV_ANALOG  I2C_ADAP_CLASS_TV_ANALOG
# define  I2C_CLASS_TV_DIGITAL I2C_ADAP_CLASS_TV_DIGITAL
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
# define __user
# define __kernel
# define __iomem
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
# define pm_message_t                      u32
# define pci_choose_state(pci_dev, state)  (state)
# define PCI_D0                            (0)
# define assert_spin_locked(foobar)
#endif
#if !defined(I2C_HW_B_CX2388x)
# define I2C_HW_B_CX2388x I2C_HW_B_BT848
#endif
#if !defined(I2C_HW_SAA7134)
# define I2C_HW_SAA7134 I2C_ALGO_SAA7134
#endif
#if !defined(I2C_HW_SAA7146)
# define I2C_HW_SAA7146 I2C_ALGO_SAA7146
#endif

#if !defined(I2C_HW_B_EM2820)
#define I2C_HW_B_EM2820 I2C_HW_B_BT848
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
#define __le32 __u32
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)
static inline unsigned long msecs_to_jiffies(const unsigned int m)
{
#if HZ <= 1000 && !(1000 % HZ)
        return (m + (1000 / HZ) - 1) / (1000 / HZ);
#elif HZ > 1000 && !(HZ % 1000)
        return m * (HZ / 1000);
#else
        return (m * HZ + 999) / 1000;
#endif
}
static inline unsigned int jiffies_to_msecs(const unsigned long j)
{
#if HZ <= 1000 && !(1000 % HZ)
        return (1000 / HZ) * j;
#elif HZ > 1000 && !(HZ % 1000)
        return (j + (HZ / 1000) - 1)/(HZ / 1000);
#else
        return (j * 1000) / HZ;
#endif
}
static inline void msleep(unsigned int msecs)
{
	unsigned long timeout = msecs_to_jiffies(msecs);
	while (timeout) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		timeout = schedule_timeout(timeout);
	}
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
static inline unsigned long msleep_interruptible(unsigned int msecs)
{
	unsigned long timeout = msecs_to_jiffies(msecs);

	while (timeout) {
		set_current_state(TASK_INTERRUPTIBLE);
		timeout = schedule_timeout(timeout);
	}
	return jiffies_to_msecs(timeout);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
#define pm_message_t u32
#endif

#ifndef DVB_CVS
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
#undef HAVE_LGDT330X
#undef HAVE_TDA1004X
#endif
#endif

#endif
/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
