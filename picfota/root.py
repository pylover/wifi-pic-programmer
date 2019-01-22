from .easycli import Root, Argument

#from .firmware import Firmware


class PicFota(Root):
    __help__ = 'WIFI PIC Programmer'

    __arguments__ = [
        Argument('-V', '--version', action='store_true', help='Show version')
    ]

    def __call__(self, args):
        if args.version:
            import picfota
            print(picfota.__version__)
            return

        return super().__call__(args)

#    __subcommands__ = [
#        Firmware,
#    ]
