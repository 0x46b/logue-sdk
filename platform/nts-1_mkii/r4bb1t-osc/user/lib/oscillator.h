#ifndef OSCILLATOR_H
#define OSCILLATOR_H
#include <climits>

enum OscillatorType{
  SINUS,
  SAW,
  SQR
};

class Oscillator {
 private:
  float phi;
  float w0;
  uint32_t detune;
  uint32_t level;
  static unit_runtime_desc_t runtime_desc;
  OscillatorType type;
  float GetSignal() {
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
  
  float AdjustSignalLevel(float signal){
    return signal * this->level / 6;
  }

public:
  void Init(const unit_runtime_desc_t* desc) {
    this->Reset();  
  }
  
  float Render() {
    float phi0 = this->phi / 2;
    const float w00 = this->w0;
    const float signal = this->GetSignal();
    return this->AdjustSignalLevel(signal);
  }
  
  void Tick() {
    this->phi += this->w0;
    this->phi -= (uint32_t)this->phi;
  }
  
  void Reset(){
    this->phi = 0;
    this->w0 = 0;
  }

  void SetType(OscillatorType type){
    this->type = type;
  }
  
  OscillatorType GetType(){
    return this->type;
  }

  void SetLevel(uint32_t level) {
    this->level = level;
  }
  
  uint32_t GetLevel() {
    return this->level;
  }

  void SetNote(uint16_t pitch){
    this->w0 = osc_w0f_for_note(pitch >> 8, pitch & 0xFF);
  }

   uint32_t GetNote(uint16_t pitch) {
    return this->w0;
  }

  void SetDetune(uint32_t detune){
    this->detune = detune;
  }
  
  uint32_t GetDetune(){
    return this->detune;
  }
};

#endif // OSCILLATOR_H
