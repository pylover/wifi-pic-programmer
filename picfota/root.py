from .easycli import Root, Argument

#from .firmware import Firmware


class PicFota(Root):
    
    __arguments__ = [
        Argument('-v', '--v', action='store_true', help='Show version')
    ]

#    __subcommands__ = [
#        Firmware,
#    ]
