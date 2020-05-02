#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80sio.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"

class rc2014_sio_state : public driver_device
{
public:
	rc2014_sio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void rc2014_sio(machine_config &config);

private:

	void mem_map(address_map &map);
	void mem_io(address_map &map);

	required_device<cpu_device> m_maincpu;

	virtual void machine_start() override;
	virtual void machine_reset() override;
};


/******************************************************************************
 Machine Start/Reset
******************************************************************************/

void rc2014_sio_state::machine_start()
{
}

void rc2014_sio_state::machine_reset()
{
}

/******************************************************************************
 Address Maps
******************************************************************************/

void rc2014_sio_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0x4000);
	map(0x4000, 0xffff).ram();
}


void rc2014_sio_state::mem_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x80).rw("sio", FUNC(z80sio_device::ca_r), FUNC(z80sio_device::ca_w));
	map(0x81, 0x81).rw("sio", FUNC(z80sio_device::da_r), FUNC(z80sio_device::da_w));
	map(0x82, 0x82).rw("sio", FUNC(z80sio_device::cb_r), FUNC(z80sio_device::cb_w));	
	map(0x83, 0x83).rw("sio", FUNC(z80sio_device::db_r), FUNC(z80sio_device::db_w));
}
// RC2014 standard addresses for Spencer's official SIO/2: (type 2)
// 0x80   Channel A control registers (read and write)
// 0x81   Channel A data registers (read and write)
// 0x82   Channel B control registers (read and write)
// 0x83   Channel B data registers (read and write)


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START( rc2014_sio )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

void rc2014_sio_state::rc2014_sio(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(7'372'800));
	m_maincpu->set_addrmap(AS_PROGRAM, &rc2014_sio_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &rc2014_sio_state::mem_io);

	clock_device &uart_clock(CLOCK(config, "uart_clock", XTAL(7'372'800)));
	uart_clock.signal_handler().set("sio", FUNC(z80sio_device::txca_w));
	uart_clock.signal_handler().append("sio", FUNC(z80sio_device::rxca_w));

	clock_device &uart_clock2(CLOCK(config, "uart_clock2", XTAL(7'372'800)));
	uart_clock2.signal_handler().set("sio", FUNC(z80sio_device::txcb_w));
	uart_clock2.signal_handler().append("sio", FUNC(z80sio_device::rxcb_w));

	/* Devices */
	z80sio_device& sio(Z80SIO(config, "sio", 7372800));
	sio.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}


/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( rc2014_sio )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "24886009.bin",    0x0000, 0x10000, CRC(9e9e6f00) SHA1(2e520dbec0b229e0afe3c30cd81f0f8422de97d9))
	ROM_FILL(0x0001, 1, 0xc3) 
ROM_END


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                         FULLNAME                            FLAGS
COMP( 2014, rc2014_sio, 0,      0,      rc2014_sio,  rc2014_sio, rc2014_sio_state, empty_init, "Spencer Owen", "RC2014 (SIO)", MACHINE_IS_SKELETON )
