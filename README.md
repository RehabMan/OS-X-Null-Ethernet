## Null Ethernet Network Driver by RehabMan

The purpose of this driver is to enable Mac App Store access even if you don’t have a built-in Ethernet port with supporting drivers.  The idea is to use a USB WiFi and this “fake ethernet” driver to make the system still allow Mac App Store access.

Note: Having a real Ethernet port with working driver or a real PCIe WiFi device with working AirPort driver is a better idea.  This is to be used as only a last resort where getting working built-in WiFi or Ethernet is not possible.

Warning: I do not know if this works.  I do not have a USB WiFi to test with.  I do know that I was able to get it to load as IOBuiltIn=Yes, bsdname en0.  I did the test with my real Ethernet driver renamed, EthernetBuiltIn=No, and no PCIe WiFi card installed.  You will need to test in a real scenario.  Don’t expect it to work as the code probably needs more work.


### How to Install:

Install the kext, NullEthernet.kext, itself with Kext Wizard or your favorite kext installer.  The Release build should be used for normal installs.  Use the Debug build for troubleshooting.

In order to cause the kext to be loaded, you need to apply the DSDT patch provided in patch.txt.  It adds a fake device ‘RMNE’ which the driver will attach to.


### Downloads:

Downloads are available on Bitbucket:

TBD


### Build Environment

My build environment is currently Xcode 5.02, using SDK 10.6, targeting OS X 10.6.

This kext can be built with any of the following SDKs: 10.8, 10.7, or 10.6 but only by enabling
the hacks previously used in the code (see DISABLE_ALL_HACKS in the source code)

In addition, it can be built supporting any of these OS X targets: 10.8, 10.7, or 10.6.

For greatest compatibility, the provided build is SDK 10.6 targeting 10.6.


### 32-bit Builds

This project does not support 32-bit builds.  It is coded for 64-bit only.


### Source Code:

The source code is maintained at the following sites:

https://code.google.com/p/os-x-null-ethernet/

https://github.com/RehabMan/OS-X-Null-Ethernet


### Feedback:

Please use the following thread on tonymacx86.com for feedback, questions, and help:

TBD


### Known issues:


### Change Log:

2014-01-21 (RehabMan)

- Initial build created by RehabMan


