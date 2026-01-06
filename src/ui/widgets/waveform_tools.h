#ifndef WAVEFORM_TOOLS_H_INCLUDED
#define WAVEFORM_TOOLS_H_INCLUDED

#include "ref_counted.h"
#include "waveform_part.h"

namespace fmpire
{

class Waveform;

enum class WaveformToolType
{
	FREE,
	LINE,
	HALF_SINE,
	SINE_SLOPE
};

struct WaveformToolStepInfo
{
	float x;
	float y;
	float snapped_x;
	float snapped_y;
};

class WaveformTool : public RefCounted
{
public:
	WaveformTool(Waveform* wf);
	virtual ~WaveformTool();

	virtual void start(const WaveformToolStepInfo& info) = 0;

	virtual void step(const WaveformToolStepInfo& info) = 0;

	virtual void end() = 0;

protected:
	Ref<Waveform> waveform;
};

class FreeDrawTool : public WaveformTool
{
public:
	FreeDrawTool(Waveform* wf, SamplesWaveformPart* existing_part = nullptr);
	virtual ~FreeDrawTool();

	virtual void start(const WaveformToolStepInfo& info) override;

	virtual void step(const WaveformToolStepInfo& info) override;

	virtual void end() override;

private:
	Ref<SamplesWaveformPart> part;
	WaveformToolStepInfo last_info;
};

class FunctionDrawTool : public WaveformTool
{
public:
	FunctionDrawTool(Waveform* wf);
	virtual ~FunctionDrawTool();

	virtual void start(const WaveformToolStepInfo& info) override;

	virtual void step(const WaveformToolStepInfo& info) override;

	virtual void end() override;

protected:
	Ref<FunctionWaveformPart> part;
	float start_x;
	float start_y;

	virtual std::string create_function(float end_x, float end_y) = 0;
};

class LineDrawTool : public FunctionDrawTool
{
public:
	LineDrawTool(Waveform* wf) :
		FunctionDrawTool(wf)
	{
	}

	virtual ~LineDrawTool() {}

protected:
	virtual std::string create_function(float end_x, float end_y) override;
};

class HalfSineTool : public FunctionDrawTool
{
public:
	HalfSineTool(Waveform* wf) :
		FunctionDrawTool(wf)
	{
	}

	virtual ~HalfSineTool() {}

protected:
	virtual std::string create_function(float end_x, float end_y) override;
};

class SineSlopeTool : public FunctionDrawTool
{
public:
	SineSlopeTool(Waveform* wf) :
		FunctionDrawTool(wf)
	{
	}

	virtual ~SineSlopeTool() {}

protected:
	virtual std::string create_function(float end_x, float end_y) override;
};

} // namespace fmpire

#endif // WAVEFORM_TOOLS_H_INCLUDED
