cmake_minimum_required(VERSION 3.20)

project(fmpire)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(DPF)

dpf_add_plugin(fmpire
	TARGETS jack clap lv2 vst3
	UI_TYPE cairo
	FILES_DSP
		src/dsp/fmpire.cpp
		src/dsp/modulator.cpp
		src/dsp/oscillator.cpp
		src/dsp/voice.cpp
		src/common/FftRealPair.cpp
		src/common/utils.cpp
		src/common/waveform.cpp
		src/common/waveform_part.cpp
		src/common/wavetable.cpp
		src/common/wavetable_creator.cpp
	FILES_UI
		src/common/FftRealPair.cpp
		src/common/utils.cpp
		src/common/waveform.cpp
		src/common/waveform_part.cpp
		src/common/wavetable_creator.cpp
		src/ui/double_click.cpp
		src/ui/fmpire_ui.cpp
		src/ui/state_manager.cpp
		src/ui/widgets/arc.cpp
		src/ui/widgets/aspect_ratio_container.cpp
		src/ui/widgets/border.cpp
		src/ui/widgets/button.cpp
		src/ui/widgets/draw_operations.cpp
		src/ui/widgets/fmpire_widget.cpp
		src/ui/widgets/fmpire_window.cpp
		src/ui/widgets/grid_container.cpp
		src/ui/widgets/image_button.cpp
		src/ui/widgets/int_editor.cpp
		src/ui/widgets/knob.cpp
		src/ui/widgets/label.cpp
		src/ui/widgets/oscillator_bar.cpp
		src/ui/widgets/oscillator_settings.cpp
		src/ui/widgets/relative_container.cpp
		src/ui/widgets/selector.cpp
		src/ui/widgets/theme.cpp
		src/ui/widgets/tooltip.cpp
		src/ui/widgets/waveform_editor.cpp
		src/ui/widgets/waveform_selector.cpp
		src/ui/widgets/wavetable_editor.cpp
		src/ui/widgets/wavetable_view.cpp
)

target_include_directories(fmpire
	PUBLIC
	src
	src/common/
	src/dsp
	src/ui
	src/ui/widgets/
)
