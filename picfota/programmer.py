from easycli import SubCommand, Argument

from .protocol import WifiProgrammer


DEFAULT_TCP_PORT = 8585
DEFAULT_HOST = 'WPP.local'



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
            default=DEFAULT_HOST,
            help=f'Programmer\'s hostname, default: {DEFAULT_HOST}'
        )

    ]

    def __call__(self, args):
        resolve('_WPPS._tcp.local.')
        with WifiProgrammer(args.host, args.port) as p:
            if args.version:
                print(p.get_version())

