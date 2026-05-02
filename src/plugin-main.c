/*
Dynamic Timer — OBS filter for advanced text-source timers (Countdown,
Countup, Specific time, Specific date, Streaming timer, Recording timer).
Copyright (C) 2026 K_STYER, GPLv2 or later.
*/

#include <obs-module.h>
#include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

MODULE_EXPORT const char *obs_module_description(void)
{
	return "Filter that turns text sources into highly configurable timers (countdown/up, specific time, streaming/recording duration).";
}

MODULE_EXPORT const char *obs_module_name(void)
{
	return "Dynamic Timer";
}

MODULE_EXPORT const char *obs_module_author(void)
{
	return "K_STYER";
}

extern struct obs_source_info dynamic_timer_filter_info;

bool obs_module_load(void)
{
	obs_register_source(&dynamic_timer_filter_info);
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}
