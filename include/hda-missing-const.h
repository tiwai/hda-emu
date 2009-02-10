#ifndef AC_EAPDBTL_BALANCED
#define AC_EAPDBTL_BALANCED		(1<<0)
#define AC_EAPDBTL_EAPD			(1<<1)
#define AC_EAPDBTL_LR_SWAP		(1<<2)
#endif

#ifndef AC_DEFCFG_MISC_NO_PRESENCE
#define AC_DEFCFG_MISC_NO_PRESENCE	(1<<0)
#endif

#ifndef AC_PAR_VOL_KNB_CAP
#define AC_PAR_VOL_KNB_CAP		0x13
#endif

#ifndef AC_CONV_CHANNEL
#define AC_CONV_CHANNEL			(0xf<<0)
#define AC_CONV_STREAM			(0xf<<4)
#define AC_CONV_STREAM_SHIFT		4
#endif

#ifndef AC_SDI_SELECT
#define AC_SDI_SELECT			(0xf<<0)
#endif

#ifndef AC_DIG2_CC
#define AC_DIG2_CC			(0x7f<<0)
#endif

#ifndef AC_VERB_GET_DIGI_CONVERT_1
#define AC_VERB_GET_DIGI_CONVERT_1		0x0f0d
#define AC_VERB_GET_DIGI_CONVERT_2		0x0f0e /* unused */
#endif

#ifndef AC_VERB_GET_GPIO_WAKE_MASK
#define AC_VERB_GET_GPIO_WAKE_MASK		0x0f18
#endif

#ifndef AC_VERB_GET_GPIO_UNSOLICITED_RSP_MASK
#define AC_VERB_GET_GPIO_UNSOLICITED_RSP_MASK	0x0f19
#endif

#ifndef AC_VERB_GET_GPIO_STICKY_MASK
#define AC_VERB_GET_GPIO_STICKY_MASK		0x0f1a
#endif

#ifndef AC_PWRST_SETTING
#define AC_PWRST_SETTING		(0xf<<0)
#define AC_PWRST_ACTUAL			(0xf<<4)
#define AC_PWRST_ACTUAL_SHIFT		4
#define AC_PWRST_D0			0x00
#define AC_PWRST_D1			0x01
#define AC_PWRST_D2			0x02
#define AC_PWRST_D3			0x03
#endif

#ifndef AC_UNSOL_TAG
#define AC_UNSOL_TAG			(0x3f<<0)
#define AC_UNSOL_ENABLED		(1<<7)
#endif

#ifndef AC_USRSP_EN
#define AC_USRSP_EN			AC_UNSOL_ENABLED
#endif

#ifndef AC_PCAP_NUM_COEF_SHIFT
#define AC_PCAP_NUM_COEF_SHIFT		8
#endif

#ifndef AC_GPIO_IO_COUNT
#define AC_GPIO_IO_COUNT		(0xff<<0)
#define AC_GPIO_O_COUNT			(0xff<<8)
#define AC_GPIO_O_COUNT_SHIFT		8
#define AC_GPIO_I_COUNT			(0xff<<16)
#define AC_GPIO_I_COUNT_SHIFT		16
#define AC_GPIO_UNSOLICITED		(1<<30)
#define AC_GPIO_WAKE			(1<<31)
#endif
