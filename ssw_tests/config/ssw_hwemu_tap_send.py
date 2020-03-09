#!/usr/bin/python
import os
import sys
import argparse
import subprocess

from time import sleep

from scapy.supersocket import TunTapInterface
from scapy.layers.l2 import Ether
from scapy.layers.inet import IP
from scapy.layers.inet import TCP

'''
1. Install brctl.
    sudo apt install bridge-utils
2. Add bridge.
    brctl addbr br0
3. Add interface to bridge.
    brctl addif br0 enp0s25
4. Create TAP interface.
    ip tuntap add dev tap0 mode tap
5. Add TAP to bridge.
    brctl addif br0 tap0
6. Link UP interface.
    ip link set up dev tap0
'''

class SswHwemuTapSendTest:
    def __init__(self, ifname=None):
        self.ifname = ifname
        self.tiface = ifname + '_test0'

        self.check_privileges()
        self._undos = []

    def check_privileges(self):
        if os.geteuid() != 0:
            exit("You need to have root privileges to run this script.\nPlease try again, this time using 'sudo'. Exiting.")

    def un(self, func, params):
        self._undos.append((func, params))

    def deinit(self):
        undo_ops = self._undos[::-1]

        print('====== [DE-INIT] ======')
        for op in undo_ops:
            op[0](*op[1])

    def bridge_add(self, brname):
        cmd = 'brctl addbr {}'.format(brname)
        self.shell(cmd)
        print('[+] Bridge \'{}\' has been created.'.format(brname))

    def bridge_del(self, brname):
        cmd = 'brctl delbr {}'.format(brname)
        self.shell(cmd)
        print('[-] Bridge \'{}\' has been deleted.'.format(brname))

    def bridge_iface_bind(self, brname, iface):
        cmd = 'brctl addif {} {}'.format(brname, iface)
        self.shell(cmd)
        print('[+] Interface \'{}\' has been bound to bridge \'{}\''.format(iface, brname))

    def bridge_iface_unbind(self, brname, iface):
        cmd = 'brctl delif {} {}'.format(brname, iface)
        self.shell(cmd)
        print('[-] Interface \'{}\' has been unbound from bridge \'{}\''.format(iface, brname))

    def iface_add(self, iface):
        cmd = 'ip tuntap add dev {} mode tap'.format(iface)
        self.shell(cmd)
        print('[+] Interface \'{}\' has been created.'.format(iface))

    def iface_del(self, iface):
        cmd = 'ip tuntap del dev {} mode tap'.format(iface)
        self.shell(cmd)
        print('[-] Interface \'{}\' has been deleted.'.format(iface))

    def iface_up(self, iface):
        cmd = 'ip link set up dev {}'.format(iface)
        self.shell(cmd)
        print('[+] Interface \'{}\' status set to UP.'.format(iface))

    def iface_down(self, iface):
        cmd = 'ip link set down dev {}'.format(iface)
        self.shell(cmd)
        print('[-] Interface \'{}\' status set to DOWN.'.format(iface))

    def shell(self, cmd):
        #print(' $ {}'.format(cmd))
        process = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
        output, error = process.communicate()
        self.check_status(process.returncode)
        return output, error, process.returncode

    def check_status(self, retcode):
        if retcode:
            print('[error] {}'.format(retcode))
            self.deinit()
            exit(1)

    def send_packet(self, packet):
        self.tap = TunTapInterface(iface=self.tiface)
        self.tap.open()
        self.tap.send(packet)
        self.tap.close()

    def tcase(self):
        brname = 'br_{}'.format(self.tiface)

        print('===== [TEST CASE] =====')
        # config environment
        self.bridge_add(brname)
        self.un(self.bridge_del, (brname,))

        self.iface_add(self.tiface)
        self.un(self.iface_del, (self.tiface,))

        self.bridge_iface_bind(brname, self.ifname)
        self.un(self.bridge_iface_unbind, (brname, self.ifname))

        self.bridge_iface_bind(brname, self.tiface)
        self.un(self.bridge_iface_unbind, (brname, self.tiface))

        self.iface_up(self.ifname)
        self.un(self.iface_down, (self.ifname,))

        self.iface_up(self.tiface)
        self.un(self.iface_down, (self.tiface,))

        self.iface_up(brname)
        self.un(self.iface_down, (brname,))

        import pdb
        pdb.set_trace()

        # 'd2:8c:b1:01:00:01'
        # Broadcast
        packet = Ether(dst='ff:ff:ff:ff:ff:ff', src='76:0f:cb:8b:4f:46')/IP(dst='255.255.255.255')/TCP()/'PTN PHN PAYLOAD'
        self.send_packet(packet)

        # IP broadcast
        # 'd2:8c:b1:01:00:01'
        packet = Ether(dst='d2:8c:b1:01:00:01', src='76:0f:cb:8b:4f:46')/IP(dst='255.255.255.255')/TCP()/'PTN PHN PAYLOAD'
        self.send_packet(packet)

        # Unicast
        packet = Ether(dst='d2:8c:b1:01:00:01', src='76:0f:cb:8b:4f:46')/IP(dst='10.10.10.10')/TCP()/'PTN PHN PAYLOAD'
        self.send_packet(packet)

        # L2 only
        packet = Ether(dst='d2:8c:b1:01:00:01', src='76:0f:cb:8b:4f:46')
        self.send_packet(packet)

        pdb.set_trace()

        self.deinit()

    def run(self):
        self.tcase()


def main():
    parser = argparse.ArgumentParser(description='SSW HWEMU TAP send test.')
    parser.add_argument('-s', '--swid', type=int, help='Switch ID', required=True)
    parser.add_argument('-p', '--port', type=int, help='Port number', required=True)
    args = parser.parse_args()

    ifname = 'sw{}p{}'.format(args.swid, args.port)

    test = SswHwemuTapSendTest(ifname=ifname)
    test.run()


if __name__ == '__main__':
    main()
