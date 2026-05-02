/*
 * Dynamic Timer — filter implementation.
 * v1.1.0: H/M/S inputs, format presets, grouped UI.
 * Filter-enabled-state is the timer activation trigger (multi-instance pattern).
 */

#include "format-time.h"

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include <util/bmem.h>
#include <util/platform.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MODE_COUNTDOWN          "countdown"
#define MODE_COUNTUP            "countup"
#define MODE_SPECIFIC_TIME      "specific_time"
#define MODE_SPECIFIC_DATETIME  "specific_date_time"
#define MODE_STREAMING          "streaming"
#define MODE_RECORDING          "recording"

/* ---------- format presets ---------- */

#define PRESET_MS        "ms"        /* 05:30 */
#define PRESET_HMS       "hms"       /* 00:05:30 — default */
#define PRESET_H_MS      "h_ms"      /* 0:05:30 (hours unpadded) */
#define PRESET_MS_CENTI  "ms_centi"  /* 05:30.42 */
#define PRESET_DAYS_H    "days_h"    /* 1 T 03:14 */
#define PRESET_TOTAL_S   "total_s"   /* 330 (seconds) */
#define PRESET_TOTAL_M   "total_m"   /* 5 (minutes, floor) */
#define PRESET_TOTAL_H   "total_h"   /* 0 (hours, floor) */

static const char *preset_to_dsl(const char *preset)
{
	if (!preset || !*preset)
		return "%0H:%0m:%0s";
	if (strcmp(preset, PRESET_MS) == 0)        return "%0m:%0s";
	if (strcmp(preset, PRESET_HMS) == 0)       return "%0H:%0m:%0s";
	if (strcmp(preset, PRESET_H_MS) == 0)      return "%H:%0m:%0s";
	if (strcmp(preset, PRESET_MS_CENTI) == 0)  return "%0m:%0s.%2t";
	if (strcmp(preset, PRESET_DAYS_H) == 0)    return "%d T %0h:%0m";
	if (strcmp(preset, PRESET_TOTAL_S) == 0)   return "%S";
	if (strcmp(preset, PRESET_TOTAL_M) == 0)   return "%M";
	if (strcmp(preset, PRESET_TOTAL_H) == 0)   return "%H";
	return "%0H:%0m:%0s";
}

/* For migration: reverse-detect a preset from a stored DSL string. */
static const char *dsl_to_preset(const char *dsl)
{
	if (!dsl || !*dsl)
		return PRESET_HMS;
	if (strcmp(dsl, "%0m:%0s") == 0)        return PRESET_MS;
	if (strcmp(dsl, "%0H:%0m:%0s") == 0)    return PRESET_HMS;
	if (strcmp(dsl, "%H:%0m:%0s") == 0)     return PRESET_H_MS;
	if (strcmp(dsl, "%0m:%0s.%2t") == 0)    return PRESET_MS_CENTI;
	if (strcmp(dsl, "%d T %0h:%0m") == 0)   return PRESET_DAYS_H;
	if (strcmp(dsl, "%S") == 0)             return PRESET_TOTAL_S;
	if (strcmp(dsl, "%M") == 0)             return PRESET_TOTAL_M;
	if (strcmp(dsl, "%H") == 0)             return PRESET_TOTAL_H;
	/* Unknown DSL — fall back to default; user's setting will be silently
	 * upgraded to the canonical HMS preset. v1.0.0 only ever produced one
	 * of these strings, so this branch is rare. */
	return PRESET_HMS;
}

/* ---------- filter struct ---------- */

struct timer_filter {
	obs_source_t *self;

	bool parent_valid;
	obs_source_t *parent_src;

	/* settings (resolved) */
	char *mode;
	char *format;          /* DSL, derived from preset */
	char *stop_text;
	char *next_scene_name;

	int duration_sec;      /* total = h*3600 + m*60 + s */
	int offset_sec;
	int target_year, target_month, target_day, target_hour, target_min, target_sec;

	bool up_when_finished;
	bool switch_to_scene;

	/* runtime */
	bool was_enabled;
	bool running;
	bool counting_up;
	bool finished;

	uint64_t start_ns;
	int64_t  base_ns;

	/* hotkeys */
	obs_hotkey_id hk_toggle;
	obs_hotkey_id hk_reset;

	/* diff check */
	char last_text[256];
};

static bool parent_is_text_source(obs_source_t *parent)
{
	if (!parent)
		return false;
	const char *id = obs_source_get_unversioned_id(parent);
	if (!id)
		return false;
	return strcmp(id, "text_gdiplus") == 0 ||
	       strcmp(id, "text_ft2_source") == 0;
}

/* ---------- helpers ---------- */

static int64_t mode_initial_ns(const struct timer_filter *f)
{
	if (!f->mode)
		return 0;
	if (strcmp(f->mode, MODE_COUNTDOWN) == 0)
		return (int64_t)f->duration_sec * 1000000000LL;
	if (strcmp(f->mode, MODE_COUNTUP) == 0)
		return (int64_t)f->offset_sec * 1000000000LL;
	if (strcmp(f->mode, MODE_SPECIFIC_TIME) == 0) {
		time_t now = time(NULL);
		struct tm t;
#ifdef _WIN32
		localtime_s(&t, &now);
#else
		localtime_r(&now, &t);
#endif
		t.tm_hour = f->target_hour;
		t.tm_min = f->target_min;
		t.tm_sec = f->target_sec;
		t.tm_isdst = -1;
		time_t target = mktime(&t);
		double secs = difftime(target, now);
		if (secs < 0)
			secs += 86400.0;
		return (int64_t)(secs * 1000000000.0);
	}
	if (strcmp(f->mode, MODE_SPECIFIC_DATETIME) == 0) {
		struct tm t = {0};
		t.tm_year = f->target_year - 1900;
		t.tm_mon = f->target_month - 1;
		t.tm_mday = f->target_day;
		t.tm_hour = f->target_hour;
		t.tm_min = f->target_min;
		t.tm_sec = f->target_sec;
		t.tm_isdst = -1;
		time_t target = mktime(&t);
		time_t now = time(NULL);
		double secs = difftime(target, now);
		if (secs < 0)
			secs = 0;
		return (int64_t)(secs * 1000000000.0);
	}
	return 0;
}

static bool mode_is_countdown_like(const struct timer_filter *f)
{
	if (!f->mode)
		return false;
	return strcmp(f->mode, MODE_COUNTDOWN) == 0
		|| strcmp(f->mode, MODE_SPECIFIC_TIME) == 0
		|| strcmp(f->mode, MODE_SPECIFIC_DATETIME) == 0;
}

static bool mode_is_streaming(const struct timer_filter *f)
{
	return f->mode && strcmp(f->mode, MODE_STREAMING) == 0;
}

static bool mode_is_recording(const struct timer_filter *f)
{
	return f->mode && strcmp(f->mode, MODE_RECORDING) == 0;
}

static void write_to_parent(struct timer_filter *f, const char *text)
{
	if (!text)
		return;
	if (!f->parent_valid)
		return;
	if (strcmp(text, f->last_text) == 0)
		return;
	strncpy(f->last_text, text, sizeof(f->last_text) - 1);
	f->last_text[sizeof(f->last_text) - 1] = '\0';

	obs_source_t *parent = obs_filter_get_parent(f->self);
	if (!parent)
		return;

	obs_data_t *d = obs_data_create();
	obs_data_set_string(d, "text", text);
	obs_source_update(parent, d);
	obs_data_release(d);
}

static void switch_to_named_scene(const char *name)
{
	if (!name || !*name)
		return;
	struct obs_frontend_source_list scenes = {0};
	obs_frontend_get_scenes(&scenes);
	for (size_t i = 0; i < scenes.sources.num; i++) {
		obs_source_t *s = scenes.sources.array[i];
		const char *sn = obs_source_get_name(s);
		if (sn && strcmp(sn, name) == 0) {
			obs_frontend_set_current_scene(s);
			break;
		}
	}
	obs_frontend_source_list_free(&scenes);
}

/* ---------- lifecycle ---------- */

static void start_running(struct timer_filter *f)
{
	f->running = true;
	f->finished = false;
	f->counting_up = false;
	f->base_ns = mode_initial_ns(f);
	f->start_ns = obs_get_video_frame_time();
}

static void stop_running(struct timer_filter *f)
{
	f->running = false;
}

/* ---------- hotkey callbacks ---------- */

static void hk_toggle_cb(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed)
{
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(key);
	if (!pressed)
		return;
	struct timer_filter *f = data;
	bool en = obs_source_enabled(f->self);
	obs_source_set_enabled(f->self, !en);
}

static void hk_reset_cb(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed)
{
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(key);
	if (!pressed)
		return;
	struct timer_filter *f = data;
	if (obs_source_enabled(f->self)) {
		start_running(f);
		f->last_text[0] = '\0';
	} else {
		f->running = false;
		f->finished = false;
		f->counting_up = false;
		f->last_text[0] = '\0';
	}
}

/* ---------- frontend events ---------- */

static void frontend_event_cb(enum obs_frontend_event event, void *data)
{
	struct timer_filter *f = data;
	if (mode_is_streaming(f)) {
		if (event == OBS_FRONTEND_EVENT_STREAMING_STARTED && obs_source_enabled(f->self)) {
			start_running(f);
		} else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPED) {
			stop_running(f);
		}
	} else if (mode_is_recording(f)) {
		if (event == OBS_FRONTEND_EVENT_RECORDING_STARTED && obs_source_enabled(f->self)) {
			start_running(f);
		} else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPED) {
			stop_running(f);
		}
	}
}

/* ---------- OBS API callbacks ---------- */

static const char *timer_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("FilterName");
}

/* Read H/M/S from settings; if all zero AND legacy_key is set, derive from it.
 * Returns total seconds. Writes back the new fields if migration happened. */
static int read_hms_with_migration(obs_data_t *settings,
	const char *key_h, const char *key_m, const char *key_s,
	const char *legacy_key)
{
	int h = (int)obs_data_get_int(settings, key_h);
	int m = (int)obs_data_get_int(settings, key_m);
	int s = (int)obs_data_get_int(settings, key_s);
	int total = h * 3600 + m * 60 + s;

	if (total == 0 && legacy_key) {
		int legacy = (int)obs_data_get_int(settings, legacy_key);
		if (legacy > 0) {
			h = legacy / 3600;
			m = (legacy / 60) % 60;
			s = legacy % 60;
			total = legacy;
			obs_data_set_int(settings, key_h, h);
			obs_data_set_int(settings, key_m, m);
			obs_data_set_int(settings, key_s, s);
		}
	}
	return total;
}

static void timer_update(void *data, obs_data_t *settings)
{
	struct timer_filter *f = data;

	bfree(f->mode);
	bfree(f->format);
	bfree(f->stop_text);
	bfree(f->next_scene_name);

	f->mode = bstrdup(obs_data_get_string(settings, "mode"));
	if (!f->mode || !*f->mode) {
		bfree(f->mode);
		f->mode = bstrdup(MODE_COUNTDOWN);
	}

	/* Format preset → DSL.
	 * Migration: if preset is empty and legacy "format" exists, reverse-detect. */
	const char *preset = obs_data_get_string(settings, "format_preset");
	if (!preset || !*preset) {
		const char *legacy_fmt = obs_data_get_string(settings, "format");
		preset = dsl_to_preset(legacy_fmt);
		obs_data_set_string(settings, "format_preset", preset);
	}
	f->format = bstrdup(preset_to_dsl(preset));

	f->stop_text = bstrdup(obs_data_get_string(settings, "stop_text"));
	f->next_scene_name = bstrdup(obs_data_get_string(settings, "next_scene"));

	/* Time fields with legacy-fallback migration. */
	f->duration_sec = read_hms_with_migration(settings,
		"duration_h", "duration_m", "duration_s", "duration");
	f->offset_sec = read_hms_with_migration(settings,
		"offset_h", "offset_m", "offset_s", "offset");

	f->target_year = (int)obs_data_get_int(settings, "year");
	f->target_month = (int)obs_data_get_int(settings, "month");
	f->target_day = (int)obs_data_get_int(settings, "day");
	f->target_hour = (int)obs_data_get_int(settings, "hour");
	f->target_min = (int)obs_data_get_int(settings, "minutes");
	f->target_sec = (int)obs_data_get_int(settings, "seconds");

	f->up_when_finished = obs_data_get_bool(settings, "countup_countdown_finished");
	f->switch_to_scene = obs_data_get_bool(settings, "switch_to_scene");

	f->finished = false;
	f->counting_up = false;
	f->last_text[0] = '\0';

	if (obs_source_enabled(f->self)) {
		start_running(f);
	} else {
		f->running = false;
	}

	char buf[256];
	format_time_ns(mode_initial_ns(f), f->format, buf, sizeof(buf));
	if (*buf)
		write_to_parent(f, buf);
}

static void *timer_create(obs_data_t *settings, obs_source_t *source)
{
	struct timer_filter *f = bzalloc(sizeof(struct timer_filter));
	f->self = source;
	f->mode = bstrdup(MODE_COUNTDOWN);

	f->hk_toggle = obs_hotkey_register_source(source,
		"dynamic_timer.toggle", obs_module_text("HotkeyToggle"),
		hk_toggle_cb, f);
	f->hk_reset = obs_hotkey_register_source(source,
		"dynamic_timer.reset", obs_module_text("HotkeyReset"),
		hk_reset_cb, f);

	obs_frontend_add_event_callback(frontend_event_cb, f);

	timer_update(f, settings);
	return f;
}

static void timer_destroy(void *data)
{
	struct timer_filter *f = data;
	if (!f)
		return;
	obs_frontend_remove_event_callback(frontend_event_cb, f);
	obs_hotkey_unregister(f->hk_toggle);
	obs_hotkey_unregister(f->hk_reset);
	bfree(f->mode);
	bfree(f->format);
	bfree(f->stop_text);
	bfree(f->next_scene_name);
	bfree(f);
}

static void timer_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, "mode", MODE_COUNTDOWN);
	obs_data_set_default_string(settings, "format_preset", PRESET_HMS);
	obs_data_set_default_string(settings, "stop_text", "Starting soon (tm)");

	/* New H/M/S fields. Default countdown = 5 minutes. */
	obs_data_set_default_int(settings, "duration_h", 0);
	obs_data_set_default_int(settings, "duration_m", 5);
	obs_data_set_default_int(settings, "duration_s", 0);
	obs_data_set_default_int(settings, "offset_h", 0);
	obs_data_set_default_int(settings, "offset_m", 0);
	obs_data_set_default_int(settings, "offset_s", 0);

	time_t now = time(NULL);
	struct tm t;
#ifdef _WIN32
	localtime_s(&t, &now);
#else
	localtime_r(&now, &t);
#endif
	obs_data_set_default_int(settings, "year", t.tm_year + 1900);
	obs_data_set_default_int(settings, "month", t.tm_mon + 1);
	obs_data_set_default_int(settings, "day", t.tm_mday);
	obs_data_set_default_int(settings, "hour", 0);
	obs_data_set_default_int(settings, "minutes", 0);
	obs_data_set_default_int(settings, "seconds", 0);

	obs_data_set_default_bool(settings, "countup_countdown_finished", false);
	obs_data_set_default_bool(settings, "switch_to_scene", false);
	obs_data_set_default_string(settings, "next_scene", "");
}

/* ---------- properties ---------- */

static bool mode_modified(obs_properties_t *props, obs_property_t *prop, obs_data_t *settings)
{
	UNUSED_PARAMETER(prop);
	const char *m = obs_data_get_string(settings, "mode");
	if (!m)
		m = MODE_COUNTDOWN;

	bool is_countdown = strcmp(m, MODE_COUNTDOWN) == 0;
	bool is_countup = strcmp(m, MODE_COUNTUP) == 0;
	bool is_specific_time = strcmp(m, MODE_SPECIFIC_TIME) == 0;
	bool is_specific_dt = strcmp(m, MODE_SPECIFIC_DATETIME) == 0;

	bool show_duration_hms = is_countdown;
	bool show_offset_hms = is_countup;
	bool show_date = is_specific_dt;
	bool show_clock = is_specific_time || is_specific_dt;
	bool show_stop = is_countdown || is_specific_time || is_specific_dt;
	bool show_finish_actions = show_stop;

	obs_property_set_visible(obs_properties_get(props, "duration_h"), show_duration_hms);
	obs_property_set_visible(obs_properties_get(props, "duration_m"), show_duration_hms);
	obs_property_set_visible(obs_properties_get(props, "duration_s"), show_duration_hms);
	obs_property_set_visible(obs_properties_get(props, "offset_h"), show_offset_hms);
	obs_property_set_visible(obs_properties_get(props, "offset_m"), show_offset_hms);
	obs_property_set_visible(obs_properties_get(props, "offset_s"), show_offset_hms);
	obs_property_set_visible(obs_properties_get(props, "year"), show_date);
	obs_property_set_visible(obs_properties_get(props, "month"), show_date);
	obs_property_set_visible(obs_properties_get(props, "day"), show_date);
	obs_property_set_visible(obs_properties_get(props, "hour"), show_clock);
	obs_property_set_visible(obs_properties_get(props, "minutes"), show_clock);
	obs_property_set_visible(obs_properties_get(props, "seconds"), show_clock);
	obs_property_set_visible(obs_properties_get(props, "stop_text"), show_stop);
	obs_property_set_visible(obs_properties_get(props, "countup_countdown_finished"), show_finish_actions);
	obs_property_set_visible(obs_properties_get(props, "switch_to_scene"), show_finish_actions);

	bool switch_on = obs_data_get_bool(settings, "switch_to_scene");
	obs_property_set_visible(obs_properties_get(props, "next_scene"), show_finish_actions && switch_on);

	return true;
}

static bool switch_modified(obs_properties_t *props, obs_property_t *prop, obs_data_t *settings)
{
	UNUSED_PARAMETER(prop);
	bool on = obs_data_get_bool(settings, "switch_to_scene");
	const char *m = obs_data_get_string(settings, "mode");
	bool show_finish = m && (strcmp(m, MODE_COUNTDOWN) == 0
		|| strcmp(m, MODE_SPECIFIC_TIME) == 0
		|| strcmp(m, MODE_SPECIFIC_DATETIME) == 0);
	obs_property_set_visible(obs_properties_get(props, "next_scene"), on && show_finish);
	return true;
}

static void timer_filter_add(void *data, obs_source_t *parent)
{
	struct timer_filter *f = data;
	f->parent_src = parent;
	f->parent_valid = parent_is_text_source(parent);
}

static void timer_filter_remove(void *data, obs_source_t *parent)
{
	UNUSED_PARAMETER(parent);
	struct timer_filter *f = data;
	f->parent_valid = false;
	f->parent_src = NULL;
}

static obs_properties_t *timer_properties(void *data)
{
	struct timer_filter *f = data;
	obs_properties_t *props = obs_properties_create();

	if (!f || !f->parent_valid) {
		obs_property_t *warn = obs_properties_add_text(props,
			"warning", obs_module_text("WarningTitle"),
			OBS_TEXT_INFO);
		obs_property_text_set_info_type(warn, OBS_TEXT_INFO_WARNING);
		obs_properties_add_text(props,
			"warning_help", obs_module_text("WarningHelp"),
			OBS_TEXT_INFO);
		return props;
	}

	/* ===== Group: Modus ===== */
	{
		obs_properties_t *g = obs_properties_create();
		obs_property_t *p_mode = obs_properties_add_list(g, "mode",
			obs_module_text("Mode"),
			OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
		obs_property_list_add_string(p_mode, obs_module_text("ModeCountdown"), MODE_COUNTDOWN);
		obs_property_list_add_string(p_mode, obs_module_text("ModeCountup"), MODE_COUNTUP);
		obs_property_list_add_string(p_mode, obs_module_text("ModeSpecificTime"), MODE_SPECIFIC_TIME);
		obs_property_list_add_string(p_mode, obs_module_text("ModeSpecificDateTime"), MODE_SPECIFIC_DATETIME);
		obs_property_list_add_string(p_mode, obs_module_text("ModeStreaming"), MODE_STREAMING);
		obs_property_list_add_string(p_mode, obs_module_text("ModeRecording"), MODE_RECORDING);
		obs_property_set_modified_callback(p_mode, mode_modified);
		obs_properties_add_group(props, "g_mode", obs_module_text("GroupMode"), OBS_GROUP_NORMAL, g);
	}

	/* ===== Group: Zeit ===== */
	{
		obs_properties_t *g = obs_properties_create();

		/* Countdown duration (visible only in Countdown) */
		obs_properties_add_int(g, "duration_h", obs_module_text("DurationH"), 0, 999, 1);
		obs_properties_add_int(g, "duration_m", obs_module_text("DurationM"), 0, 59, 1);
		obs_properties_add_int(g, "duration_s", obs_module_text("DurationS"), 0, 59, 1);

		/* Countup offset */
		obs_properties_add_int(g, "offset_h", obs_module_text("OffsetH"), 0, 999, 1);
		obs_properties_add_int(g, "offset_m", obs_module_text("OffsetM"), 0, 59, 1);
		obs_properties_add_int(g, "offset_s", obs_module_text("OffsetS"), 0, 59, 1);

		/* Specific date components */
		obs_properties_add_int(g, "year", obs_module_text("Year"), 1971, 9999, 1);
		obs_properties_add_int(g, "month", obs_module_text("Month"), 1, 12, 1);
		obs_properties_add_int(g, "day", obs_module_text("Day"), 1, 31, 1);

		/* Specific time-of-day (used by Specific time and Specific date+time) */
		obs_properties_add_int(g, "hour", obs_module_text("Hour"), 0, 23, 1);
		obs_properties_add_int(g, "minutes", obs_module_text("Minutes"), 0, 59, 1);
		obs_properties_add_int(g, "seconds", obs_module_text("Seconds"), 0, 59, 1);

		obs_properties_add_group(props, "g_time", obs_module_text("GroupTime"), OBS_GROUP_NORMAL, g);
	}

	/* ===== Group: Anzeige ===== */
	{
		obs_properties_t *g = obs_properties_create();
		obs_property_t *p_preset = obs_properties_add_list(g, "format_preset",
			obs_module_text("FormatPreset"),
			OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
		obs_property_list_add_string(p_preset, obs_module_text("PresetMS"), PRESET_MS);
		obs_property_list_add_string(p_preset, obs_module_text("PresetHMS"), PRESET_HMS);
		obs_property_list_add_string(p_preset, obs_module_text("PresetH_MS"), PRESET_H_MS);
		obs_property_list_add_string(p_preset, obs_module_text("PresetMSCenti"), PRESET_MS_CENTI);
		obs_property_list_add_string(p_preset, obs_module_text("PresetDaysH"), PRESET_DAYS_H);
		obs_property_list_add_string(p_preset, obs_module_text("PresetTotalH"), PRESET_TOTAL_H);
		obs_property_list_add_string(p_preset, obs_module_text("PresetTotalM"), PRESET_TOTAL_M);
		obs_property_list_add_string(p_preset, obs_module_text("PresetTotalS"), PRESET_TOTAL_S);

		obs_properties_add_text(g, "stop_text", obs_module_text("StopText"), OBS_TEXT_DEFAULT);

		obs_properties_add_group(props, "g_display", obs_module_text("GroupDisplay"), OBS_GROUP_NORMAL, g);
	}

	/* ===== Group: Aktion am Ende ===== */
	{
		obs_properties_t *g = obs_properties_create();
		obs_properties_add_bool(g, "countup_countdown_finished", obs_module_text("UpWhenFinished"));
		obs_property_t *p_switch = obs_properties_add_bool(g, "switch_to_scene", obs_module_text("SwitchToScene"));
		obs_property_set_modified_callback(p_switch, switch_modified);

		obs_property_t *p_scene = obs_properties_add_list(g, "next_scene",
			obs_module_text("NextScene"),
			OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
		struct obs_frontend_source_list scenes = {0};
		obs_frontend_get_scenes(&scenes);
		for (size_t i = 0; i < scenes.sources.num; i++) {
			const char *name = obs_source_get_name(scenes.sources.array[i]);
			if (name)
				obs_property_list_add_string(p_scene, name, name);
		}
		obs_frontend_source_list_free(&scenes);

		obs_properties_add_group(props, "g_action", obs_module_text("GroupAction"), OBS_GROUP_NORMAL, g);
	}

	return props;
}

/* ---------- tick + render ---------- */

static void timer_video_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(seconds);
	struct timer_filter *f = data;
	bool is_enabled = obs_source_enabled(f->self);

	if (is_enabled != f->was_enabled) {
		if (is_enabled) {
			if (mode_is_streaming(f)) {
				if (obs_frontend_streaming_active())
					start_running(f);
			} else if (mode_is_recording(f)) {
				if (obs_frontend_recording_active())
					start_running(f);
			} else {
				start_running(f);
			}
		} else {
			stop_running(f);
		}
		f->was_enabled = is_enabled;
	}

	if (!is_enabled || !f->running)
		return;

	if (mode_is_streaming(f) && !obs_frontend_streaming_active())
		return;
	if (mode_is_recording(f) && !obs_frontend_recording_active())
		return;

	uint64_t now_ns = obs_get_video_frame_time();
	int64_t elapsed = (int64_t)(now_ns - f->start_ns);

	int64_t cur_ns;
	bool counting_up_now = mode_is_streaming(f) || mode_is_recording(f)
		|| strcmp(f->mode, MODE_COUNTUP) == 0
		|| f->counting_up;

	if (counting_up_now) {
		cur_ns = f->base_ns + elapsed;
	} else {
		cur_ns = f->base_ns - elapsed;
	}

	if (mode_is_countdown_like(f) && cur_ns <= 0 && !f->counting_up) {
		if (f->up_when_finished) {
			f->counting_up = true;
			f->base_ns = 0;
			f->start_ns = now_ns;
			cur_ns = 0;
		} else if (f->switch_to_scene) {
			switch_to_named_scene(f->next_scene_name);
			f->running = false;
			f->finished = true;
			return;
		} else {
			write_to_parent(f, f->stop_text ? f->stop_text : "");
			f->running = false;
			f->finished = true;
			return;
		}
	}

	char buf[256];
	format_time_ns(cur_ns, f->format ? f->format : "%0H:%0m:%0s", buf, sizeof(buf));
	write_to_parent(f, buf);
}

static void timer_video_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	struct timer_filter *f = data;
	obs_source_skip_video_filter(f->self);
}

struct obs_source_info dynamic_timer_filter_info = {
	.id             = "dynamic_timer_filter",
	.type           = OBS_SOURCE_TYPE_FILTER,
	.output_flags   = OBS_SOURCE_VIDEO,
	.get_name       = timer_get_name,
	.create         = timer_create,
	.destroy        = timer_destroy,
	.update         = timer_update,
	.get_defaults   = timer_defaults,
	.get_properties = timer_properties,
	.filter_add     = timer_filter_add,
	.filter_remove  = timer_filter_remove,
	.video_tick     = timer_video_tick,
	.video_render   = timer_video_render,
};
