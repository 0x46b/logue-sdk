#ifndef OSCILLATOR_H
#define OSCILLATOR_H
#include "unit_osc.h"
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
  float GetSignal();
  float AdjustSignalLevel(float signal);
 public:
  void Init(const unit_runtime_desc_t* desc);
  float Render();
  void Tick();
  void Reset();

  void SetType(OscillatorType type);
  OscillatorType GetType();

  void SetLevel(uint32_t level);
  uint32_t GetLevel();

  void SetNote(uint16_t pitch);
  void Getnote(uint16_t pitch);

  void SetDetune(uint32_t detune);
  uint32_t GetDetune();
};

#endif // OSCILLATOR_H
