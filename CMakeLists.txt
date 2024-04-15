cmake_minimum_required(VERSION 3.20)

project(fmpire)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(DPF)

dpf_add_plugin(fmpire
	TARGETS jack clap lv2 vst3
	UI_TYPE cairo
	FILES_DSP
		src/dsp/fmpire.cpp
		src/dsp/oscillator.cpp
		src/common/wavetable.cpp
		src/common/wavetable_creator.cpp
	FILES_UI
		src/common/utils.cpp
		src/ui/fmpire_ui.cpp
		src/ui/widgets/arc.cpp
		src/ui/widgets/border.cpp
		src/ui/widgets/double_click.cpp
		src/ui/widgets/draw_operations.cpp
		src/ui/widgets/fmpire_widget.cpp
		src/ui/widgets/grid_container.cpp
		src/ui/widgets/knob.cpp
		src/ui/widgets/theme.cpp
)

target_include_directories(fmpire
	PUBLIC
	src
	src/common/
	src/dsp
	src/ui
	src/ui/widgets/
)
