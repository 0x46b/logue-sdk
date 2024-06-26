/*
Copyright 2024 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "unit_osc.h"
#include "oscillator.h"
#include <climits>

enum {
	SHAPE = 0U,
	DETUNE,
	TYP1,
	TYP2,
	LVL1,
	LVL2,
	NUM_PARAMS
};

enum TYPE_VALUES {
	TYPE_SINUS = 0,
	TYPE_SAW,
	TYPE_SQR,
	NUM_TYPE_VALUES,
};

static struct UserParameters {
	float shape = 0.f;
	float shiftshape = 0.f;
	uint32_t detune = 0.f;
	uint32_t lvl1 = 0;
	uint32_t lvl2 = 0;
	uint32_t type1{ 1 };
	uint32_t type2{ 1 };

	void reset() {
		shape = 0.f;
		detune = 0.f;
		type1 = 1;
		type2 = 1;
	}
} s_param;

static unit_runtime_desc_t runtime_desc;
static Oscillator osc_1;
static Oscillator osc_2;

static OscillatorType OscTypeFromTypeValue(uint32_t value) {
  switch(value){
  case TYPE_SINUS:
    return SINUS;
  case TYPE_SAW:
    return SAW;
  case TYPE_SQR:
    return SQR;
  default:
    return SINUS;
  }
}

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
	
	osc_1.Reset();
	osc_2.Reset();

	return k_unit_err_none;
}

__unit_callback void unit_teardown() {
}

__unit_callback void unit_reset() {
	osc_1.Reset();
	osc_2.Reset();
}

__unit_callback void unit_resume() {
}

__unit_callback void unit_suspend() {
}

__unit_callback void unit_render(const float* in, float* out, uint32_t frames) {
	const unit_runtime_osc_context_t* ctxt = static_cast<const unit_runtime_osc_context_t*>(runtime_desc.hooks.runtime_context);
	
	float shape = unit_get_param_value(SHAPE);
	uint32_t detune = unit_get_param_value(DETUNE);
	uint32_t type1 = unit_get_param_value(TYP1);
	uint32_t type2 = unit_get_param_value(TYP2);
	uint32_t lvl1 = unit_get_param_value(LVL1);
	uint32_t lvl2 = unit_get_param_value(LVL2);

	osc_1.SetNote(ctxt->pitch);
	
	// // Temporaries.
	
	// float phi1 = s_state_osc2.phi;
	// const float w01 = s_state_osc2.w0;
	

	// const float * __restrict in_p = in;
	float* __restrict y = out;
	const float* y_e = y + frames;

	for (; y != y_e; )
	{
	  osc_1.Render();
	  
	  // float moddedPhi0 = phi0/2;
	  
	  // if (shape > 0)
	  //   {
	  //     moddedPhi0 = phi0 * (shape / 2);
	  //     moddedPhi1 = phi1 * (shape / 2);
	  //   }
	  
	  const float sig1 = osc_1.Render();
	  const float sig2 = osc_2.Render();
	  
	  *(y++) = sig1 + sig2;
	  
	  osc_1.Tick();
	  osc_2.Tick();
	}
}

__unit_callback void unit_set_param_value(uint8_t id, int32_t value) {
	switch (id) {
	case SHAPE:
		// 0 .. 1023 -> 0.0 .. 1.0
		value = clipminmaxi32(0, value, 1023);
		s_param.shape = param_10bit_to_f32(value);
		break;
	case DETUNE:
		// 0 .. 1023 -> 0.0 .. 1.0
		value = clipminmaxi32(0, value, 15);
		osc_2.SetDetune(value);
		break;
	case TYP1:
		value = clipminmaxi32(TYPE_SINUS, value, NUM_TYPE_VALUES - 1);
		osc_1.SetType(OscTypeFromTypeValue(value));
	case TYP2:
		value = clipminmaxi32(TYPE_SINUS, value, NUM_TYPE_VALUES - 1);
		osc_2.SetType(OscTypeFromTypeValue(value));
	case LVL1:
		value = clipminmaxi32(0, value, 6);
		osc_1.SetLevel(value);
	case LVL2:
		value = clipminmaxi32(0, value, 6);
		osc_2.SetLevel(value);
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
	case DETUNE:
		// 0.0 .. 1.0 -> 0 .. 1023
	  return param_f32_to_10bit(osc_2.GetDetune());
		break;
	case LVL1:
	  return osc_1.GetLevel();
	case LVL2:
	  return osc_2.GetLevel();
	case TYP1:
	  return osc_1.GetType();
	case TYP2:
	  return osc_2.GetType();
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
	case TYP1:
	case TYP2:
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
