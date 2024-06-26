/*
Copyright 2024 Sebastian Murschall
*/

#include "unit_osc.h"
#include "oscillator.h"
#include <climits>

enum {
	BITCRUSH = 0U,
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
	TYPE_OFF,
	NUM_TYPE_VALUES,
};

static struct UserParameters {
	float bitcrush = 0.f;
	float shiftshape = 0.f;
	uint32_t detune = 0.f;
	uint32_t lvl1 = 0;
	uint32_t lvl2 = 0;
	uint32_t type1{ 1 };
	uint32_t type2{ 1 };

	void reset() {
		bitcrush = 0.f;
		detune = 0.f;
		type1 = 1;
		type2 = 1;
		lvl1 = 6;
		lvl2 = 6;
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
  case TYPE_OFF:
    return OFF;
  default:
    return OFF;
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
  s_param.reset();
  osc_1.Reset();
  osc_2.Reset();
}

__unit_callback void unit_resume() {
}

__unit_callback void unit_suspend() {
}

__unit_callback void unit_render(const float* in, float* out, uint32_t frames) {
	const unit_runtime_osc_context_t* ctxt = static_cast<const unit_runtime_osc_context_t*>(runtime_desc.hooks.runtime_context);
	
	float bitcrush = unit_get_param_value(BITCRUSH);
	
	osc_1.SetNote(ctxt->pitch);
	osc_2.SetNote(ctxt->pitch);
	
	// const float * __restrict in_p = in;
	float* __restrict y = out;
	const float* y_e = y + frames;

	for (; y != y_e; )
	{
	  const float sig1 = osc_1.Render();
	  const float sig2 = osc_2.Render();
	  float combinedSignal;

	  /* No need to add signal if osc_2 is off*/
	  if(osc_2.GetType() == OFF){
	    combinedSignal = fastertanhf(sig1);
	  } else{
	    combinedSignal = fastertanhf((sig1/2) + (sig2/2));
	  }

	  if(bitcrush > 0) {
	    float scaling_f = osc_bitresf(s_param.bitcrush);
	    combinedSignal = combinedSignal*scaling_f;
	  }

	  *(y++) = combinedSignal;
	  osc_1.Tick();
	  osc_2.Tick();
	}
}

__unit_callback void unit_set_param_value(uint8_t id, int32_t value) {
	switch (id) {
	case BITCRUSH:
		// 0 .. 1023 -> 0.0 .. 1.0
		value = clipminmaxi32(0, value, 1023);
		s_param.bitcrush = param_10bit_to_f32(value);
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
	case BITCRUSH:
		// 0.0 .. 1.0 -> 0 .. 1023
		return param_f32_to_10bit(s_param.bitcrush);
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
	  "OFF"
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
