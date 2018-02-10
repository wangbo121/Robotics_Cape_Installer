/*******************************************************************************
* SPI - Serial Peripheral Interface
*
* The Sitara's SPI bus is broken out on two JST SH 6-pin sockets labeled SPI1.1
* and SPI1.2 These share clock and serial IO signals, but have independent slave
* select lines.
*
* The slaves can be selected automatically by the SPI linux driver or manually
* with select/deselect_spi_slave() functions. On the Robotics Cape, slave 1
* can be used in either mode, but slave 2 must be selected manually. On the
* BB Blue either slave can be used in manual or automatic modes.
*******************************************************************************/
#ifndef RC_SPI_H
#define RC_SPI_H

typedef enum ss_mode_t{
	SS_MODE_AUTO,
	SS_MODE_MANUAL
} ss_mode_t;

#define SPI_MODE_CPOL0_CPHA0 0
#define SPI_MODE_CPOL0_CPHA1 1
#define SPI_MODE_CPOL1_CPHA0 2
#define SPI_MODE_CPOL1_CPHA1 3

int rc_spi_init(ss_mode_t ss_mode, int spi_mode, int speed_hz, int slave);
int rc_spi_fd(int slave);
int rc_spi_close(int slave);
int rc_manual_select_spi_slave(int slave);
int rc_manual_deselect_spi_slave(int slave);
int rc_spi_send_bytes(char* data, int bytes, int slave);
int rc_spi_read_bytes(char* data, int bytes, int slave);
int rc_spi_transfer(char* tx_data, int tx_bytes, char* rx_data, int slave);
int rc_spi_write_reg_byte(char reg_addr, char data, int slave);
char rc_spi_read_reg_byte(char reg_addr, int slave);
int rc_spi_read_reg_bytes(char reg_addr, char* data, int bytes, int slave);

#endif // RC_SPI_H