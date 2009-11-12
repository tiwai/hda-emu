#include <sound/core.h>
#include "hda_codec.h"
#include "hda_local.h"

#if defined(CONFIG_SND_HDA_POWER_SAVE) && defined(CONFIG_SND_HDA_HWDEP)
int snd_hda_hwdep_add_power_sysfs(struct hda_codec *codec)
{
	return 0;
}
#endif

