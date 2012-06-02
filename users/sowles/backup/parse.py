#!/usr/bin/python2.7
from struct import *

f=open("/debug/e1000/tx_ring")
buf=f.read()
tx_ring=unpack("IIIIIIIIHH",buf)
f=open("/debug/e1000/rx_ring")
buf=f.read()
rx_ring=unpack("IIIIIIIIIHH",buf)

print"""
struct e1000_tx_ring {
        void *desc=0x%X;
        dma_addr_t dma=0x%X;
        unsigned int size=%u;
        unsigned int count=%u;
        unsigned int next_to_use=%u;
        unsigned int next_to_clean=%u;
        struct e1000_buffer *buffer_info=0x%X;
        u16 tdh=%u;
        u16 tdt=%u;
        bool last_tx_tso=%u;
};
""" %tuple(tx_ring)
print """
struct e1000_rx_ring {
        void *desc=0x%X;
        dma_addr_t dma=0x%x;
        unsigned int size=%u;
        unsigned int count=%u;
        unsigned int next_to_use=%u;
        unsigned int next_to_clean=%u;
        struct e1000_buffer *buffer_info=0x%X;
        struct sk_buff *rx_skb_top=0x%X;
        int cpu=%i;
        u16 rdh=%u;
        u16 rdt=%u;
};
""" %tuple(rx_ring)
