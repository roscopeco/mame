#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6850acia.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"

class grant6502_state : public driver_device
{
public:
	grant6502_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void grant6502(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

/******************************************************************************
 Address Maps
******************************************************************************/

void grant6502_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram();
	map(0xa000, 0xbfff).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xc000, 0xffff).rom().region("maincpu", 0);
}

/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START( grant6502 )
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

void grant6502_state::grant6502(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(1'843'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &grant6502_state::mem_map);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 1843200));
	uart_clock.signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	uart_clock.signal_handler().append("acia", FUNC(acia6850_device::write_rxc));

	acia6850_device &acia(ACIA6850(config, "acia", 0));
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set("acia", FUNC(acia6850_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}


/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( grant6502 )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "osi_bas.bin",    0x0000, 0x4000, CRC(0b1d8348) SHA1(482451aa8cc0c470ce9706b43bfa093df47c8ab1))
ROM_END


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                         FULLNAME                            FLAGS
COMP( 200?, grant6502, 0,      0,      grant6502,  grant6502, grant6502_state, empty_init, "Grant Serle", "Grant Serle 6502", MACHINE_IS_SKELETON )
