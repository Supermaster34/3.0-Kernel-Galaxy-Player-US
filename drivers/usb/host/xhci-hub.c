/*
 * xHCI host controller driver
 *
 * Copyright (C) 2008 Intel Corp.
 *
 * Author: Sarah Sharp
 * Some code borrowed from the Linux EHCI driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <asm/unaligned.h>

#include "xhci.h"

static void xhci_hub_descriptor(struct xhci_hcd *xhci,
		struct usb_hub_descriptor *desc)
{
	int ports;
	u16 temp;

	ports = HCS_MAX_PORTS(xhci->hcs_params1);

	/* USB 3.0 hubs have a different descriptor, but we fake this for now */
	desc->bDescriptorType = 0x29;
	desc->bPwrOn2PwrGood = 10;	/* xhci section 5.4.9 says 20ms max */
	desc->bHubContrCurrent = 0;

	desc->bNbrPorts = ports;
	temp = 1 + (ports / 8);
	desc->bDescLength = 7 + 2 * temp;

	/* Why does core/hcd.h define bitmap?  It's just confusing. */
	memset(&desc->DeviceRemovable[0], 0, temp);
	memset(&desc->DeviceRemovable[temp], 0xff, temp);

	/* Ugh, these should be #defines, FIXME */
	/* Using table 11-13 in USB 2.0 spec. */
	temp = 0;
	/* Bits 1:0 - support port power switching, or power always on */
	if (HCC_PPC(xhci->hcc_params))
		temp |= 0x0001;
	else
		temp |= 0x0002;
	/* Bit  2 - root hubs are not part of a compound device */
	/* Bits 4:3 - individual port over current protection */
	temp |= 0x0008;
	/* Bits 6:5 - no TTs in root ports */
	/* Bit  7 - no port indicators */
<<<<<<< HEAD
	desc->wHubCharacteristics = (__force __u16) cpu_to_le16(temp);
=======
	desc->wHubCharacteristics = cpu_to_le16(temp);
}

/* Fill in the USB 2.0 roothub descriptor */
static void xhci_usb2_hub_descriptor(struct usb_hcd *hcd, struct xhci_hcd *xhci,
		struct usb_hub_descriptor *desc)
{
	int ports;
	u16 temp;
	__u8 port_removable[(USB_MAXCHILDREN + 1 + 7) / 8];
	u32 portsc;
	unsigned int i;

	ports = xhci->num_usb2_ports;

	xhci_common_hub_descriptor(xhci, desc, ports);
	desc->bDescriptorType = 0x29;
	temp = 1 + (ports / 8);
	desc->bDescLength = 7 + 2 * temp;

	/* The Device Removable bits are reported on a byte granularity.
	 * If the port doesn't exist within that byte, the bit is set to 0.
	 */
	memset(port_removable, 0, sizeof(port_removable));
	for (i = 0; i < ports; i++) {
		portsc = xhci_readl(xhci, xhci->usb2_ports[i]);
		/* If a device is removable, PORTSC reports a 0, same as in the
		 * hub descriptor DeviceRemovable bits.
		 */
		if (portsc & PORT_DEV_REMOVE)
			/* This math is hairy because bit 0 of DeviceRemovable
			 * is reserved, and bit 1 is for port 1, etc.
			 */
			port_removable[(i + 1) / 8] |= 1 << ((i + 1) % 8);
	}

	/* ch11.h defines a hub descriptor that has room for USB_MAXCHILDREN
	 * ports on it.  The USB 2.0 specification says that there are two
	 * variable length fields at the end of the hub descriptor:
	 * DeviceRemovable and PortPwrCtrlMask.  But since we can have less than
	 * USB_MAXCHILDREN ports, we may need to use the DeviceRemovable array
	 * to set PortPwrCtrlMask bits.  PortPwrCtrlMask must always be set to
	 * 0xFF, so we initialize the both arrays (DeviceRemovable and
	 * PortPwrCtrlMask) to 0xFF.  Then we set the DeviceRemovable for each
	 * set of ports that actually exist.
	 */
	memset(desc->u.hs.DeviceRemovable, 0xff,
			sizeof(desc->u.hs.DeviceRemovable));
	memset(desc->u.hs.PortPwrCtrlMask, 0xff,
			sizeof(desc->u.hs.PortPwrCtrlMask));

	for (i = 0; i < (ports + 1 + 7) / 8; i++)
		memset(&desc->u.hs.DeviceRemovable[i], port_removable[i],
				sizeof(__u8));
}

/* Fill in the USB 3.0 roothub descriptor */
static void xhci_usb3_hub_descriptor(struct usb_hcd *hcd, struct xhci_hcd *xhci,
		struct usb_hub_descriptor *desc)
{
	int ports;
	u16 port_removable;
	u32 portsc;
	unsigned int i;

	ports = xhci->num_usb3_ports;
	xhci_common_hub_descriptor(xhci, desc, ports);
	desc->bDescriptorType = 0x2a;
	desc->bDescLength = 12;

	/* header decode latency should be zero for roothubs,
	 * see section 4.23.5.2.
	 */
	desc->u.ss.bHubHdrDecLat = 0;
	desc->u.ss.wHubDelay = 0;

	port_removable = 0;
	/* bit 0 is reserved, bit 1 is for port 1, etc. */
	for (i = 0; i < ports; i++) {
		portsc = xhci_readl(xhci, xhci->usb3_ports[i]);
		if (portsc & PORT_DEV_REMOVE)
			port_removable |= 1 << (i + 1);
	}
	memset(&desc->u.ss.DeviceRemovable,
			(__force __u16) cpu_to_le16(port_removable),
			sizeof(__u16));
}

static void xhci_hub_descriptor(struct usb_hcd *hcd, struct xhci_hcd *xhci,
		struct usb_hub_descriptor *desc)
{

	if (hcd->speed == HCD_USB3)
		xhci_usb3_hub_descriptor(hcd, xhci, desc);
	else
		xhci_usb2_hub_descriptor(hcd, xhci, desc);

>>>>>>> GoogleSamsung/android-samsung-3.0-jb
}

static unsigned int xhci_port_speed(unsigned int port_status)
{
	if (DEV_LOWSPEED(port_status))
		return USB_PORT_STAT_LOW_SPEED;
	if (DEV_HIGHSPEED(port_status))
		return USB_PORT_STAT_HIGH_SPEED;
	if (DEV_SUPERSPEED(port_status))
		return USB_PORT_STAT_SUPER_SPEED;
	/*
	 * FIXME: Yes, we should check for full speed, but the core uses that as
	 * a default in portspeed() in usb/core/hub.c (which is the only place
	 * USB_PORT_STAT_*_SPEED is used).
	 */
	return 0;
}

/*
 * These bits are Read Only (RO) and should be saved and written to the
 * registers: 0, 3, 10:13, 30
 * connect status, over-current status, port speed, and device removable.
 * connect status and port speed are also sticky - meaning they're in
 * the AUX well and they aren't changed by a hot, warm, or cold reset.
 */
#define	XHCI_PORT_RO	((1<<0) | (1<<3) | (0xf<<10) | (1<<30))
/*
 * These bits are RW; writing a 0 clears the bit, writing a 1 sets the bit:
 * bits 5:8, 9, 14:15, 25:27
 * link state, port power, port indicator state, "wake on" enable state
 */
#define XHCI_PORT_RWS	((0xf<<5) | (1<<9) | (0x3<<14) | (0x7<<25))
/*
 * These bits are RW; writing a 1 sets the bit, writing a 0 has no effect:
 * bit 4 (port reset)
 */
#define	XHCI_PORT_RW1S	((1<<4))
/*
 * These bits are RW; writing a 1 clears the bit, writing a 0 has no effect:
 * bits 1, 17, 18, 19, 20, 21, 22, 23
 * port enable/disable, and
 * change bits: connect, PED, warm port reset changed (reserved zero for USB 2.0 ports),
 * over-current, reset, link state, and L1 change
 */
#define XHCI_PORT_RW1CS	((1<<1) | (0x7f<<17))
/*
 * Bit 16 is RW, and writing a '1' to it causes the link state control to be
 * latched in
 */
#define	XHCI_PORT_RW	((1<<16))
/*
 * These bits are Reserved Zero (RsvdZ) and zero should be written to them:
 * bits 2, 24, 28:31
 */
#define	XHCI_PORT_RZ	((1<<2) | (1<<24) | (0xf<<28))

/*
 * Given a port state, this function returns a value that would result in the
 * port being in the same state, if the value was written to the port status
 * control register.
 * Save Read Only (RO) bits and save read/write bits where
 * writing a 0 clears the bit and writing a 1 sets the bit (RWS).
 * For all other types (RW1S, RW1CS, RW, and RZ), writing a '0' has no effect.
 */
static u32 xhci_port_state_to_neutral(u32 state)
{
	/* Save read-only status and port state */
	return (state & XHCI_PORT_RO) | (state & XHCI_PORT_RWS);
}

static void xhci_disable_port(struct xhci_hcd *xhci, u16 wIndex,
		u32 __iomem *addr, u32 port_status)
{
	/* Write 1 to disable the port */
	xhci_writel(xhci, port_status | PORT_PE, addr);
	port_status = xhci_readl(xhci, addr);
	xhci_dbg(xhci, "disable port, actual port %d status  = 0x%x\n",
			wIndex, port_status);
}

static void xhci_clear_port_change_bit(struct xhci_hcd *xhci, u16 wValue,
		u16 wIndex, u32 __iomem *addr, u32 port_status)
{
	char *port_change_bit;
	u32 status;

	switch (wValue) {
	case USB_PORT_FEAT_C_RESET:
		status = PORT_RC;
		port_change_bit = "reset";
		break;
	case USB_PORT_FEAT_C_CONNECTION:
		status = PORT_CSC;
		port_change_bit = "connect";
		break;
	case USB_PORT_FEAT_C_OVER_CURRENT:
		status = PORT_OCC;
		port_change_bit = "over-current";
		break;
	case USB_PORT_FEAT_C_ENABLE:
		status = PORT_PEC;
		port_change_bit = "enable/disable";
		break;
	default:
		/* Should never happen */
		return;
	}
	/* Change bits are all write 1 to clear */
	xhci_writel(xhci, port_status | status, addr);
	port_status = xhci_readl(xhci, addr);
	xhci_dbg(xhci, "clear port %s change, actual port %d status  = 0x%x\n",
			port_change_bit, wIndex, port_status);
}

<<<<<<< HEAD
=======
static int xhci_get_ports(struct usb_hcd *hcd, __le32 __iomem ***port_array)
{
	int max_ports;
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);

	if (hcd->speed == HCD_USB3) {
		max_ports = xhci->num_usb3_ports;
		*port_array = xhci->usb3_ports;
	} else {
		max_ports = xhci->num_usb2_ports;
		*port_array = xhci->usb2_ports;
	}

	return max_ports;
}

/* Test and clear port RWC bit */
void xhci_test_and_clear_bit(struct xhci_hcd *xhci, __le32 __iomem **port_array,
				int port_id, u32 port_bit)
{
	u32 temp;

	temp = xhci_readl(xhci, port_array[port_id]);
	if (temp & port_bit) {
		temp = xhci_port_state_to_neutral(temp);
		temp |= port_bit;
		xhci_writel(xhci, temp, port_array[port_id]);
	}
}

>>>>>>> GoogleSamsung/android-samsung-3.0-jb
int xhci_hub_control(struct usb_hcd *hcd, u16 typeReq, u16 wValue,
		u16 wIndex, char *buf, u16 wLength)
{
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	int ports;
	unsigned long flags;
	u32 temp, status;
	int retval = 0;
	u32 __iomem *addr;

	ports = HCS_MAX_PORTS(xhci->hcs_params1);

	spin_lock_irqsave(&xhci->lock, flags);
	switch (typeReq) {
	case GetHubStatus:
		/* No power source, over-current reported per port */
		memset(buf, 0, 4);
		break;
	case GetHubDescriptor:
		xhci_hub_descriptor(xhci, (struct usb_hub_descriptor *) buf);
		break;
	case GetPortStatus:
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		status = 0;
		addr = &xhci->op_regs->port_status_base + NUM_PORT_REGS*(wIndex & 0xff);
		temp = xhci_readl(xhci, addr);
		xhci_dbg(xhci, "get port status, actual port %d status  = 0x%x\n", wIndex, temp);

		/* wPortChange bits */
		if (temp & PORT_CSC)
			status |= USB_PORT_STAT_C_CONNECTION << 16;
		if (temp & PORT_PEC)
			status |= USB_PORT_STAT_C_ENABLE << 16;
		if ((temp & PORT_OCC))
			status |= USB_PORT_STAT_C_OVERCURRENT << 16;
		/*
		 * FIXME ignoring suspend, reset, and USB 2.1/3.0 specific
		 * changes
		 */
		if (temp & PORT_CONNECT) {
			status |= USB_PORT_STAT_CONNECTION;
			status |= xhci_port_speed(temp);
		}
		if (temp & PORT_PE)
			status |= USB_PORT_STAT_ENABLE;
		if (temp & PORT_OC)
			status |= USB_PORT_STAT_OVERCURRENT;
		if (temp & PORT_RESET)
			status |= USB_PORT_STAT_RESET;
		if (temp & PORT_POWER)
			status |= USB_PORT_STAT_POWER;
		xhci_dbg(xhci, "Get port status returned 0x%x\n", status);
		put_unaligned(cpu_to_le32(status), (__le32 *) buf);
		break;
	case SetPortFeature:
		wIndex &= 0xff;
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		addr = &xhci->op_regs->port_status_base + NUM_PORT_REGS*(wIndex & 0xff);
		temp = xhci_readl(xhci, addr);
		temp = xhci_port_state_to_neutral(temp);
		switch (wValue) {
		case USB_PORT_FEAT_POWER:
			/*
			 * Turn on ports, even if there isn't per-port switching.
			 * HC will report connect events even before this is set.
			 * However, khubd will ignore the roothub events until
			 * the roothub is registered.
			 */
			xhci_writel(xhci, temp | PORT_POWER, addr);

			temp = xhci_readl(xhci, addr);
			xhci_dbg(xhci, "set port power, actual port %d status  = 0x%x\n", wIndex, temp);
			break;
		case USB_PORT_FEAT_RESET:
			temp = (temp | PORT_RESET);
			xhci_writel(xhci, temp, addr);

			temp = xhci_readl(xhci, addr);
			xhci_dbg(xhci, "set port reset, actual port %d status  = 0x%x\n", wIndex, temp);
			break;
		default:
			goto error;
		}
		temp = xhci_readl(xhci, addr); /* unblock any posted writes */
		break;
	case ClearPortFeature:
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS*(wIndex & 0xff);
		temp = xhci_readl(xhci, addr);
		temp = xhci_port_state_to_neutral(temp);
		switch (wValue) {
		case USB_PORT_FEAT_C_RESET:
		case USB_PORT_FEAT_C_CONNECTION:
		case USB_PORT_FEAT_C_OVER_CURRENT:
		case USB_PORT_FEAT_C_ENABLE:
			xhci_clear_port_change_bit(xhci, wValue, wIndex,
					addr, temp);
			break;
		case USB_PORT_FEAT_ENABLE:
			xhci_disable_port(xhci, wIndex, addr, temp);
			break;
		default:
			goto error;
		}
		break;
	default:
error:
		/* "stall" on error */
		retval = -EPIPE;
	}
	spin_unlock_irqrestore(&xhci->lock, flags);
	return retval;
}

/*
 * Returns 0 if the status hasn't changed, or the number of bytes in buf.
 * Ports are 0-indexed from the HCD point of view,
 * and 1-indexed from the USB core pointer of view.
 *
 * Note that the status change bits will be cleared as soon as a port status
 * change event is generated, so we use the saved status from that event.
 */
int xhci_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	unsigned long flags;
	u32 temp, status;
	int i, retval;
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	int ports;
	u32 __iomem *addr;

	ports = HCS_MAX_PORTS(xhci->hcs_params1);

	/* Initial status is no changes */
	retval = (ports + 8) / 8;
	memset(buf, 0, retval);
	status = 0;

	spin_lock_irqsave(&xhci->lock, flags);
	/* For each port, did anything change?  If so, set that bit in buf. */
	for (i = 0; i < ports; i++) {
		addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS*i;
		temp = xhci_readl(xhci, addr);
		if (temp & (PORT_CSC | PORT_PEC | PORT_OCC)) {
			buf[(i + 1) / 8] |= 1 << (i + 1) % 8;
			status = 1;
		}
	}
	spin_unlock_irqrestore(&xhci->lock, flags);
	return status ? retval : 0;
}
<<<<<<< HEAD
=======

#ifdef CONFIG_PM

int xhci_bus_suspend(struct usb_hcd *hcd)
{
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	int max_ports, port_index;
	__le32 __iomem **port_array;
	struct xhci_bus_state *bus_state;
	unsigned long flags;

	max_ports = xhci_get_ports(hcd, &port_array);
	bus_state = &xhci->bus_state[hcd_index(hcd)];

	spin_lock_irqsave(&xhci->lock, flags);

	if (hcd->self.root_hub->do_remote_wakeup) {
		port_index = max_ports;
		while (port_index--) {
			if (bus_state->resume_done[port_index] != 0) {
				spin_unlock_irqrestore(&xhci->lock, flags);
				xhci_dbg(xhci, "suspend failed because "
						"port %d is resuming\n",
						port_index + 1);
				return -EBUSY;
			}
		}
	}

	port_index = max_ports;
	bus_state->bus_suspended = 0;
	while (port_index--) {
		/* suspend the port if the port is not suspended */
		u32 t1, t2;
		int slot_id;

		t1 = xhci_readl(xhci, port_array[port_index]);
		t2 = xhci_port_state_to_neutral(t1);

		if ((t1 & PORT_PE) && !(t1 & PORT_PLS_MASK)) {
			xhci_dbg(xhci, "port %d not suspended\n", port_index);
			slot_id = xhci_find_slot_id_by_port(hcd, xhci,
					port_index + 1);
			if (slot_id) {
				spin_unlock_irqrestore(&xhci->lock, flags);
				xhci_stop_device(xhci, slot_id, 1);
				spin_lock_irqsave(&xhci->lock, flags);
			}
			t2 &= ~PORT_PLS_MASK;
			t2 |= PORT_LINK_STROBE | XDEV_U3;
			set_bit(port_index, &bus_state->bus_suspended);
		}
		if (hcd->self.root_hub->do_remote_wakeup) {
			if (t1 & PORT_CONNECT) {
				t2 |= PORT_WKOC_E | PORT_WKDISC_E;
				t2 &= ~PORT_WKCONN_E;
			} else {
				t2 |= PORT_WKOC_E | PORT_WKCONN_E;
				t2 &= ~PORT_WKDISC_E;
			}
		} else
			t2 &= ~PORT_WAKE_BITS;

		t1 = xhci_port_state_to_neutral(t1);
		if (t1 != t2)
			xhci_writel(xhci, t2, port_array[port_index]);

		if (hcd->speed != HCD_USB3) {
			/* enable remote wake up for USB 2.0 */
			__le32 __iomem *addr;
			u32 tmp;

			/* Add one to the port status register address to get
			 * the port power control register address.
			 */
			addr = port_array[port_index] + 1;
			tmp = xhci_readl(xhci, addr);
			tmp |= PORT_RWE;
			xhci_writel(xhci, tmp, addr);
		}
	}
	hcd->state = HC_STATE_SUSPENDED;
	bus_state->next_statechange = jiffies + msecs_to_jiffies(10);
	spin_unlock_irqrestore(&xhci->lock, flags);
	return 0;
}

int xhci_bus_resume(struct usb_hcd *hcd)
{
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	int max_ports, port_index;
	__le32 __iomem **port_array;
	struct xhci_bus_state *bus_state;
	u32 temp;
	unsigned long flags;

	max_ports = xhci_get_ports(hcd, &port_array);
	bus_state = &xhci->bus_state[hcd_index(hcd)];

	if (time_before(jiffies, bus_state->next_statechange))
		msleep(5);

	spin_lock_irqsave(&xhci->lock, flags);
	if (!HCD_HW_ACCESSIBLE(hcd)) {
		spin_unlock_irqrestore(&xhci->lock, flags);
		return -ESHUTDOWN;
	}

	/* delay the irqs */
	temp = xhci_readl(xhci, &xhci->op_regs->command);
	temp &= ~CMD_EIE;
	xhci_writel(xhci, temp, &xhci->op_regs->command);

	port_index = max_ports;
	while (port_index--) {
		/* Check whether need resume ports. If needed
		   resume port and disable remote wakeup */
		u32 temp;
		int slot_id;

		temp = xhci_readl(xhci, port_array[port_index]);
		if (DEV_SUPERSPEED(temp))
			temp &= ~(PORT_RWC_BITS | PORT_CEC | PORT_WAKE_BITS);
		else
			temp &= ~(PORT_RWC_BITS | PORT_WAKE_BITS);
		if (test_bit(port_index, &bus_state->bus_suspended) &&
		    (temp & PORT_PLS_MASK)) {
			if (DEV_SUPERSPEED(temp)) {
				temp = xhci_port_state_to_neutral(temp);
				temp &= ~PORT_PLS_MASK;
				temp |= PORT_LINK_STROBE | XDEV_U0;
				xhci_writel(xhci, temp, port_array[port_index]);
			} else {
				temp = xhci_port_state_to_neutral(temp);
				temp &= ~PORT_PLS_MASK;
				temp |= PORT_LINK_STROBE | XDEV_RESUME;
				xhci_writel(xhci, temp, port_array[port_index]);

				spin_unlock_irqrestore(&xhci->lock, flags);
				msleep(20);
				spin_lock_irqsave(&xhci->lock, flags);

				temp = xhci_readl(xhci, port_array[port_index]);
				temp = xhci_port_state_to_neutral(temp);
				temp &= ~PORT_PLS_MASK;
				temp |= PORT_LINK_STROBE | XDEV_U0;
				xhci_writel(xhci, temp, port_array[port_index]);
			}
			/* wait for the port to enter U0 and report port link
			 * state change.
			 */
			spin_unlock_irqrestore(&xhci->lock, flags);
			msleep(20);
			spin_lock_irqsave(&xhci->lock, flags);

			/* Clear PLC */
			xhci_test_and_clear_bit(xhci, port_array, port_index,
						PORT_PLC);

			slot_id = xhci_find_slot_id_by_port(hcd,
					xhci, port_index + 1);
			if (slot_id)
				xhci_ring_device(xhci, slot_id);
		} else
			xhci_writel(xhci, temp, port_array[port_index]);

		if (hcd->speed != HCD_USB3) {
			/* disable remote wake up for USB 2.0 */
			__le32 __iomem *addr;
			u32 tmp;

			/* Add one to the port status register address to get
			 * the port power control register address.
			 */
			addr = port_array[port_index] + 1;
			tmp = xhci_readl(xhci, addr);
			tmp &= ~PORT_RWE;
			xhci_writel(xhci, tmp, addr);
		}
	}

	(void) xhci_readl(xhci, &xhci->op_regs->command);

	bus_state->next_statechange = jiffies + msecs_to_jiffies(5);
	/* re-enable irqs */
	temp = xhci_readl(xhci, &xhci->op_regs->command);
	temp |= CMD_EIE;
	xhci_writel(xhci, temp, &xhci->op_regs->command);
	temp = xhci_readl(xhci, &xhci->op_regs->command);

	spin_unlock_irqrestore(&xhci->lock, flags);
	return 0;
}

#endif	/* CONFIG_PM */
>>>>>>> GoogleSamsung/android-samsung-3.0-jb
