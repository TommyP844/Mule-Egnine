#pragma once

#include <bgfx/bgfx.h>
#include <spdlog/spdlog.h>

class BGFXDebugCallback : public bgfx::CallbackI
{
public:
	BGFXDebugCallback()
	{
	}

	virtual ~BGFXDebugCallback() = default;

	// Override traceVargs for logging
	void traceVargs(const char* _filePath, uint16_t _line, const char* _format, va_list _argList) override
	{
		char buffer[1024];

		SPDLOG_INFO("[{}:{}] {}", _filePath, _line, buffer); // Log the message
	}

	// Override other methods as needed
	void fatal(const char* _filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char* _str) override
	{
		SPDLOG_CRITICAL("[FATAL] [{}:{}] {} (Code: {})", _filePath, _line, _str, static_cast<int>(_code));
	}

	uint32_t cacheReadSize(uint64_t /*_id*/) override { return 0; }
	bool cacheRead(uint64_t /*_id*/, void* /*_data*/, uint32_t /*_size*/) override { return false; }
	void cacheWrite(uint64_t /*_id*/, const void* /*_data*/, uint32_t /*_size*/) override {}

	void screenShot(const char* _filePath, uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_pitch*/, const void* /*_data*/, uint32_t /*_size*/, bool /*_yflip*/) override
	{
		SPDLOG_INFO("Screenshot saved to {}", _filePath);
	}

	void captureBegin(uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_pitch*/, bgfx::TextureFormat::Enum /*_format*/, bool /*_yflip*/) override {}
	void captureEnd() override {}
	void captureFrame(const void*, uint32_t) override {}

	virtual void profilerBegin(const char* name, uint32_t abgr, const char* path, uint16_t line) override
	{
		//SPDLOG_INFO("BGFX Profiler Begin\n\t [{}] [{}] [{}:{}]", name, abgr, path, line);
	}

	virtual void profilerBeginLiteral(const char* name, uint32_t abgr, const char* path, uint16_t line) override
	{
		//SPDLOG_INFO("BGFX Profiler Begin [{}] [{}] [{}:{}]", name, abgr, path, line);
	}

	virtual void profilerEnd() override
	{
		//SPDLOG_INFO("BGFX Profiler End");
	}

private:
};