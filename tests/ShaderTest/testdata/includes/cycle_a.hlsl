// Two-file pair used by DependencyScannerTest to exercise the recursive scan
// + cycle detection. cycle_a.hlsl #includes cycle_b.hlsl, which in turn
// #includes cycle_a.hlsl back.

#include "cycle_b.hlsl"

float identity_a(float v) { return v; }
