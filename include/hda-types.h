#ifndef __HDA_TYPES_H
#define __HDA_TYPES_H

#include <stdio.h>

struct xhda_amp_caps {
	int ofs;
	int nsteps;
	int stepsize;
	int mute;
	int override;
};

#define HDA_MAX_CONNECTIONS		32

struct xhda_amp_vals {
	unsigned char vals[HDA_MAX_CONNECTIONS][2];
};

struct xhda_pcm_info {
	unsigned int rates;
	unsigned int bits;
	unsigned int formats;
};

struct xhda_coef_table {
	unsigned short idx;
	unsigned short value;
	struct xhda_coef_table *next;
};

struct xhda_node {
	unsigned int nid;
	unsigned int wcaps;
	unsigned int pinctl;
	unsigned int pincap;
	unsigned int pin_default;
	struct xhda_pcm_info pcm;
	struct xhda_amp_caps amp_in_caps;
	struct xhda_amp_caps amp_out_caps;
	struct xhda_amp_vals amp_in_vals;
	struct xhda_amp_vals amp_out_vals;
	unsigned int num_nodes;
	unsigned int curr_conn;
	unsigned int node[HDA_MAX_CONNECTIONS];
	int power_setting, power_current;
	int coef_benign, coef_num;
	unsigned int dig_conv, dig_category;
	unsigned int stream_format;
	unsigned char sdi;
	unsigned char unsol, streamid, eapd, volknob_ctl;
	unsigned char proc_state, jack_state, beep_div;
	unsigned int gpio_cap;
	unsigned char gpio_data, gpio_dir, gpio_mask;
	unsigned char gpio_wake, gpio_unsol, gpio_sticky;
	unsigned int coef_idx;
	struct xhda_coef_table *coef_tbl;
	struct xhda_node *next;
};

struct xhda_codec;

struct xhda_verb_table {
	unsigned int verb;
	int (*func)(struct xhda_codec *codec, struct xhda_node *node,
		    unsigned int cmd);
	const char *name;
};

struct xhda_value_cache {
	unsigned int verb;
	unsigned int val;
	struct xhda_value_cache *next;
};

struct xhda_codec {
	unsigned int addr;
	unsigned int vendor_id;
	unsigned int subsystem_id;
	unsigned int revision_id;
	unsigned int num_widgets;
	struct xhda_node afg;
	unsigned int rc;
	struct xhda_verb_table *extended_verbs;
	struct xhda_verb_table *extended_parameters;
	struct xhda_value_cache *value_cache;
};

int parse_codec_proc(FILE *fp, struct xhda_codec *codecp, int idx);
void add_codec_extensions(struct xhda_codec *codec);

int hda_cmd(struct xhda_codec *codec, unsigned int cmd);
int hda_get_jack_state(struct xhda_codec *codec, int nid);
int hda_set_jack_state(struct xhda_codec *codec, int nid, int val);
int hda_get_unsol_state(struct xhda_codec *codec, int nid);

const char *get_verb_name(struct xhda_codec *codec, unsigned int cmd);
const char *get_parameter_name(struct xhda_codec *codec, unsigned int cmd);

#endif /* __HDA_TYPES_H */
