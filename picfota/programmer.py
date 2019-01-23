from easycli import SubCommand, Argument

from .protocol import WifiProgrammer
from .hosts import Hosts


DEFAULT_TCP_PORT = 8585
DEFAULT_SERVICE_NAME = '_WPPS._tcp.local'


class Programmer(SubCommand):
    __command__ = 'programmer'
    __arguments__ = [
        Argument(
            '-V', '--version',
            action='store_true',
            help='Show programmer\'s version'
        ),
        Argument(
            '-p', '--port',
            type=int,
            default=DEFAULT_TCP_PORT,
            help=f'Programmer\'s TCP port, default: {DEFAULT_TCP_PORT}'
        ),
        Argument(
            '-H', '--host',
            default=DEFAULT_SERVICE_NAME,
            help= \
                f'Programmer\'s hostname. if ends with ".local", the ' \
                f'hostname and port will be resolved using a mDNS query ' \
                f'broadcast. default: "{DEFAULT_SERVICE_NAME}"'
        ),
        Argument(
            '-f', '--force',
            action='store_true',
            help='Force to do a mDNS query, and do not use hosts cache'
        )
    ]

    def __call__(self, args):
        host, port = self.get_wifi_module_address(args)
        print(f'Connecting to {host}:{port}')
        with WifiProgrammer(host, port) as p:
            if args.version:
                print(p.get_version())

    def get_wifi_module_address(self, args):
        if not args.host.endswith('.local'):
            return args.host, args.port

        with Hosts() as h:
            if args.force:
                h.clear()

            return h[args.host]



