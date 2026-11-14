// Extra pointer checks happen only while capturing a busy boundary/held report.
static bool SCReadable(UPtr at,U32 n){InterlockedIncrement(&g_scReadChecks);const bool okay=Readable(at,n);if(!okay && at)InterlockedIncrement(&g_scReadFailures);return okay;}
static SCNode SCReadNode(UPtr at) {
    SCNode n={};n.pointer=at;if(!at || !SCReadable(at,16u))return n;
    n.valid=1u;n.typeFlags=Read32(at);n.timeBits=Read32(at+4u);n.bytes=Read32(at+8u);n.next=Read32(at+12u);return n;
}
static SCView SCCapture(UPtr owner,UPtr self,U32 index) {
    SCView v={};v.owner=owner;v.self=self;v.vtable=Read32(self);v.recording=Read32(owner+0x4E58Cu);
    v.mode=Read32(owner+0x4E588u);v.clockBits=Read32(owner+0x4E9A0u);v.requestedBits=Read32(owner+0x4E99Cu);
    v.phase=Read32(owner+0x438ECu);v.transition=Read32(owner+0x4E9F4u);v.loading=Read8(owner+0x4E9ECu);v.countdown=Read32(owner+0x4E9F0u);
    v.vehiclePending=index==0u?Read8(self+0x34u):0xFFFFFFFFu;
    v.nodes[0]=SCReadNode(Read32(self+4u));
    if(v.nodes[0].valid)v.nodes[1]=SCReadNode(v.nodes[0].next);
    if(v.nodes[1].valid)v.nodes[2]=SCReadNode(v.nodes[1].next);
    const UPtr r=v.recording;
    if(!r || !SCReadable(r,0x8598u))return v;
    v.recordingValid=1u;v.recordingVtable=Read32(r);v.ioMode=Read32(r+4u);
    v.backend=v.recordingVtable==SCAddress(0xE2A088u)?1u:(v.recordingVtable==SCAddress(0xE2A33Cu)?2u:0u);
    v.capacity=Read32(r+0x855Cu);v.base=Read32(r+0x8560u);v.retained=Read32(r+0x8564u);
    v.filled=Read32(r+0x8568u);v.parse=Read32(r+0x856Cu);v.filePosition=Read32(r+0x8580u);
    v.inFlightBytes=Read32(r+0x8584u);v.endOfData=Read8(r+0x858Cu);v.active=Read8(r+0x8595u);v.eventHead=Read32(r+0x8554u);
    const U32 limit=v.base+v.capacity;
    if(v.base && v.capacity && v.capacity<=0x10000000u && limit>v.base &&
       v.retained>=v.base && v.retained<limit && v.filled>=v.base && v.filled<limit && v.parse>=v.base && v.parse<limit) {
        v.ringValid=1u;v.freeBytes=v.retained==v.filled?v.capacity:
            (v.retained>v.filled?v.retained-v.filled:v.capacity-(v.filled-v.retained));
        v.usedBytes=v.capacity-v.freeBytes;v.unparsedBytes=v.filled>=v.parse?v.filled-v.parse:v.capacity-(v.parse-v.filled);
    }
    return v;
}
