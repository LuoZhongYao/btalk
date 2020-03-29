add_library(webrtc)
target_sources(webrtc PRIVATE
	webrtc/modules/audio_processing/aecm/aecm_core.c
	webrtc/modules/audio_processing/aecm/aecm_core_c.c
	webrtc/modules/audio_processing/aecm/echo_control_mobile.c

	webrtc/modules/audio_processing/ns/noise_suppression.c
	webrtc/modules/audio_processing/ns/noise_suppression_x.c
	webrtc/modules/audio_processing/ns/ns_core.c
	webrtc/modules/audio_processing/ns/nsx_core.c
	webrtc/modules/audio_processing/ns/nsx_core_c.c


	webrtc/modules/audio_processing/agc/legacy/analog_agc.c
	webrtc/modules/audio_processing/agc/legacy/digital_agc.c

	webrtc/modules/audio_processing/utility/delay_estimator.c
	webrtc/modules/audio_processing/utility/delay_estimator_wrapper.c

	webrtc/common_audio/signal_processing/auto_correlation.c
	webrtc/common_audio/signal_processing/auto_corr_to_refl_coef.c
	webrtc/common_audio/signal_processing/complex_bit_reverse.c
	webrtc/common_audio/signal_processing/complex_fft.c
	webrtc/common_audio/signal_processing/copy_set_operations.c
	webrtc/common_audio/signal_processing/cross_correlation.c
	webrtc/common_audio/signal_processing/division_operations.c
	webrtc/common_audio/signal_processing/dot_product_with_scale.c
	webrtc/common_audio/signal_processing/downsample_fast.c
	webrtc/common_audio/signal_processing/energy.c
	webrtc/common_audio/signal_processing/filter_ar.c
	webrtc/common_audio/signal_processing/filter_ar_fast_q12.c
	webrtc/common_audio/signal_processing/filter_ma_fast_q12.c
	webrtc/common_audio/signal_processing/get_hanning_window.c
	webrtc/common_audio/signal_processing/get_scaling_square.c
	webrtc/common_audio/signal_processing/ilbc_specific_functions.c
	webrtc/common_audio/signal_processing/levinson_durbin.c
	webrtc/common_audio/signal_processing/lpc_to_refl_coef.c
	webrtc/common_audio/signal_processing/min_max_operations.c
	webrtc/common_audio/signal_processing/randomization_functions.c
	webrtc/common_audio/signal_processing/real_fft.c
	webrtc/common_audio/signal_processing/refl_coef_to_lpc.c
	webrtc/common_audio/signal_processing/resample_48khz.c
	webrtc/common_audio/signal_processing/resample_by_2.c
	webrtc/common_audio/signal_processing/resample_by_2_internal.c
	webrtc/common_audio/signal_processing/resample.c
	webrtc/common_audio/signal_processing/resample_fractional.c
	webrtc/common_audio/signal_processing/spl_init.c
	webrtc/common_audio/signal_processing/splitting_filter.c
	webrtc/common_audio/signal_processing/spl_sqrt.c
	webrtc/common_audio/signal_processing/spl_sqrt_floor.c
	webrtc/common_audio/signal_processing/sqrt_of_one_minus_x_squared.c
	webrtc/common_audio/signal_processing/vector_scaling_operations.c
	webrtc/common_audio/ring_buffer.c
	)
target_include_directories(webrtc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(webrtc PRIVATE sys)

add_subdirectory(sbc-1.4)
