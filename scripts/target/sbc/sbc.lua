CPUS["Z80"] = true
MACHINES["Z80DAISY"] = true
MACHINES["Z80SIO"] = true
MACHINES["ACIA6850"] = true
MACHINES["CLOCK"] = true


CPUS["M680X0"] = true
MACHINES["MC68901"] = true
VIDEOS["V9938"] = true

CPUS["M6502"] = true
CPUS["M6809"] = true
MACHINES["68681"] = true

--------------------------------------
-- To be able to use IDE devices    --
--------------------------------------
BUSES["ATA"] = true
SOUNDS["CDDA"] = true
CPUS["FR"] = true
CPUS["MCS51"] = true
CPUS["MC68HC11"] = true
MACHINES["ATASTORAGE"] = true
MACHINES["ATAHLE"] = true

--------------------------------------
-- To be able to use serial devices --
--------------------------------------
CPUS["IE15"] = true
SOUNDS["AY8910"] = true
SOUNDS["BEEP"] = true
MACHINES["IE15"] = true
MACHINES["SWTPC8212"] = true
CPUS["M6800"] = true
BUSES["RS232"] = true
BUSES["SUNKBD"] = true
MACHINES["6821PIA"] = true
MACHINES["INS8250"] = true
MACHINES["INPUT_MERGER"] = true
VIDEOS["MC6845"] = true

function createProjects_sbc_sbc(_target, _subtarget)
	project ("sbc_sbc")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-sbc-sbc"))
	addprojectflags()
	precompiledheaders_novs()

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}

files{
	MAME_DIR .. "src/sbc/drivers/rc2014.cpp",
	MAME_DIR .. "src/sbc/drivers/rc2014_sio.cpp",
	MAME_DIR .. "src/sbc/drivers/rc2014_wbw.cpp",
	MAME_DIR .. "src/sbc/drivers/rosco.cpp",
	MAME_DIR .. "src/sbc/drivers/rosco_classicv2.cpp",
	MAME_DIR .. "src/sbc/drivers/grantz80.cpp",
	MAME_DIR .. "src/sbc/drivers/grant6809.cpp",
	MAME_DIR .. "src/sbc/drivers/grant6502.cpp",
	MAME_DIR .. "src/sbc/drivers/t68krc.cpp",
	MAME_DIR .. "src/sbc/drivers/tiny68k.cpp",
	MAME_DIR .. "src/sbc/drivers/ts2.cpp",
}
end

function linkProjects_sbc_sbc(_target, _subtarget)
	links {
		"sbc_sbc",
	}
end
