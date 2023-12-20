#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "machine/clock.h"

class ts2_state : public driver_device
{
public:
	ts2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void ts2(machine_config &config);
private:
	void ts2_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void duart_output(uint8_t data);
	void duart_irq_handler(int state);

	void cpu_space_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

/******************************************************************************
 Address Maps
******************************************************************************/

/*
+------------------+---------------------------------------------------+
| Address Range    | Description                                       |
+------------------+---------------------------------------------------+
|$000000 - $000007 | EPROM1. Stack pointer/Reset vector.               |
+------------------+---------------------------------------------------+
|$000008 - $003FFF | RAM1 (16K). U11 (odd), U13 (even).                |
|$004000 - $007FFF | RAM2 (16K). U15 (odd), U17 (even).                |
+------------------+---------------------------------------------------+
|$008000 - $00BFFF | EPROM1 (16K). U12 (odd), U14 (even).              |
|$00C000 - $00FFFF | EPROM1 (16K). U16 (odd), U18 (even).              |
+------------------+---------------------------------------------------+
|$010040 - $010042 | 6850 Console/terminal ACIA U30 (even addresses).  |
|$010040           | read=status register, write=control register.     |
|$010042           | read=receive data reg, write=transmit data reg.   |
|                  |                                                   |
|$010041 - $010043 | 6850 Auxiliary/host ACIA U29 (odd addresses).     |
|$010041           | read=status register, write=control register.     |
|$010043           | read=receive data reg, write=transmit data reg.   |
+------------------+---------------------------------------------------+
 1   8          EPROM1        $00000000-$00000007
 2  16K         RAM1          $00000008-$00003FFF
 3  16K         RAM2          $00004000-$00007FFF
 4  16K         EPROM1        $00008000-$0000BFFF
 5  16K         EPROM2        $0000C000-$0000FFFF
 6  64          Peripheral 1  $01000000-$0100003F
 7  64          Peripheral 2  $01000040-$0100007F
 8  64          Peripheral 3  $01000080-$010000BF
 9  64          Peripheral 4  $010000C0-$010000FF
10  64          Peripheral 5  $01000100-$0100013F
11  64          Peripheral 6  $01000140-$0100017F
12  64          Peripheral 7  $01000180-$010001BF
13  64          Peripheral 8  $010001C0-$010001FF
*/
void ts2_state::ts2_map(address_map &map)
{
	map(0x000000, 0x000007).rom().region("bios", 0x8000);
	map(0x000008, 0x003fff).ram();
	map(0x004000, 0x007fff).ram();
	map(0x008000, 0x00ffff).rom().region("bios", 0x8000);
	map(0x010040, 0x010043).rw("acia",FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0xff00);
}

/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START( ts2 )
INPUT_PORTS_END

static const input_device_default terminal_defaults[] =
{
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
	{ nullptr, 0, 0 }
};

/******************************************************************************
 Machine Start/Reset
******************************************************************************/

void ts2_state::machine_start()
{
}

void ts2_state::machine_reset()
{
}

/******************************************************************************
 Machine Drivers
******************************************************************************/

void ts2_state::ts2(machine_config &config)
{
	M68000(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ts2_state::ts2_map);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 2457600 / 16));
														 
	uart_clock.signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	uart_clock.signal_handler().append("acia", FUNC(acia6850_device::write_rxc));

	acia6850_device &acia(ACIA6850(config, "acia", 0));
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set("acia", FUNC(acia6850_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", terminal_defaults);
}

/******************************************************************************
 ROM Definitions
******************************************************************************/
ROM_START( ts2 )
	ROM_REGION16_BE(0x10000, "bios", 0)
	ROM_LOAD( "tutor13.bin", 0x0000, 0x0c000, CRC(fb922bec) SHA1(94456894598e233f2e7ead53bb3df10dc5435d11))
	//ROM_LOAD( "monitor.bin", 0x0000, 0x08c58, CRC(cd8577b8) SHA1(8ea36b7866d718cace5937f1402c7ddd7f9b68cf))
ROM_END

COMP( 2017, ts2, 0, 0, ts2, ts2, ts2_state, empty_init, "Jeff Tranter", "TS2", MACHINE_IS_SKELETON )
