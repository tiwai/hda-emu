#define CONFIG_PROC_FS

#define CONFIG_PM
#define CONFIG_SND_HDA_POWER_SAVE
#define CONFIG_SND_HDA_POWER_SAVE_DEFAULT	0

#define CONFIG_SND_DEBUG
#define CONFIG_SND_DEBUG_VERBOSE

#define CONFIG_SND_HDA_GENERIC
#define CONFIG_SND_HDA_HWDEP
#define CONFIG_SND_HDA_INPUT_BEEP
#undef CONFIG_SND_HDA_JACK	/* unsupported */

#define CONFIG_SND_HDA_CODEC_REALTEK
#define CONFIG_SND_HDA_CODEC_CMEDIA
#define CONFIG_SND_HDA_CODEC_ANALOG
#define CONFIG_SND_HDA_CODEC_SIGMATEL
#define CONFIG_SND_HDA_CODEC_SI3054
#define CONFIG_SND_HDA_CODEC_ATIHDMI
#define CONFIG_SND_HDA_CODEC_CONEXANT
#define CONFIG_SND_HDA_CODEC_INTELHDMI
#define CONFIG_SND_HDA_CODEC_VIA
#define CONFIG_SND_HDA_CODEC_CA0110
#define CONFIG_SND_HDA_ELD
#define CONFIG_SND_HDA_RECONFIG
