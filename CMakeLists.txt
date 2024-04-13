cmake_minimum_required(VERSION 3.20)

project(fmpire)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(DPF)

dpf_add_plugin(fmpire
	TARGETS jack clap lv2 vst3
	UI_TYPE cairo
	FILES_DSP
		src/dsp/fmpire.cpp
	FILES_UI
		src/common/utils.cpp
		src/ui/fmpire_ui.cpp
		src/ui/widgets/arc.cpp
		src/ui/widgets/double_click.cpp
		src/ui/widgets/fmpire_border.cpp
		src/ui/widgets/fmpire_grid_container.cpp
		src/ui/widgets/fmpire_knob.cpp
		src/ui/widgets/fmpire_theme.cpp
		src/ui/widgets/fmpire_widget.cpp
)

target_include_directories(fmpire
	PUBLIC
	src
	src/common/
	src/dsp
	src/ui
	src/ui/widgets/
)
