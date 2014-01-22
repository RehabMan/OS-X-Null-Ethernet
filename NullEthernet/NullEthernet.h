/* NullEthernet.h -- NullEthernet driver class implementation.
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

#define EXPORT __attribute__((visibility("default")))
#define PRIVATE __attribute__((visibility("hidden")))

#ifdef DEBUG
#define DebugLog(args...) IOLog("NullEthernet: " args)
#else
#define DebugLog(args...) 
#endif
#define AlwaysLog(args...) IOLog("NullEthernet: " args)

#define	RELEASE(x)	if(x){(x)->release();(x)=NULL;}

#include <Availability.h>

#define MakeKernelVersion(maj,min,rev) (maj<<16|min<<8|rev)
#include <libkern/version.h>
#define GetKernelVersion() MakeKernelVersion(version_major,version_minor,version_revision)

enum
{
	MEDIUM_INDEX_AUTO = 0,
	MEDIUM_INDEX_10HD,
	MEDIUM_INDEX_10FD,
	MEDIUM_INDEX_100HD,
	MEDIUM_INDEX_100FD,
	MEDIUM_INDEX_1000FD,
	MEDIUM_INDEX_COUNT
};

#define MBit 1000000

enum {
    kSpeed1000MBit = 1000*MBit,
    kSpeed100MBit = 100*MBit,
    kSpeed10MBit = 10*MBit,
};

enum
{
    kPowerStateOff = 0,
    kPowerStateOn,
    kPowerStateCount
};

#define kNameLenght 64

#define NullEthernet org_rehabman_NullEthernet

class EXPORT NullEthernet : public IOEthernetController
{
    typedef IOEthernetController super;
	OSDeclareDefaultStructors(org_rehabman_NullEthernet)
	
public:
	/* IOService (or its superclass) methods. */
	virtual bool start(IOService *provider);
	virtual void stop(IOService *provider);
	virtual bool init(OSDictionary *properties);
	virtual void free();
	
	/* Power Management Support */
	virtual IOReturn registerWithPolicyMaker(IOService *policyMaker);
    virtual IOReturn setPowerState(unsigned long powerStateOrdinal, IOService *policyMaker );
	virtual void systemWillShutdown(IOOptionBits specifier);

	/* IONetworkController methods. */
	virtual IOReturn enable(IONetworkInterface *netif);
	virtual IOReturn disable(IONetworkInterface *netif);
	
	virtual UInt32 outputPacket(mbuf_t m, void *param);
	
	virtual void getPacketBufferConstraints(IOPacketBufferConstraints *constraints) const;
	
	virtual IOOutputQueue* createOutputQueue();
	
	virtual const OSString* newVendorString() const;
	virtual const OSString* newModelString() const;
	
	virtual IOReturn selectMedium(const IONetworkMedium *medium);
	virtual bool configureInterface(IONetworkInterface *interface);
	
	/* Methods inherited from IOEthernetController. */
	virtual IOReturn getHardwareAddress(IOEthernetAddress *addr);
	virtual IOReturn setHardwareAddress(const IOEthernetAddress *addr);
	virtual IOReturn setPromiscuousMode(bool active);
	virtual IOReturn setMulticastMode(bool active);
	virtual IOReturn setMulticastList(IOEthernetAddress *addrs, UInt32 count);
	virtual IOReturn getChecksumSupport(UInt32 *checksumMask, UInt32 checksumFamily, bool isOutput);
	virtual IOReturn setMaxPacketSize(UInt32 maxSize);
	virtual IOReturn getMaxPacketSize(UInt32 *maxSize) const;
	virtual IOReturn getMinPacketSize(UInt32 *minSize) const;
    virtual IOReturn setWakeOnMagicPacket(bool active);
    virtual IOReturn getPacketFilters(const OSSymbol *group, UInt32 *filters) const;
    
    virtual UInt32 getFeatures() const;
    
    virtual IOReturn setProperties(OSObject* props);

private:
    IOACPIPlatformDevice* m_pProvider;
    IONetworkInterface* m_netif;
    bool m_isEnabled;
    bool m_linkUp;
    unsigned long m_powerState;
    UInt32 unitNumber;
	OSDictionary *m_mediumDict;
	IONetworkMedium *m_mediumTable[MEDIUM_INDEX_COUNT];
    IOCommandGate* m_pCommandGate;
    char m_rgMacAddr[kIOEthernetAddressSize];

    PRIVATE bool setupMediumDict();

    PRIVATE IOReturn setPropertiesGated(OSObject* props);
};

