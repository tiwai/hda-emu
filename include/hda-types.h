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

#define HDA_MAX_NUM_CONNS		32

struct xhda_amp_vals {
	unsigned char vals[HDA_MAX_NUM_CONNS][2];
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
	unsigned int orig_pinctl;
	unsigned int pincap;
	unsigned int pin_default;
	struct xhda_pcm_info pcm;
	struct xhda_amp_caps amp_in_caps;
	struct xhda_amp_caps amp_out_caps;
	struct xhda_amp_vals amp_in_vals;
	struct xhda_amp_vals amp_out_vals;
	struct xhda_amp_vals orig_amp_in_vals;
	struct xhda_amp_vals orig_amp_out_vals;
	unsigned int num_nodes;
	unsigned int curr_conn;
	unsigned int node[HDA_MAX_NUM_CONNS];
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
	unsigned char cvt_channel_count;
	unsigned char asp_channel_slot[16];
	unsigned char dip_xmitctrl;
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


struct xhda_sysfs_value {
	int val[3];
	struct xhda_sysfs_value *next;
};

struct xhda_sysfs_hints {
	char *line;
	struct xhda_sysfs_hints *next;
};

enum { XHDA_SYS_PINCFG, XHDA_SYS_VERBS, XHDA_SYS_HINTS };

struct xhda_sysfs_list {
	char *id;
	struct xhda_sysfs_list *next;
	int type;
	union {
		struct xhda_sysfs_value *vals;
		struct xhda_sysfs_hints *hints;
	} entry;
};

struct xhda_codec {
	char *parsed_name;
	unsigned int addr;
	unsigned int function_id;
	unsigned int vendor_id;
	unsigned int subsystem_id;
	unsigned int revision_id;
	unsigned int num_widgets;
	struct xhda_node afg;
	struct xhda_node mfg;
	unsigned int modem_function_id;
	unsigned int rc;
	struct xhda_verb_table *extended_verbs;
	struct xhda_verb_table *extended_parameters;
	int (*extended_cmd) (struct xhda_codec *codec, unsigned int cmd);
	struct xhda_value_cache *value_cache;
	unsigned short pci_vendor, pci_device;
	unsigned short pci_subvendor, pci_subdevice;
	unsigned short pci_revision;
	/* flags */
	unsigned int pin_amp_workaround:1; /* pin out-amp can take index */
	/* sysfs lists */
	struct xhda_sysfs_list *sysfs_list;
};

int parse_codec_proc(FILE *fp, struct xhda_codec *codecp, int idx);
void add_codec_extensions(struct xhda_codec *codec);
void hda_set_proc_coef(struct xhda_codec *codec, int nid, int idx, int val);

int hda_cmd(struct xhda_codec *codec, unsigned int cmd);
int hda_get_jack_state(struct xhda_codec *codec, int nid);
int hda_set_jack_state(struct xhda_codec *codec, int nid, int val);
int hda_get_unsol_state(struct xhda_codec *codec, int nid);

const char *get_verb_name(struct xhda_codec *codec, unsigned int cmd);
const char *get_parameter_name(struct xhda_codec *codec, unsigned int cmd);

#define MAX_ROUTE_DEPTH		10

struct xhda_route_list {
	int num_nodes;
	struct xhda_node *node[MAX_ROUTE_DEPTH + 1];
	struct xhda_route_list *next;
};

struct xhda_route_list *
hda_routes_connected_to(struct xhda_codec *codec, int nid, unsigned int flags);
struct xhda_route_list *
hda_routes_connected_from(struct xhda_codec *codec, int nid, unsigned int flags);
void hda_free_route_lists(struct xhda_route_list *list);

#define SHOW_DIR_IN	(1 << 0)
#define SHOW_DIR_OUT	(1 << 1)
#define SHOW_INACTIVE	(1 << 2)
#define SHOW_ALL	(1 << 3)
#define SHOW_MUTE	(1 << 4)

void hda_show_routes(int nid, unsigned int flags);

void hda_verify_pin_ctl(struct xhda_node *node, int log,
		        unsigned int *sanified_pinctl);


void *xalloc(size_t size);

#endif /* __HDA_TYPES_H */
