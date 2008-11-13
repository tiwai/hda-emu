#ifndef __HDA_LOG_H
#define __HDA_LOG_H

enum {
	HDA_LOG_ERR,
	HDA_LOG_INFO,
	HDA_LOG_VERB,
};

int hda_log_init(const char *file);
void hda_log(int level, const char *fmt, ...);
void hda_log_echo(int level, const char *fmt, ...);
#define hda_log_printk(fmt, args...) hda_log(HDA_LOG_INFO, fmt, ##args)

void hda_log_dump_proc(void);
void hda_log_jack_state(int nid);
void hda_log_set_jack(int nid, int val);

void hda_test_suspend(void);
void hda_test_resume(void);
void hda_exec_verb(int nid, int verb, int parm);

void hda_codec_reset(void);
int hda_codec_reconfig(void);

#endif /* __HDA_LOG_H */
