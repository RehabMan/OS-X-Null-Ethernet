## Null Ethernet Network Driver by RehabMan

The purpose of this driver is to enable Mac App Store access even if you don’t have a built-in Ethernet port with supporting drivers.  The idea is to use a USB WiFi and this “fake ethernet” driver to make the system still allow Mac App Store access.

Note: Having a real Ethernet port with working driver or a real PCIe WiFi device with working AirPort driver is a better idea.  This is to be used as only a last resort where getting working built-in WiFi or Ethernet is not possible.

Warning: I do not know if this works.  I do not have a USB WiFi to test with.  I do know that I was able to get it to load as IOBuiltIn=Yes, bsdname en0.  I did the test with my real Ethernet driver renamed, EthernetBuiltIn=No, and no PCIe WiFi card installed.  You will need to test in a real scenario.  Don’t expect it to work as the code probably needs more work.

Update: It definitely works.  Confirmed a handfull of times on the day of the initial release.


### How to Install (DSDT/SSDT method):

Install the kext itself, NullEthernet.kext, with Kext Wizard or your favorite kext installer.  The Release build should be used for normal installs.  Use the Debug build for troubleshooting.

In order to cause the kext to be loaded, you need to apply the DSDT patch provided in patch.txt.  It adds a fake device ‘RMNE’ which the driver will attach to.

You may also use the provided ssdt-rmne.aml as an extra SSDT for the bootloader to load in lieu of implementing the DSDT patch.

To install the SSDT:

Chameleon: Place in /Extra/ssdt.aml or /Extra/ssdt-1.aml, /Extra/ssdt-2.aml, depending on what SSDTs you already have installed.

Clover: Place in /EFI/CLOVER/patched/ssdt-X.aml where 'X' is some number that you're not already using for SSDTs.


### Alternate way to install (PCIe/injector method)

This method is most appropriate if you have a PCIe Ethernet device that is not supported or has drivers that do not work for it.  Instead of creating a special device in your DSDT, the kext can directly attach to the PCI device.  Install the kext, NullEthernet.kext, with Kext Wizard or your favorite kext installer, just as above.

Instead of DSDT patching, you will instead create a custom NullEthernetInjector.  To do so, modify the Info.plist in NullEthernetInjector.kext/Contents/Info.plist.  Change IOPCIMatch to suit your device.  Also, change the MAC-address property as appropriate (default is 12:34:56:78:ab:cd).  Then install your custom NullEthernetInjector.kext like you would any kext.  When updates happen to the main NullEthernet.kext this step does not need to be repeated.


### Providing the MAC address:

Obviously the kext cannot provide a real MAC address from the device.  Instead it just reports the MAC address you provide.  The MAC address provided to the system from this kext is determined as follows:

- the default MAC address is 01:02:03:04:05:06

- if there is a MAC-address property provided in NullEthernet.kext/Contents/Info.plist (or if using the injector, in NullEthernetInjector.kext/Contents/Info.plist), that one is used. By default, there is no MAC-address specified in NullEthernet.kext/Contents/Info.plist.

- if loading from ACPI (DSDT patch), a method called MAC can provide a MAC address.  The return value must be a buffer of exactly 6-bytes.  The default in patch.txt is 11:22:33:44:55:66

- lastly, the provider (parent object of the kext), can provide a MAC address as a property named "RM,MAC-address".  This property is usually set via a _DSM method in the DSDT.  For example, here is an example patch that works on the HP ProBook for its built-in Ethernet device:


```
into method label _DSM parent_label NIC parent_label RP06 remove_entry;
into device label NIC parent_label RP06 insert
begin
Method (_DSM, 4, NotSerialized)\n
{\n
    If (LEqual (Arg2, Zero)) { Return (Buffer() { 0x03 } ) }\n
    Return (Package()\n
    {\n
        "RM,MAC-address", Buffer() { 0x11, 0x22, 0x33, 0x66, 0x55, 0x44 },\n
    })\n
}\n
end;
```

You can even set the built-in property while you're at it:

```
into method label _DSM parent_label NIC parent_label RP06 remove_entry;
into device label NIC parent_label RP06 insert
begin
Method (_DSM, 4, NotSerialized)\n
{\n
    If (LEqual (Arg2, Zero)) { Return (Buffer() { 0x03 } ) }\n
    Return (Package()\n
    {\n
        "RM,MAC-address", Buffer() { 0x11, 0x22, 0x33, 0x66, 0x55, 0x44 },\n
        "built-in", Buffer() { 0x00 },\n
        "device_type", Buffer() { "ethernet" },\n
    })\n
}\n
end;
```


Note:

If you've previously had network interfaces setup (eg. not a fresh install), you may need to remove all network interfaces and set them up again.  To do that, go into SysPrefs->Network and remove all interfaces, Apply, then remove /Library/Preferences/SystemConfiguration/NetworkInterfaces.plist.  Reboot, then add all your network interfaces back, starting with NullEthernet.


### Downloads:

Downloads are available on Bitbucket:

https://bitbucket.org/RehabMan/os-x-null-ethernet/downloads


### Build Environment

My build environment is currently Xcode 6.1, using SDK 10.6, targeting OS X 10.6.

This kext can be built with any of the following SDKs: 10.8, 10.7, or 10.6 but only by enabling
the hacks previously used in the code (see DISABLE_ALL_HACKS in the source code)

In addition, it can be built supporting any of these OS X targets: 10.8, 10.7, or 10.6.

For greatest compatibility, the provided build is SDK 10.6 targeting 10.6.


### 32-bit Builds

This project does not support 32-bit builds.  It is coded for 64-bit only.


### Source Code:

The source code is maintained at the following sites:

https://bitbucket.org/RehabMan/os-x-null-ethernet

https://github.com/RehabMan/OS-X-Null-Ethernet


### Feedback:

Please use the following threads for feedback, questions, and help:

http://www.tonymacx86.com/network/122884-mac-app-store-access-nullethernet-kext.html

http://www.insanelymac.com/forum/topic/295534-mac-app-store-access-with-nullethernetkext/


### Known issues:


### Change Log:

2014-10-16 v1.0.3

- Fixed some minor bugs

- Build with Xcode 6.1


2014-04-27 v1.0.1

- Lowered dependency requirements for better compatibility with Snow Leopard.

2014-01-23 v1.0.0

- Added ability to attach to a PCIe device instead of ACPI device.

2014-01-21

- Initial build created by RehabMan


### History:

The source for this kext is heavily based on Mieze's RealtekRTL8111.kext.  I simply stripped everything meaningful from it to leave just a shell of an Ethernet driver.
