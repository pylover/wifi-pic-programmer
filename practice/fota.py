#! /usr/bin/env python3.6

import re
import sys
import time
import struct
import base64
import socket
import asyncio
import argparse
import functools

import aiofiles
from easyq import client as easyqclient


__version__ = '0.1.0a'


DEFAULT_BIND_ADDRESS = '0.0.0.0:6666'


if len(sys.argv) > 1 and sys.argv[1] == '-V':
    print(__version__)
    sys.exit(0)


cli = argparse.ArgumentParser(
    prog=sys.argv[0],
    description='Utility for ESP8266 FOTA using easyq'
)
cli.add_argument('filename', help='Input file to download to device')
cli.add_argument('device_name', help='Device name')
cli.add_argument(
    'host',
    metavar='{EASYQHOST:}PORT',
    help='EasyQ server Address'
)
cli.add_argument('-V', '--version', action='store_true', help='Show version')
cli.add_argument(
    '-b', '--bind',
    metavar='{HOST:}PORT',
    default=DEFAULT_BIND_ADDRESS,
    help='Bind Address. if ommited the value from the config file will be used'
         'The default config value is: %s' % DEFAULT_BIND_ADDRESS
)


args = cli.parse_args()
finish_future = asyncio.Future()


class ServerProtocol(asyncio.Protocol):
    transport = None
    chunk = None
    peername = None
    file = None
    filectx = None

    class Patterns:
        regex = functools.partial(re.compile, flags=re.DOTALL + re.IGNORECASE)
        get = regex(
            b'^GET (?P<address>[0-9a-fx]+):(?P<length>[0-9a-fA-Fx]+)$'
        )

    def connection_made(self, transport):
        self.peername = transport.get_extra_info('peername')
        print(f'Connection from {self.peername}')
        self.transport = transport

    def connection_lost(self, exc):
        print(f'Connection lost: {self.peername}')
        finish_future.set_result(True)

    def eof_received(self):
        print(f'EOF Received: {self.peername}')
        self.transport.close()

    def data_received(self, data):
        print(f'Data received: {data.strip()}')
        if self.chunk:
            data = self.chunk + data

        # Splitting the received data with \n and adding buffered chunk if available
        lines = data.split(b';')

        # Adding unterminated command into buffer (if available) to be completed with the next call
        if not lines[-1].endswith(b';'):
            self.chunk = lines.pop()

        # Exiting if there is no command to process
        if not lines:
            return

        for command in lines:
            command = command.strip()
            if command:
                asyncio.ensure_future(self.process_command(command))

    async def process_command(self, command):
        m = self.Patterns.get.match(command)
        if m is not None:
            return await self.get(**m.groupdict())

        print(f'Invalid command: {command}')
        self.transport.write(b'ERROR: Invalid command: %s;\n' % command)

    async def get(self, address, length):
        address = eval(address)
        length = eval(length)
        data = await self.read_file(address, length)
        datalen = len(data)
        data = struct.pack('<H', datalen) + data
        self.transport.write(data)

    async def read_file(self, address, length):
        if self.file is None:
            self.filectx = aiofiles.open(args.filename, mode='rb')
            self.file = await self.filectx.__aenter__()

        await self.file.seek(address)
        return await self.file.read(length)


class Server:
    _server = None

    def __init__(self, bind=None, loop=None):
        self.loop = loop or asyncio.get_event_loop()

        # Host and Port to listen
        self.host, self.port = bind.split(':') if ':' in bind else ('', bind)

    async def start(self):
        self._server = await self.loop.create_server(
            ServerProtocol, self.host, self.port
        )

    async def close(self):
        self._server.close()
        await self._server.wait_closed()

    @property
    def address(self):
        return self._server.sockets[0].getsockname()




async def main(loop):
    s = Server(args.bind, loop)
    await s.start()

    device = args.device_name.encode()
    host = args.host
    host, port = host.split(':') if ':' in host else ('', host)
    host, port = socket.gethostbyname(host), int(port)

    await asyncio.sleep(1);
    c = await easyqclient.connect('fota.py', host, port, loop=loop)

    async def device_status_updated(queue, message):
        info = message.decode()
        info = info.replace(': ', ':')
        info = dict(i.split(':') for i in info.split(' '))
        print('Device Info: ', info)
        if info['Image'] != 'FOTA':
            await c.push(b'%s:fota' % device, b'R')
        else:
            await c.push(b'%s:fota' % device, b'S%b' % args.bind.encode())

    await c.pull(b'%s:fota:status' % device, device_status_updated)
    await asyncio.sleep(1)

    await c.push(b'%s:fota' % device, b'I')
    await finish_future



if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main(loop))

