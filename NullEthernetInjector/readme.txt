The purpose of the injector is to allow NullEthernet.kext to attach to an existing but
not working PCI ethernet device.  This is done in an injector that is customized for the
device you wish to have NullEthernet attach to.  By doing this in the injector instead
of the main Info.plist for the kext itself, it is possible to update the kext without
editing the custom data.

To have NullEthernet attach to your PCI device (instead of using DSDT patch), customize 
this injector with your PCI device-id in the Info.plist for the injector.

The default IOPCIMatch here is 0x816810ec, a Realtek device with tons of support.  In
fact, my computer has it and I generally use RealtekRTL8111.kext with it, which is why
the default is this device... to facilitate my testing of the mechanism.

You should change the IOPCIMatch to your own device you wish the NullEthernet driver to
attach to.

You could provide the MAC address with a DSDT patch in the provider or you can provide the
MAC address in the injector.  To inject at the provider, use a _DSM injection of the property
"RM,MAC-address"

An example patch (this is based on the ProBook DSDT):

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

Of course, at the same time you do that patch you could add the necessary items
for EthernetBuiltIn:

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
