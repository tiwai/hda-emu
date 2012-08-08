#ifndef __HDA_LOG_H
#define __HDA_LOG_H

enum {
	HDA_LOG_ERR,
	HDA_LOG_WARN,
	HDA_LOG_KERN,
	HDA_LOG_INFO,
	HDA_LOG_VERB,
};

/* flags */
#define HDA_LOG_FLAG_NO_ECHO	(1 << 0)
#define HDA_LOG_FLAG_COLOR	(1 << 1)
#define HDA_LOG_FLAG_PREFIX	(1 << 2)

int hda_log_init(const char *file, unsigned int flags);
void hda_log(int level, const char *fmt, ...);
void hda_log_printk(const char *fmt, ...);
void hda_log_echo(int level, const char *fmt, ...);
void hda_log_flush(void);

extern int hda_log_trap_on_error;

int hda_log_level_set(int level);
FILE *hda_get_logfp(void);

void hda_log_dump_proc(unsigned int nid, const char *file);
void hda_log_list_jacks(void);
void hda_log_jack_state(int nid);
void hda_log_set_jack(int nid, int val);
void hda_log_issue_unsol(int nid);

void hda_test_suspend(void);
void hda_test_resume(void);
void hda_exec_verb(int nid, int verb, int parm);

void hda_codec_reset(void);
int hda_codec_reconfig(void);

void hda_list_pcms(void);

enum {
	PCM_TEST_ALL,
	PCM_TEST_START,
	PCM_TEST_END,
};

void hda_test_pcm(int stream, int op, int substream,
		  int dir, int rate, int channels, int format);

int hda_get_power_save(void);
void hda_set_power_save(int val);

struct xhda_codec;

unsigned int hda_decode_verb_parm(struct xhda_codec *codec,
				  unsigned int verb, unsigned int parm);
int hda_encode_verb_parm(const char *verb, const char *parm,
			 unsigned int *verb_ret, unsigned int *parm_ret);

#ifdef HAVE_USER_PINCFGS
void hda_log_show_driver_pin_configs(void);
void hda_log_show_init_pin_configs(void);
void hda_log_show_user_pin_configs(void);
void hda_log_set_user_pin_configs(unsigned int nid, unsigned int cfg);
#endif /* HAVE_USER_PINCFGS */

#endif /* __HDA_LOG_H */
