/* See LICENSE file for copyright and license details. */



#define BATTERY_NAME "BAT0"

#define ETHERNET_DEVICE "eth0"
#define WIFI_DEVICE "wlan0"


const char *battery_perc(const char *bat);
//const char *volume_perc(void);
const char *netspeed_rx_auto(void);



/* interval between updates (in ms) */
const unsigned int interval = 1000;

/* text to show if no value can be retrieved */
static const char unknown_str[] = "><";

/* maximum output string length */
#define MAXLEN 2048

/*
 * function            description                     argument (example)
 *
 * battery_perc        battery percentage              battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * cpu_perc            cpu usage in percent            NULL
 * cpu_freq            cpu frequency in MHz            NULL
 * datetime            date and time                   format string (%F %T)
 * disk_free           free disk space in GB           mountpoint path (/)
 * disk_perc           disk usage in percent           mountpoint path (/)
 * disk_total          total disk space in GB          mountpoint path (/")
 * disk_used           used disk space in GB           mountpoint path (/)
 * entropy             available entropy               NULL
 * gid                 GID of current user             NULL
 * hostname            hostname                        NULL
 * ipv4                IPv4 address                    interface name (eth0)
 * ipv6                IPv6 address                    interface name (eth0)
 * kernel_release      `uname -r`                      NULL
 * keyboard_indicators caps/num lock indicators        format string (c?n?)
 *                                                     see keyboard_indicators.c
 * keymap              layout (variant) of current     NULL
 *                     keymap
 * load_avg            load average                    NULL
 * netspeed_rx         receive network speed           interface name (wlan0)
 * netspeed_tx         transfer network speed          interface name (wlan0)
 * num_files           number of files in a directory  path
 *                                                     (/home/foo/Inbox/cur)
 * ram_free            free memory in GB               NULL
 * ram_perc            memory usage in percent         NULL
 * ram_total           total memory size in GB         NULL
 * ram_used            used memory in GB               NULL
 * run_command         custom shell command            command (echo foo)
 * separator           string to echo                  NULL
 * swap_free           free swap in GB                 NULL
 * swap_perc           swap usage in percent           NULL
 * swap_total          total swap size in GB           NULL
 * swap_used           used swap in GB                 NULL
 * temp                temperature in degree celsius   sensor file
 *                                                     (/sys/class/thermal/...)
 *                                                     NULL on OpenBSD
 *                                                     thermal zone on FreeBSD
 *                                                     (tz0, tz1, etc.)
 * uid                 UID of current user             NULL
 * uptime              system uptime                   NULL
 * username            username of current user        NULL
 * vol_perc            OSS/ALSA volume in percent      mixer file (/dev/mixer)
 *                                                     NULL on OpenBSD
 * wifi_perc           WiFi signal in percent          interface name (wlan0)
 * wifi_essid          WiFi ESSID                      interface name (wlan0)
 */
static const struct arg args[] = {
	/* function          format              argument */
// { vol_perc,          "%7s%%  ",          "/dev/snd/controlC1",                 /*   "#EBCB8B",   "#2E3440"*/ },
	{ netspeed_rx_auto,  " %s  ",            NULL,                 /*   "#EBCB8B",   "#2E3440"*/ },
   { cpu_perc,          "%7s%%  ",          NULL,                 /*   "#EBCB8B",   "#2E3440"*/ },
   { ram_used,          "%11sMB  ",         NULL,                 /*   "#BF616A",   "#D8DEE9"*/ },
   { battery_perc,      "%7s%%  ",          NULL,                 /*   "#3B4252",   "#D8DEE9"*/ },
   { datetime,          "%s ",              "%a %e.%m.%g  %T",    /*   "#8FBCBB",   "#2E3440"*/ },
};










#include <stdbool.h>






const char *battery_perc(const char *bat){
	static char bat_str[8];
	FILE *status_file = fopen("/sys/class/power_supply/"BATTERY_NAME"/status", "r");
	FILE *capacity_file = fopen("/sys/class/power_supply/"BATTERY_NAME"/capacity", "r");
	memcpy(bat_str, getc(status_file)=='D' ? "bat:   " : "sup:   ", 8);
	fgets(bat_str+4, 4, capacity_file);
	if (bat_str[5] == '\n'){
		bat_str[6] = bat_str[4];
		bat_str[5] = ' ';
		bat_str[4] = ' ';
	} else if (bat_str[6] == '\n'){
		bat_str[6] = bat_str[5];
		bat_str[5] = bat_str[4];
		bat_str[4] = ' ';
	}
	
	fclose(status_file);
	fclose(capacity_file);
	return bat_str;
}





typedef struct{
	const char *digits;
	size_t len;
} UnsToStringRes;

static UnsToStringRes uns_to_string(size_t val){
	static char digits[32];

	char *it = digits + LEN(digits) - 1;
	size_t len = 0;
	
	*it = '\0';
	
	for (;;){
		--it;
		++len;
		*it = '0' + val % 10;
		val /= 10;
		if (val == 0) break;
	}
	return (UnsToStringRes){it, len};
}

static void insert_character(char *arr, size_t len, size_t pos, char c){
	char *it = arr + len;
	while (--it != arr+pos) *it = *(it-1);
	*it = c;
}

const char *netspeed_rx_auto(void){
	static size_t rxbytes;
	size_t oldrxbytes;
	extern const unsigned int interval;

	oldrxbytes = rxbytes;
	
	static char buf[64];
	char state;
	
	pscanf("/sys/class/net/"WIFI_DEVICE"/operstate", "%c", &state);
	if (state == 'u'){
		pscanf("/sys/class/net/"WIFI_DEVICE"/statistics/rx_bytes", "%ju", &rxbytes);
		strcpy(buf, "wif:");
	} else{
		pscanf("/sys/class/net/"ETHERNET_DEVICE"/operstate", "%c", &state);
		if (state == 'u'){
			pscanf("/sys/class/net/"ETHERNET_DEVICE"/statistics/rx_bytes", "%ju", &rxbytes);
			strcpy(buf, "eth:");
		} else{
			return "no internet  ";
		}
	}

	UnsToStringRes digits = uns_to_string((rxbytes - oldrxbytes) * 1000 / interval);

	if (digits.len > 3){
		memcpy(buf+4, digits.digits, 4);
		insert_character(buf+4+1, 4, (digits.len-1) % 3, '.');
		for (char *it=buf+4+4;; --it){
			if (*it != '0' || *(it-1) == '.') break;
			*it = ' ';
		}
		if (digits.len <= 6){
			buf[4+5] = 'k';
		} else if (digits.len <= 9){
			buf[4+5] = 'M';
		} else{
			strcpy(buf+4, "  MAX ");
		}
	} else{
		strcpy(buf+4, "      ");
		memcpy(buf+4+(5-digits.len), digits.digits, digits.len);
	}
	buf[4+6] = 'B';
	buf[4+7] = '/';
	buf[4+8] = 's';
	buf[4+9] = '\0';
	return buf;
}





/*
#include <alsa/asoundlib.h>

const char *volume_perc(void){
	long min, max;
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name = "Master";

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, card);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	long volume;
	snd_mixer_selem_get_playback_volume(elem, 0, &volume);

	snd_mixer_close(handle);
	
	static char buf[32];
	snprintf(buf, sizeof(buf), "%li", volume * max / 100);
	return buf;
}

void SetAlsaMasterVolume(long volume){
	long min, max;
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name = "Master";

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, card);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

	snd_mixer_close(handle);
}*/
