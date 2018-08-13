## ASPEED LPC Control

This tool is for controlling the LPC firmware bus on the ASPEED BMC devices
using the aspeed-lpc-ctrl kernel device.

It can be used to set the initial mapping of LPC to AHB bridge, and enable
"firmware cycles": the translation of LPC accesses to BMC address space access.
