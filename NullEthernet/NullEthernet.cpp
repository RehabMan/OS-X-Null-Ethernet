/* NullEthernet.c -- NullEthernet driver class implementation.
 *
 * Copyright (c) 2014 RehabMan <racerrehabman@gmail.com>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#include "NullEthernet.h"

#pragma mark --- public methods ---

OSDefineMetaClassAndStructors(org_rehabman_NullEthernet, IOEthernetController)

/* IOService (or its superclass) methods. */

bool NullEthernet::init(OSDictionary *properties)
{
    DebugLog("init() ===>\n");

    if (!super::init(properties))
    {
        DebugLog("super::init failed\n");
        return false;
    }

    m_pProvider = NULL;
    m_netif = NULL;
    m_isEnabled = false;
    unitNumber = 0;

    // load default MAC address (can be overridden in DSDT)
    static unsigned char rgDefault[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    bcopy(rgDefault, m_rgMacAddr, kIOEthernetAddressSize);
    if (properties)
    {
        OSData* pData = OSDynamicCast(OSData, properties->getObject("MAC-address"));
        if (pData && pData->getLength() == kIOEthernetAddressSize)
            bcopy(pData->getBytesNoCopy(), m_rgMacAddr, kIOEthernetAddressSize);
    }

    DebugLog("init() <===>\n");
    return true;
}

void NullEthernet::free()
{
    DebugLog("free() ===>\n");
    //TODO: implement

    super::free();
    DebugLog("free() <===>\n");
}

bool NullEthernet::start(IOService *provider)
{
    DebugLog("start() ===>\n");

    if (!super::start(provider))
    {
        AlwaysLog("NullEthernet: IOEthernetController::start failed.\n");
        return false;
    }

    // retain provider...
    m_pProvider = OSDynamicCast(IOACPIPlatformDevice, provider);
    if (!m_pProvider)
    {
        AlwaysLog("NullEthernet: No provider.\n");
        return false;
    }
    m_pProvider->retain();

    // get MAC address from DSDT if provided...
    OSObject *ret;
    IOReturn result = m_pProvider->evaluateObject("MAC", &ret);
    if (kIOReturnSuccess == result && NULL != ret)
    {
        OSData* pData = OSDynamicCast(OSData, ret);
        if (pData && pData->getLength() == kIOEthernetAddressSize)
        {
            bcopy(pData->getBytesNoCopy(), m_rgMacAddr, kIOEthernetAddressSize);
            AlwaysLog("Using MAC address from DSDT: %02x:%02x:%02x:%02x:%02x:%02x\n", m_rgMacAddr[0], m_rgMacAddr[1], m_rgMacAddr[2], m_rgMacAddr[3], m_rgMacAddr[4], m_rgMacAddr[5]);
        }
        else
        {
            AlwaysLog("Using default MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", m_rgMacAddr[0], m_rgMacAddr[1], m_rgMacAddr[2], m_rgMacAddr[3], m_rgMacAddr[4], m_rgMacAddr[5]);
        }
        ret->release();
    }

    if (!attachInterface(reinterpret_cast<IONetworkInterface**>(&m_netif)))
    {
        AlwaysLog("NullEthernet: attachInterface() failed.\n");
        goto error1;
    }

    AlwaysLog("NullEthernet: NullEthernet v0.01 starting.\n");

done:
    DebugLog("start() <===\n");
    return true;

error1:
    OSSafeReleaseNULL(m_pProvider);
    return false;
}

void NullEthernet::stop(IOService *provider)
{
    DebugLog("stop() ===>\n");
    if (m_netif)
    {
        detachInterface(m_netif);
        m_netif = NULL;
    }
    for (int i = MEDIUM_INDEX_AUTO; i < MEDIUM_INDEX_COUNT; i++)
        m_mediumTable[i] = NULL;

    OSSafeReleaseNULL(m_pProvider);
    
    super::stop(provider);
    DebugLog("stop() <===\n");
}

/* Property support */
IOReturn NullEthernet::setPropertiesGated(OSObject* props)
{
    OSDictionary* dict = OSDynamicCast(OSDictionary, props);
    if (!dict)
        return kIOReturnSuccess;
#if 0
    // allow intrMitigateValue to change on the fly...
	if (OSNumber* num = OSDynamicCast(OSNumber, dict->getObject(kIntrMitigateName))) {
		intrMitigateValue = (int)num->unsigned16BitValue();
        setProperty(kIntrMitigateName, intrMitigateValue, 16);
        WriteReg16(IntrMitigate, intrMitigateValue);
    }
#endif
    return kIOReturnSuccess;
}

IOReturn NullEthernet::setProperties(OSObject* props)
{
    if (m_pCommandGate) {
        // syncronize through workloop...
        IOReturn result = m_pCommandGate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &NullEthernet::setPropertiesGated), props);
        if (kIOReturnSuccess != result)
            return result;
    }
    return kIOReturnSuccess;
    //return super::setProperties(props);
}

/* Power Management Support */
static IOPMPowerState powerStateArray[kPowerStateCount] =
{
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, kIOPMDeviceUsable, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
};

IOReturn NullEthernet::registerWithPolicyMaker(IOService *policyMaker)
{    
    DebugLog("registerWithPolicyMaker() ===>\n");
    
    m_powerState = kPowerStateOn;
    
    DebugLog("registerWithPolicyMaker() <===\n");

    return policyMaker->registerPowerDriver(this, powerStateArray, kPowerStateCount);
}

IOReturn NullEthernet::setPowerState(unsigned long powerStateOrdinal, IOService *policyMaker)
{
    IOReturn result = IOPMAckImplied;
    
    DebugLog("setPowerState() ===>\n");
        
    if (powerStateOrdinal == m_powerState) {
        DebugLog("Already in power state %lu.\n", powerStateOrdinal);
        goto done;
    }
    DebugLog("switching to power state %lu.\n", powerStateOrdinal);

#if 0
    if (powerStateOrdinal == kPowerStateOff)
        commandGate->runAction(setPowerStateSleepAction);
    else
        commandGate->runAction(setPowerStateWakeAction);
#endif

    m_powerState = powerStateOrdinal;
    
done:
    DebugLog("setPowerState() <===\n");

    return result;
}

void NullEthernet::systemWillShutdown(IOOptionBits specifier)
{
    DebugLog("systemWillShutdown() ===>\n");
    
    if ((kIOMessageSystemWillPowerOff | kIOMessageSystemWillRestart) & specifier)
        disable(m_netif);
    
    DebugLog("systemWillShutdown() <===\n");

    /* Must call super shutdown or system will stall. */
    super::systemWillShutdown(specifier);
}

/* IONetworkController methods. */
IOReturn NullEthernet::enable(IONetworkInterface *netif)
{
    const IONetworkMedium *selectedMedium;
    IOReturn result = kIOReturnError;
    
    DebugLog("enable() ===>\n");

    if (m_isEnabled) {
        DebugLog("Interface already enabled.\n");
        result = kIOReturnSuccess;
        goto done;
    }

    selectedMedium = getSelectedMedium();
    if (!selectedMedium) {
        DebugLog("No medium selected. Falling back to autonegotiation.\n");
        selectedMedium = m_mediumTable[MEDIUM_INDEX_AUTO];
    }
    selectMedium(selectedMedium);
    setLinkStatus(kIONetworkLinkValid);

    m_isEnabled = true;

    result = kIOReturnSuccess;
    
    DebugLog("enable() <===\n");

done:
    return result;
}

IOReturn NullEthernet::disable(IONetworkInterface *netif)
{
    IOReturn result = kIOReturnSuccess;
    
    DebugLog("disable() ===>\n");

    if (!m_isEnabled)
        goto done;

    setLinkStatus(kIONetworkLinkValid);
    m_linkUp = false;
    m_isEnabled = false;

    DebugLog("disable() <===\n");

done:
    return result;
}

UInt32 NullEthernet::outputPacket(mbuf_t m, void *param)
{
    return kIOReturnOutputDropped;
}

void NullEthernet::getPacketBufferConstraints(IOPacketBufferConstraints *constraints) const
{
    DebugLog("getPacketBufferConstraints() ===>\n");

	constraints->alignStart = kIOPacketBufferAlign8;
	constraints->alignLength = kIOPacketBufferAlign8;
    
    DebugLog("getPacketBufferConstraints() <===\n");
}

IOOutputQueue* NullEthernet::createOutputQueue()
{
    DebugLog("createOutputQueue() ===>\n");
    DebugLog("createOutputQueue() <===\n");

    return IOBasicOutputQueue::withTarget(this);
}

const OSString* NullEthernet::newVendorString() const
{
    DebugLog("newVendorString() ===>\n");
    DebugLog("newVendorString() <===\n");

    return OSString::withCString("RehabMan");
}

const OSString* NullEthernet::newModelString() const
{
    DebugLog("newModelString() ===>\n");
    DebugLog("newModelString() <===\n");
    
    return OSString::withCString("NullEthernet");
}

bool NullEthernet::configureInterface(IONetworkInterface *interface)
{
    char modelName[kNameLenght];
    ////IONetworkData *data;
    bool result;

    DebugLog("configureInterface() ===>\n");

    result = super::configureInterface(interface);
    if (!result)
        goto done;
#if 0
    /* Get the generic network statistics structure. */
    data = interface->getParameter(kIONetworkStatsKey);
    if (data) {
        netStats = (IONetworkStats *)data->getBuffer();
        if (!netStats) {
            AlwaysLog("NullEthernet: Error getting IONetworkStats\n.");
            result = false;
            goto done;
        }
    }
    /* Get the Ethernet statistics structure. */    
    data = interface->getParameter(kIOEthernetStatsKey);
    if (data) {
        etherStats = (IOEthernetStats *)data->getBuffer();
        if (!etherStats) {
            AlwaysLog("NullEthernet: Error getting IOEthernetStats\n.");
            result = false;
            goto done;
        }
    }
#endif
    unitNumber = interface->getUnitNumber();
    snprintf(modelName, kNameLenght, "NullEthernet ACPI NULE0000 Gigabit Ethernet");
    setProperty("model", modelName);
    
    DebugLog("configureInterface() <===\n");

done:
    return result;
}

/* Methods inherited from IOEthernetController. */
IOReturn NullEthernet::getHardwareAddress(IOEthernetAddress *addr)
{
    IOReturn result = kIOReturnError;
    
    DebugLog("getHardwareAddress() ===>\n");
    
    if (addr) {
        bcopy(m_rgMacAddr, addr->bytes, kIOEthernetAddressSize);
        result = kIOReturnSuccess;
    }
    
    DebugLog("getHardwareAddress() <===\n");

    return result;
}

IOReturn NullEthernet::setPromiscuousMode(bool active)
{
    DebugLog("setPromiscuousMode() ===>\n");
    DebugLog("setPromiscuousMode() <===\n");

    return kIOReturnSuccess;
}

IOReturn NullEthernet::setMulticastMode(bool active)
{    
    DebugLog("setMulticastMode() ===>\n");
    DebugLog("setMulticastMode() <===\n");
    
    return kIOReturnSuccess;
}

IOReturn NullEthernet::setMulticastList(IOEthernetAddress *addrs, UInt32 count)
{
    DebugLog("setMulticastList() ===>\n");
    DebugLog("setMulticastList() <===\n");

    return kIOReturnSuccess;
}

IOReturn NullEthernet::getChecksumSupport(UInt32 *checksumMask, UInt32 checksumFamily, bool isOutput)
{
    IOReturn result = kIOReturnUnsupported;

    DebugLog("getChecksumSupport() ===>\n");
    DebugLog("getChecksumSupport() <===\n");

    return result;
}

IOReturn NullEthernet::setMaxPacketSize (UInt32 maxSize)
{
    IOReturn result = kIOReturnUnsupported;
    
done:
    return result;
}

//REVIEW: was in if_ether.h
#define ETH_DATA_LEN	1500		/* Max. octets in payload	 */

IOReturn NullEthernet::getMaxPacketSize (UInt32 *maxSize) const
{
    IOReturn result = kIOReturnBadArgument;
    
    if (maxSize) {
        *maxSize = ETH_DATA_LEN + ETHER_HDR_LEN + ETHER_CRC_LEN;
        result = kIOReturnSuccess;
    }
    return result;
}

IOReturn NullEthernet::getMinPacketSize (UInt32 *minSize) const
{
    IOReturn result = super::getMinPacketSize(minSize);
    
done:
    return result;
}

IOReturn NullEthernet::setWakeOnMagicPacket(bool active)
{
    IOReturn result = kIOReturnUnsupported;

    DebugLog("setWakeOnMagicPacket() ===>\n");
    DebugLog("setWakeOnMagicPacket() <===\n");

    return result;
}

IOReturn NullEthernet::getPacketFilters(const OSSymbol *group, UInt32 *filters) const
{
    IOReturn result = kIOReturnSuccess;

    DebugLog("getPacketFilters() ===>\n");
    DebugLog("getPacketFilters() <===\n");

    return result;
}


UInt32 NullEthernet::getFeatures() const
{
    DebugLog("getFeatures() ===>\n");
    DebugLog("getFeatures() <===\n");

    return 0;
}

IOReturn NullEthernet::setHardwareAddress(const IOEthernetAddress *addr)
{
    IOReturn result = kIOReturnError;
    
    DebugLog("setHardwareAddress() ===>\n");
    DebugLog("setHardwareAddress() <===\n");
    
    return result;
}

IOReturn NullEthernet::selectMedium(const IONetworkMedium *medium)
{
    IOReturn result = kIOReturnSuccess;
    
    DebugLog("selectMedium() ===>\n");
    
    if (medium) {
#if 0
        switch (medium->getIndex()) {
            case MEDIUM_INDEX_AUTO:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
                break;
                
            case MEDIUM_INDEX_10HD:
                autoneg = AUTONEG_DISABLE;
                speed = SPEED_10;
                duplex = DUPLEX_HALF;
                break;
                
            case MEDIUM_INDEX_10FD:
                autoneg = AUTONEG_DISABLE;
                speed = SPEED_10;
                duplex = DUPLEX_FULL;
                break;
                
            case MEDIUM_INDEX_100HD:
                autoneg = AUTONEG_DISABLE;
                speed = SPEED_100;
                duplex = DUPLEX_HALF;
                break;
                
            case MEDIUM_INDEX_100FD:
                autoneg = AUTONEG_DISABLE;
                speed = SPEED_100;
                duplex = DUPLEX_FULL;
                break;
                
            case MEDIUM_INDEX_1000FD:
                autoneg = AUTONEG_DISABLE;
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
                break;
        }
#endif
        setCurrentMedium(medium);
    }
    
    DebugLog("selectMedium() <===\n");
    
done:
    return result;
}

#pragma mark --- data structure initialization methods ---

static IOMediumType mediumTypeArray[MEDIUM_INDEX_COUNT] = {
    kIOMediumEthernetAuto,
    (kIOMediumEthernet10BaseT | kIOMediumOptionHalfDuplex),
    (kIOMediumEthernet10BaseT | kIOMediumOptionFullDuplex),
    (kIOMediumEthernet100BaseTX | kIOMediumOptionHalfDuplex),
    (kIOMediumEthernet100BaseTX | kIOMediumOptionFullDuplex),
    (kIOMediumEthernet1000BaseT | kIOMediumOptionFullDuplex)
};

static UInt32 mediumSpeedArray[MEDIUM_INDEX_COUNT] = {
    0,
    10 * MBit,
    10 * MBit,
    100 * MBit,
    100 * MBit,
    1000 * MBit
};

bool NullEthernet::setupMediumDict()
{
	IONetworkMedium *medium;
    UInt32 i;
    bool result = false;

    m_mediumDict = OSDictionary::withCapacity(MEDIUM_INDEX_COUNT + 1);
    if (m_mediumDict) {
        for (i = MEDIUM_INDEX_AUTO; i < MEDIUM_INDEX_COUNT; i++) {
            medium = IONetworkMedium::medium(mediumTypeArray[i], mediumSpeedArray[i], 0, i);
            
            if (!medium)
                goto error1;

            result = IONetworkMedium::addMedium(m_mediumDict, medium);
            medium->release();

            if (!result)
                goto error1;

            m_mediumTable[i] = medium;
        }
    }
    result = publishMediumDictionary(m_mediumDict);
    
    if (!result)
        goto error1;

done:
    return result;
    
error1:
    AlwaysLog("NullEthernet: Error creating medium dictionary.\n");
    m_mediumDict->release();
    
    for (i = MEDIUM_INDEX_AUTO; i < MEDIUM_INDEX_COUNT; i++)
        m_mediumTable[i] = NULL;

    goto done;
}


