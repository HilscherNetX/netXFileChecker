# netXFileChecker
Utility for netx 90 flash dump and file analysis

usage: netXFileChecker.exe options filename
options:
         -h    print help
         -u    use case A|B|C default:A, used for flash dump analysis and creation
         -s    split, in case of a flash dump analysis, separate files are created
         -fdl  creates a FDL - Flash Device Label

flash image analysis requires specification of use case (command line parameter -u)
no automatic evaluation of FDL supported

flash image analysis requires a input file with suffix *.bin

file analysis depends on file suffix
no automatic file type detection supported
FDL generator ignores -u option, creates just use case A FDL

file suffixs:
         .bin flash dump
         .nxi firmware
         .fdl flash device label
         .mxf maintenance firmware
         .hwc hardware config
         .mwc hardware config for maintenance firmware
         .rdt remanent data
         .mng managmenet data
         .upd update area file
         .nai user firmware on APP side
         .nxf legacy firmware netX 51, netx 52, etc.
