# BladeRF_antenna_switch
Implements (1)libbladeRF and (2)a modified VHDL stream to control a PE42540 based psuedo-doppler RDF rig

(1) bladeTest.c is a libbladerf implementation that processes the sample stream from the bladeRF. Metadata is extracted, and stored in a modified sc16q11 format, with the third column containing the currently selected antenna.

(2) FPGA contains the Quartus prime modified bladeRF VHDL. It is based off the stock FPGA, and encodes the currently selected antenna within the RX sample metadata.This approach is intended to provide a contiguous sample stream, with synchronised and variable antenna switching.

(3) Simulink is used for designing the DoA algorithm, with code generation blocks intended to be ported to (1) for eventual real-time operation.
