#! /usr/bin/env python3
import socket

from zeroconf import ServiceBrowser, Zeroconf


def resolve(service_name):
    host, port = None, 0

    class Listener:

        def add_service(self, zeroconf, type_, name):
            nonlocal host, port
            info = zeroconf.get_service_info(type_, name)
            port = info.port
            host = socket.inet_ntoa(info.address)
            zeroconf.close()


    ServiceBrowser(Zeroconf(), service_name, Listener()).join()
    return host, port


