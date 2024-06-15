/*
Copyright 2024 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "unit_osc.h"
#include <climits>

enum {
	SHAPE = 0U,
	ALT,
	TYPE,
	NUM_PARAMS
};

enum TYPE_VALUES {
	TYPE_SINUS = 0,
	TYPE_SAW,
	TYPE_SQR,
	NUM_TYPE_VALUES,
};

static struct {
	float shape = 0.f;
	float shiftshape = 0.f;
	uint32_t type{ 1 };

	void reset() {
		shape = 0.f;
		shiftshape = 0.f;
		type = 1;
	}
} s_param;

static struct {
	float phi0 = 0.f;
	float w00 = 0.f;
} s_state;

static unit_runtime_desc_t runtime_desc;

// ---- Callbacks exposed to runtime ----------------------------------------------

__unit_callback int8_t unit_init(const unit_runtime_desc_t* desc) {
	if (!desc)
		return k_unit_err_undef;

	if (desc->target != unit_header.target)
		return k_unit_err_target;

	if (!UNIT_API_IS_COMPAT(desc->api))
		return k_unit_err_api_version;

	if (desc->samplerate != 48000)
		return k_unit_err_samplerate;

	if (desc->input_channels != 2 || desc->output_channels != 1)
		return k_unit_err_geometry;

	runtime_desc = *desc;

	s_param.reset();

	return k_unit_err_none;
}

__unit_callback void unit_teardown() {
}

__unit_callback void unit_reset() {
	s_state.phi0 = 0.f;
	s_state.w00 = 0.f;
}

__unit_callback void unit_resume() {
}

__unit_callback void unit_suspend() {
}

static float GetSignal(float phi) {
	switch (s_param.type)
	{
	case TYPE_SINUS:
		return osc_sinf(phi);
	case TYPE_SAW:
		return osc_sawf(phi);
	case TYPE_SQR:
		return osc_sqrf(phi);
	}
}

__unit_callback void unit_render(const float* in, float* out, uint32_t frames) {
	const unit_runtime_osc_context_t* ctxt = static_cast<const unit_runtime_osc_context_t*>(runtime_desc.hooks.runtime_context);

	s_state.w00 = osc_w0f_for_note((ctxt->pitch) >> 8, ctxt->pitch & 0xFF);

	// Temporaries.
	float phi0 = s_state.phi0;
	const float w00 = s_state.w00;

	// const float * __restrict in_p = in;
	float* __restrict y = out;
	const float* y_e = y + frames;
	float shape = unit_get_param_value(SHAPE);

	for (; y != y_e; )
	{
		float moddedPhi = phi0;
		if (shape > 0)
		{
			moddedPhi = phi0 * (shape / 2);
		}

		const float sig = GetSignal(moddedPhi);
		*(y++) = sig;

		phi0 += w00;
		phi0 -= (uint32_t)phi0;
	}

	s_state.phi0 = phi0;
}

__unit_callback void unit_set_param_value(uint8_t id, int32_t value) {
	switch (id) {
	case SHAPE:
		// 0 .. 1023 -> 0.0 .. 1.0
		value = clipminmaxi32(0, value, 1023);
		s_param.shape = param_10bit_to_f32(value);
		break;
	case ALT:
		// 0 .. 1023 -> 0.0 .. 1.0
		value = clipminmaxi32(0, value, 1023);
		s_param.shiftshape = param_10bit_to_f32(value);
		break;
	case TYPE:
		value = clipminmaxi32(TYPE_SINUS, value, NUM_TYPE_VALUES - 1);
		s_param.type = value;
	default:
		break;
	}
}

__unit_callback int32_t unit_get_param_value(uint8_t id) {
	switch (id) {
	case SHAPE:
		// 0.0 .. 1.0 -> 0 .. 1023
		return param_f32_to_10bit(s_param.shape);
		break;
	case ALT:
		// 0.0 .. 1.0 -> 0 .. 1023
		return param_f32_to_10bit(s_param.shiftshape);
		break;
	case TYPE:
		return s_param.type;
	default:
		break;
	}

	return INT_MIN;
}

__unit_callback const char* unit_get_param_str_value(uint8_t id, int32_t value) {
	// Note: String memory must be accessible even after function returned.
	//       It can be assumed that caller will have copied or used the string
	//       before the next call to getParameterStrValue

	static const char* type_strings[NUM_TYPE_VALUES] = {
	  "SIN",
	  "SAW",
	  "SQR",
	};

	switch (id) {
	case TYPE:
		if (value >= TYPE_SINUS && value < NUM_TYPE_VALUES)
			return type_strings[value];
		break;
	default:
		break;
	}

	return nullptr;
}

__unit_callback void unit_set_tempo(uint32_t tempo) {
}

__unit_callback void unit_tempo_4ppqn_tick(uint32_t counter) {
}

__unit_callback void unit_note_on(uint8_t note, uint8_t velo) {
}

__unit_callback void unit_note_off(uint8_t note) {
}

__unit_callback void unit_all_note_off() {
}

__unit_callback void unit_pitch_bend(uint16_t bend) {
}

__unit_callback void unit_channel_pressure(uint8_t press) {
}

__unit_callback void unit_aftertouch(uint8_t note, uint8_t press) {
}
