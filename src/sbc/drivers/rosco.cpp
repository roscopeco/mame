#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68010.h"
#include "machine/mc68901.h"
#include "imagedev/snapquik.h"
#include "formats/imageutl.h"
#include "video/v9938.h"
#include "multibyte.h"


class rosco_state : public driver_device
{
public:
	rosco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
        , m_mfp(*this, "mfp")
		, m_ram(*this, "ram")
		, m_v9958(*this, "v9958")
	{
	}

	void rosco(machine_config &config);
private:
	void rosco_map(address_map &map);
	void cpu_space_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_rom_cb);

	required_device<cpu_device> m_maincpu;
	required_device<mc68901_device> m_mfp;
	required_shared_ptr<uint16_t> m_ram;
	required_device<v9958_device> m_v9958;
};

/******************************************************************************
 Address Maps
******************************************************************************/

void rosco_state::rosco_map(address_map &map)
{
	map(0x000000, 0x0fffff).ram().share("ram");
	map(0xf80000, 0xf8002f).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0xf80000, 0xf80007).rw("v9958", FUNC(v9958_device::read), FUNC(v9958_device::write)).umask16(0xff00);
	map(0xfc0000, 0xffffff).rom().region("monitor", 0);
}

void rosco_state::cpu_space_map(address_map &map)
{
    map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68010_device::autovectors_map));
    map(0xfffff9, 0xfffff9).r(m_mfp, FUNC(mc68901_device::get_vector));
}

/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START( rosco )
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

void rosco_state::machine_start()
{
}

void rosco_state::machine_reset()
{
	m_maincpu->set_pc(0xfc0000);
}

/******************************************************************************
 Machine Drivers
******************************************************************************/

QUICKLOAD_LOAD_MEMBER(rosco_state::quickload_cb)
{
	int quick_length;
	int read_;
	std::vector<uint8_t> temp_copy;

	quick_length = image.length();
	temp_copy.resize(quick_length);
	read_ = image.fread(&temp_copy[0], quick_length);
	if (read_ != quick_length) {
                return std::make_pair(image_error::INVALIDLENGTH, std::string());
	}

	uint16_t *ROM16 = (uint16_t *) m_ram + 0x40000/2;
	for (int i = 0; i < quick_length; i += 2)
                ROM16[i / 2] = get_u16be(&temp_copy[i]); //pick_integer_be(&temp_copy[0], i, 2);
        return std::make_pair(std::error_condition(), std::string());
	
}

/*QUICKLOAD_LOAD_MEMBER(rosco_state::quickload_rom_cb)
{
	int quick_length;
	int read_;
	std::vector<uint8_t> temp_copy;

	quick_length = image.length();
	temp_copy.resize(quick_length);
	read_ = image.fread(&temp_copy[0], quick_length);
	if (read_ != quick_length) {
		return image_init_result::FAIL;
	}

	uint16_t *ROM16 = (uint16_t *) memregion("monitor")->base();
	for (int i = 0; i < quick_length; i += 2)
		ROM16[i / 2] = pick_integer_be(&temp_copy[0], i, 2);
	return image_init_result::PASS;
}
*/
void rosco_state::rosco(machine_config &config)
{
	M68010(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &rosco_state::rosco_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &rosco_state::cpu_space_map);

	MC68901(config, m_mfp, 10_MHz_XTAL);
	m_mfp->set_timer_clock(3.6864_MHz_XTAL);
	m_mfp->out_so_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	m_mfp->out_tdo_cb().set(m_mfp, FUNC(mc68901_device::rc_w));
	m_mfp->out_tdo_cb().append(m_mfp, FUNC(mc68901_device::tc_w));
	m_mfp->out_irq_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_mfp, FUNC(mc68901_device::si_w));
	rs232.set_option_device_input_defaults("terminal", terminal_defaults);

	/* quickload */
	QUICKLOAD(config, "quickload", "bin").set_load_callback(FUNC(rosco_state::quickload_cb));
//	QUICKLOAD(config, "rom", "bin").set_load_callback(FUNC(rosco_state::quickload_rom_cb));

	V9958(config, m_v9958, XTAL(21'477'272));
	m_v9958->set_screen_ntsc("screen");
	m_v9958->set_vram_size(0x20000);
	m_v9958->int_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}

/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( rosco )
	ROM_REGION16_BE(0x40000, "monitor", 0)
	ROM_LOAD( "rosco_m68k_mame.rom.bin", 0x00000, 0x10000, CRC(479aa782) SHA1(a769e5d6c5a3cb56ccf6017130fe69c732eeb993))
ROM_END

COMP( 2020, rosco, 0, 0, rosco, rosco, rosco_state, empty_init, "Ross Bamford", "rosco-m68k", MACHINE_IS_SKELETON )
