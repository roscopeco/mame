#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/6850acia.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"

class rc2014_state : public driver_device
{
public:
	rc2014_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void rc2014(machine_config &config);

private:
	DECLARE_WRITE_LINE_MEMBER( acia_irq_w );

	void mem_map(address_map &map);
	void mem_io(address_map &map);

	required_device<cpu_device> m_maincpu;
};


WRITE_LINE_MEMBER( rc2014_state::acia_irq_w )
{
	m_maincpu->set_input_line(0, state);
}

/******************************************************************************
 Address Maps
******************************************************************************/

void rc2014_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0x4000);
	map(0x4000, 0xffff).ram();
}


void rc2014_state::mem_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x81).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
}

/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START( rc2014 )
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

void rc2014_state::rc2014(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(7'372'800));
	m_maincpu->set_addrmap(AS_PROGRAM, &rc2014_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &rc2014_state::mem_io);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 7372800));
	uart_clock.signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	uart_clock.signal_handler().append("acia", FUNC(acia6850_device::write_rxc));

	acia6850_device &acia(ACIA6850(config, "acia", 0));
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	acia.irq_handler().set(FUNC(rc2014_state::acia_irq_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set("acia", FUNC(acia6850_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}


/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( rc2014 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	//ROM_LOAD( "r0000000.bin",    0x0000, 0x2000, CRC(850e3ec7) SHA1(7c9613e160b324ee1ed42fc48d98bbc215791e81))
	ROM_LOAD( "24886009.bin",    0x0000, 0x10000, CRC(9e9e6f00) SHA1(2e520dbec0b229e0afe3c30cd81f0f8422de97d9))
	ROM_FILL(0x0001, 1, 0xc3) 
ROM_END


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                         FULLNAME                            FLAGS
COMP( 2014, rc2014, 0,      0,      rc2014,  rc2014, rc2014_state, empty_init, "Spencer Owen", "RC2014", MACHINE_IS_SKELETON )
