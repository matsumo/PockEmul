This has been quite a journey.... dumping the 1984 CC-40 computer's cartridges
and ROM.

Many thanks for the help from many people, including:

Hap (MESS contributor)
Lee (Previous CC-40 owner, who provided other dumps to verify against)
Ksarul (For bouncing questions off of)
JGardner (Of the Vintage Computer TI forums, for the assembly to dump the 32K BIOS)
Mattie Bush, for providing me my original CC-40 back in 1993.
Richard Bell, for providing service and support for existing CC-40 units.
MDude, for selling new CC-40s and accessories on eBay (go search for him!)

You can distribute these and use them however you want.  This Readme needs to stay with this document.

Contents:
---------

1) RAW RS232 dumps contains a comma delimited file of the actual memory
   locations as well as the ASCII character code of the memory contents.
2) Converted from RAW contains the actual 32K Binary ROMs.
3) Fixed 12K banks are where some of the banks had slightly adjusted orders to fit on EPROMs:
Example:
My raw dump (1K offset)	Lee's dump (1K offset)	CC-40 Memory Map
>0000->0FFF		>5000->5FFF		>5000->5FFF
>1000->1FFF 		>6000->6FFF		>6000->6FFF
>2000->2FFF		>7000->7FFF		>7000->7FFF
>3000->3FFF		>0000->0FFF		>8000->8FFF
>4000->4FFF		>1000->1FFF		>9000->9FFF
>5000->5FFF		>2000->2FFF		>A000->AFFF
>6000->6FFF		>3000->3FFF		>B000->BFFF
>7000->7FFF 		>4000->4FFF		>C000->CFFF

4) Provided by Lee are ROMs provided by Lee, which I verified mine against
5) Note that cc40.bin is the 32K BIOS ROM for the CC-40.
6) TMS70C20.bin is the 2K on-processor ROM for the CC-40.
7) cc40_2krom.bin and cc40_copy2.bin are just duplicates of the above.

J Guidry - "acadiel"
www.hexbus.com
06/08/2014

