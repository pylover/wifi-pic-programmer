from easycli import SubCommand, Argument


DEFAULT_TCP_PORT = 8585


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
        )
    ]

    def __call__(self, args):
        print('Programmer', args)
