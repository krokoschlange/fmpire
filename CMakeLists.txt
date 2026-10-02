cmake_minimum_required(VERSION 3.20)

project(fmpire)

set(CMAKE_CXX_STANDARD 23)

# apply local patches to the DPF submodule (skipped if already applied)
find_package(Git REQUIRED)
file(GLOB DPF_PATCHES ${CMAKE_CURRENT_SOURCE_DIR}/patches/dpf-*.patch)
foreach(patch ${DPF_PATCHES})
	execute_process(
		COMMAND ${GIT_EXECUTABLE} apply --reverse --check ${patch}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/DPF
		RESULT_VARIABLE patch_applied
		OUTPUT_QUIET ERROR_QUIET
	)
	if(NOT patch_applied EQUAL 0)
		message(STATUS "Applying ${patch}")
		execute_process(
			COMMAND ${GIT_EXECUTABLE} apply ${patch}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/DPF
			RESULT_VARIABLE patch_result
		)
		if(NOT patch_result EQUAL 0)
			message(FATAL_ERROR "Failed to apply ${patch}")
		endif()
	endif()
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${patch})
endforeach()

add_subdirectory(DPF)

dpf_add_plugin(fmpire
	TARGETS jack clap lv2 vst3
	UI_TYPE cairo
	USE_FILE_BROWSER
	FILES_DSP
		src/dsp/fmpire.cpp
		src/dsp/modulator.cpp
		src/dsp/oscillator.cpp
		src/dsp/voice.cpp
		src/common/FftRealPair.cpp
		src/common/curve.cpp
		src/common/mod_types.cpp
		src/common/utils.cpp
		src/common/waveform.cpp
		src/common/waveform_part.cpp
		src/common/wavetable.cpp
		src/common/wavetable_creator.cpp
	FILES_UI
		src/common/FftRealPair.cpp
		src/common/curve.cpp
		src/common/mod_types.cpp
		src/common/utils.cpp
		src/common/waveform.cpp
		src/common/waveform_part.cpp
		src/common/wavetable.cpp
		src/common/wavetable_creator.cpp
		src/ui/double_click.cpp
		src/ui/fmpire_ui.cpp
		src/ui/modulation_model.cpp
		src/ui/state_manager.cpp
		src/ui/wavetable_history.cpp
		src/ui/widgets/arc.cpp
		src/ui/widgets/aspect_ratio_container.cpp
		src/ui/widgets/border.cpp
		src/ui/widgets/button.cpp
		src/ui/widgets/curve_editor.cpp
		src/ui/widgets/draw_operations.cpp
		src/ui/widgets/fmpire_widget.cpp
		src/ui/widgets/fmpire_window.cpp
		src/ui/widgets/grid_container.cpp
		src/ui/widgets/harmonic_editor.cpp
		src/ui/widgets/image_button.cpp
		src/ui/widgets/int_editor.cpp
		src/ui/widgets/keyboard_bar.cpp
		src/ui/widgets/knob.cpp
		src/ui/widgets/label.cpp
		src/ui/widgets/mod_source_list.cpp
		src/ui/widgets/modulation_matrix.cpp
		src/ui/widgets/modulator_editor.cpp
		src/ui/widgets/modulator_settings_panel.cpp
		src/ui/widgets/oscillator_bar.cpp
		src/ui/widgets/oscillator_page.cpp
		src/ui/widgets/oscillator_settings.cpp
		src/ui/widgets/piano_keyboard.cpp
		src/ui/widgets/relative_container.cpp
		src/ui/widgets/scroll_container.cpp
		src/ui/widgets/selector.cpp
		src/ui/widgets/source_list_page.cpp
		src/ui/widgets/spectrum_view.cpp
		src/ui/widgets/text_entry.cpp
		src/ui/widgets/theme.cpp
		src/ui/widgets/tooltip.cpp
		src/ui/widgets/waveform_bulk_editor.cpp
		src/ui/widgets/waveform_editor.cpp
		src/ui/widgets/waveform_part_editor.cpp
		src/ui/widgets/waveform_selector.cpp
		src/ui/widgets/waveform_tools.cpp
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

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED IMPORTED_TARGET sndfile)
target_link_libraries(fmpire-ui PRIVATE PkgConfig::SNDFILE)
