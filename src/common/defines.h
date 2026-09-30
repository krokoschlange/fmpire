#ifndef DEFINES_H_INCLUDED
#define DEFINES_H_INCLUDED

#define FMPIRE_OSC_COUNT            8
#define FMPIRE_VOICE_COUNT          16
#define FMPIRE_MAX_UNISON_AMOUNT    16
#define FMPIRE_MACRO_COUNT          8
// Output parameters that report live values to the UI, after the macros: the
// modulation of the target of each route slot, then the playhead of each
// modulator id.
#define FMPIRE_METER_COUNT          32
#define FMPIRE_PLAYHEAD_COUNT       16
#define FMPIRE_METER_BASE           FMPIRE_MACRO_COUNT
#define FMPIRE_PLAYHEAD_BASE        (FMPIRE_METER_BASE + FMPIRE_METER_COUNT)
#define FMPIRE_PARAMETER_COUNT      (FMPIRE_PLAYHEAD_BASE + FMPIRE_PLAYHEAD_COUNT)
#define FMPIRE_ID_SPACE             65536


#define OSC_DATA_STRING             std::string("OSCILLATOR")


#define KEY_OSC_PREFIX              "osc/"
#define KEY_MOD_PREFIX              "mod/"

#define KEY_EVERYTHING              "everything"

#define KEY_OSC_WAVETABLE           "wavetable/"
#define KEY_OSC_ACTIVE              "active"
#define KEY_OSC_VOLUME              "volume"
#define KEY_OSC_WT_POS              "wt_pos"
#define KEY_OSC_DETUNE              "detune"
#define KEY_OSC_PAN                 "pan"
#define KEY_OSC_NOTE_SHIFT          "note_shift"
#define KEY_OSC_PHASE_OFFSET        "phase_offset"
#define KEY_OSC_PHASE_RANDOM        "phase_random"
#define KEY_OSC_UNISON_SIZE         "unison_size"
#define KEY_OSC_UNISON_DETUNE       "unison_detune"
#define KEY_OSC_UNISON_SPREAD       "unison_spread"
#define KEY_OSC_UNISON_PHASE_RANDOM "unison_phase_random"
// followed by "<type>/<modulator>": cross modulation depth (type 0-3 = AM/FM/PM/RM)
#define KEY_OSC_MOD_DEPTH           "depth/"

#define MODULATOR_DATA_STRING       std::string("MODULATOR")
#define ROUTE_DATA_STRING           std::string("ROUTE")
#define MATRIX_DATA_STRING          std::string("MATRIX")

// modulators: "mod/<id>/<key>"
#define KEY_MOD_CREATE              "create"
#define KEY_MOD_REMOVE              "remove"
#define KEY_MOD_SETTINGS            "settings"
#define KEY_MOD_CURVE               "curve"

// routes: "route/<slot>/<key>"
#define KEY_ROUTE_PREFIX            "route/"
#define KEY_ROUTE_SET               "set"
#define KEY_ROUTE_AMOUNT            "amount"
#define KEY_ROUTE_REMOVE            "remove"

#define KEY_WT_ALL                  "all"
#define KEY_WT_INSERT               "insert"
#define KEY_WT_UPDATE               "update"
#define KEY_WT_REMOVE               "remove"


#endif // DEFINES_H_INCLUDED
