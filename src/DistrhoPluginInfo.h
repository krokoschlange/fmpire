#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_NAME                "FMpire"

#define DISTRHO_PLUGIN_NUM_INPUTS          0
#define DISTRHO_PLUGIN_NUM_OUTPUTS         2

#define DISTRHO_PLUGIN_URI                 "http://github.com/krokoschlange/fmpire"

#define DISTRHO_PLUGIN_HAS_UI              1
#define DISTRHO_PLUGIN_IS_RT_SAFE          0
#define DISTRHO_PLUGIN_IS_SYNTH            1
#define DISTRHO_PLUGIN_MINIMUM_BUFFER_SIZE 2048
#define DISTRHO_PLUGIN_WANT_DIRECT_ACCESS  0

#define DISTRHO_PLUGIN_WANT_PROGRAMS       0
#define DISTRHO_PLUGIN_WANT_STATE          1
#define DISTRHO_PLUGIN_WANT_FULL_STATE     1
#define DISTRHO_PLUGIN_WANT_TIMEPOS        1


#define DISTRHO_UI_USER_RESIZABLE          1
//#define DISTRHO_UI_DEFAULT_WIDTH           960
//#define DISTRHO_UI_DEFAULT_HEIGHT          540

#define DISTRHO_PLUGIN_BRAND_ID            Krks
#define DISTRHO_PLUGIN_UNIQUE_ID           Kfmp

#define DISTRHO_PLUGIN_LV2_CATEGORY        "lv2:InstrumentPlugin"
#define DISTRHO_PLUGIN_VST3_CATEGORIES     "Instrument|Synth"
#define DISTRHO_PLUGIN_CLAP_FEATURES       "instrument", "synthesizer", "stereo"

#define DISTRHO_PLUGIN_CLAP_ID             "krokoschlange.fmpire"


#define DPF_VST3_DONT_USE_BRAND_ID

#endif // DISTRHO_PLUGIN_INFO_H_INCLUDED
