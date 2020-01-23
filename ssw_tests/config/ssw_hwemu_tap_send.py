#!/usr/bin/python
from scapy.supersocket import TunTapInterface
from scapy.layers.l2 import Ether
from scapy.layers.inet import IP
from scapy.layers.inet import TCP

class SswHwemuTapSendTest:
    def __init__(self, ifname=None):
        self.ifname = ifname
        self.tap = TunTapInterface(iface=ifname)

    def tcase(self):
        self.tap.open()
        packet = Ether(dst='ff:ff:ff:ff:ff:ff', src='02:4b:15:26:bb:93')/IP(dst='255.255.255.255')/TCP()/'PTN PHN PAYLOAD'

        self.tap.send(packet)

    def run(self):
        self.tcase()



if __name__ == '__main__':
    test = SswHwemuTapSendTest('sw1p0')
    test.run()