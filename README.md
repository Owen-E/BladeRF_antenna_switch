# BladeRF_antenna_switch
Implements (1)libbladeRF and (2)a modified VHDL stream to control a PE42540 based psuedo-doppler RDF rig

(1) bladeTest.c is a C frontend that takes n samples per antenna.This approach is bottlenecked at 125uS per antenna, and is inherently unreliable for synchronisation.

(2) FPGA contains the Quartus prime modified bladeRF VHDL. It is based off the stock FPGA, and encodes the currently selected antenna within the RX sample metadata.This approach is intended to provide a contiguous sample stream, with synchronised and variable antenna switching.
