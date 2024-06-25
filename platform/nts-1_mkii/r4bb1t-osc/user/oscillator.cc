#include "oscillator.h"

void Oscillator::Init(const unit_runtime_desc_t *desc) {
  this->Reset();  
}

void Oscillator::Reset() {
  this->phi = 0;
  this->w0 = 0;
}

void Oscillator::SetNote(uint16_t pitch) {
  this->w0 = osc_w0f_for_note(pitch >> 8, pitch & 0xFF);
}

void Oscillator::SetType(OscillatorType type) {
  this->type = type;
}

OscillatorType  Oscillator::GetType() {
  return this->type;
}

float Oscillator::GetSignal() {
  switch (this->type)
    {
    case SINUS:
      return osc_sinf(this->phi);
    case SAW:
      return osc_sawf(this->phi);
    case SQR:
      return osc_sqrf(this->phi);
    default:
      return 0.f;
    }
}

float Oscillator::Render() {
  float phi0 = this->phi / 2;
  const float w00 = this->w0;
  const float signal = this->GetSignal();
  return this->AdjustSignalLevel(signal);
}

void Oscillator::Tick() {
  this->phi += this->w0;
  this->phi -= (uint32_t)this->phi;
}

float Oscillator::AdjustSignalLevel(float signal){
  return signal * this->level / 6;
}

void Oscillator::SetDetune(uint32_t detune) { this->detune = detune; }

uint32_t Oscillator::GetDetune() { return this->detune; }

void Oscillator::SetLevel(uint32_t level) { this->level = level; }

uint32_t Oscillator::GetLevel() { return this->level; }
