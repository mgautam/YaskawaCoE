#ifndef _RIO_574X0_H
#define _RIO_574X0_H


//int rio_setup_analog_inputs (int slavenum);
int rio_set_digital_output(int slavenum, int io_port);
int rio_reset_digital_output(int slavenum, int io_port);
int rio_toggle_dout(int slavenum, int io_port);
int rio_set_analog_output(int slavenum, int io_port, UDINT value);
#endif
