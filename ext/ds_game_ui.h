#pragma once
#include <diesel.h>

class SpriteBatchBuffer;

namespace dialog {

	struct CharactorInfo {
		int x;
		int y;
		int width;
		int height;
	};

	struct FontInfo {
		ds::vec4 texture_rects[255];
	};

	void init(SpriteBatchBuffer* buffer, const FontInfo& info);

	void begin();

	void Image(const ds::vec2& pos, const ds::vec4& rect, const ds::Color& color = ds::Color(255,255,255,255));

	bool Button(const ds::vec2& pos, const ds::vec4& rect);

	bool Button(const ds::vec2& pos, const ds::vec4& rect, const char* text);

	bool Button(const ds::vec2& pos, const ds::vec4& rect, const char* text, const ds::Color& color);

	bool isCursorInside(const ds::vec2& p, const ds::vec2& dim);

	void Text(const ds::vec2& pos, const char* text, bool centered = true, const ds::vec2& scale = ds::vec2(1.0f), const ds::Color& color = ds::Color(255,255,255,255));

	void FormattedText(const ds::vec2& pos, bool centered, const ds::vec2& scale, const ds::Color& color, const char* fmt, ...);

	void Input(const ds::vec2& pos, char* text, int maxLength);

	void end();

	ds::vec2 text_size(const char* txt, const ds::vec2& scale = ds::vec2(1.0f));

}

#ifdef DS_GAME_UI_IMPLEMENTATION

#include <SpriteBatchBuffer.h>

namespace dialog {

	struct DrawCall {
		ds::vec2 pos;
		ds::vec4 rect;
		ds::vec2 scale;
		ds::Color color;
	};

	const int MAX_DRAW_CALLS = 1024;

	struct GUIContext {
		SpriteBatchBuffer* buffer;
		DrawCall calls[MAX_DRAW_CALLS];
		int num_calls;
		bool clicked;
		bool buttonPressed;
		FontInfo fontInfo;
	};

	static GUIContext _guiCtx;

	static ds::vec4 get_rect(char c) {
		// convert to lower case to upper case
		if (c > 92) {
			c -= 32;
		}		
		return _guiCtx.fontInfo.texture_rects[c];

	}

	const float FONT_PADDING = 0.0f;

	ds::vec2 text_size(const char* txt, const ds::vec2& scale) {
		int l = strlen(txt);
		ds::vec2 p(0.0f);
		for (int i = 0; i < l; ++i) {
			ds::vec4 r = get_rect(txt[i]);
			p.x += r.z * scale.x + FONT_PADDING;
			if (r.w * scale.y > p.y) {
				p.y = r.w * scale.y;
			}
		}
		return p;
	}

	// -------------------------------------------------------
	// check if mouse cursor is inside box
	// -------------------------------------------------------
	bool isCursorInside(const ds::vec2& p, const ds::vec2& dim) {
		ds::vec2 mp = ds::getMousePosition();
		if (mp.x < (p.x - dim.x * 0.5f)) {
			return false;
		}
		if (mp.x >(p.x + dim.x * 0.5f)) {
			return false;
		}
		if (mp.y < (p.y - dim.y * 0.5f)) {
			return false;
		}
		if (mp.y >(p.y + dim.y * 0.5f)) {
			return false;
		}
		return true;
	}

	// -------------------------------------------------------
	// handle mouse interaction
	// -------------------------------------------------------
	static bool isClicked(const ds::vec2& pos, const ds::vec2& size) {
		if (_guiCtx.clicked) {
			ds::vec2 p = pos;
			if (_guiCtx.clicked && isCursorInside(p, size)) {
				return true;
			}
		}
		return false;
	}

	// ---------------------------------------------------------------
	// init
	// ---------------------------------------------------------------
	void init(SpriteBatchBuffer* buffer, const FontInfo& fontInfo) {
		_guiCtx.buffer = buffer;
		_guiCtx.clicked = false;
		_guiCtx.buttonPressed = false;
		_guiCtx.num_calls = 0;
		_guiCtx.fontInfo = fontInfo;
	}

	// ---------------------------------------------------------------
	// begin
	// ---------------------------------------------------------------
	void begin() {
		_guiCtx.num_calls = 0;
		if (_guiCtx.clicked) {
			_guiCtx.clicked = false;
		}
		if (ds::isMouseButtonPressed(0)) {
			_guiCtx.buttonPressed = true;
		}
		else {
			if (_guiCtx.buttonPressed) {
				_guiCtx.clicked = true;
			}
			_guiCtx.buttonPressed = false;
		}
	}

	// ---------------------------------------------------------------
	// button
	// ---------------------------------------------------------------
	bool Button(const ds::vec2& pos, const ds::vec4& rect) {
		if (_guiCtx.num_calls < MAX_DRAW_CALLS) {
			DrawCall& call = _guiCtx.calls[_guiCtx.num_calls++];
			call.pos = pos;
			call.rect = rect;
			call.scale = ds::vec2(1.0f);
			call.color = ds::Color(255, 255, 255, 255);
			ds::vec2 dim = ds::vec2(rect.z, rect.w);
			return isClicked(pos, dim);
		}
		return false;
	}

	bool Button(const ds::vec2& pos, const ds::vec4& rect, const char* text) {
		return Button(pos, rect, text, ds::Color(255, 255, 255, 255));
	}

	// ---------------------------------------------------------------
	// button with text
	// ---------------------------------------------------------------
	bool Button(const ds::vec2& pos, const ds::vec4& rect, const char* text, const ds::Color& buttonColor) {
		DrawCall& call = _guiCtx.calls[_guiCtx.num_calls++];
		call.pos = pos;
		call.rect = rect;
		call.scale = ds::vec2(1.0f);
		call.color = buttonColor;
		ds::vec2 dim = ds::vec2(rect.z, rect.w);
		int l = strlen(text);
		ds::vec2 p = pos;
		ds::vec2 size = text_size(text);
		p.x = pos.x - size.x * 0.5f;
		float lw = 0.0f;
		for (int i = 0; i < l; ++i) {
			ds::vec4 r = get_rect(text[i]);
			p.x += lw * 0.5f + r.z * 0.5f;
			DrawCall& text_call = _guiCtx.calls[_guiCtx.num_calls++];
			text_call.pos = p;
			text_call.rect = r;
			text_call.scale = ds::vec2(1.0f);
			text_call.color = ds::Color(255, 255, 255, 255);
			lw = r.z;
		}
		return isClicked(pos, dim);
	}


	// ---------------------------------------------------------------
	// image
	// ---------------------------------------------------------------
	void Image(const ds::vec2& pos, const ds::vec4& rect, const ds::Color& color) {
		if (_guiCtx.num_calls < MAX_DRAW_CALLS) {
			DrawCall& call = _guiCtx.calls[_guiCtx.num_calls++];
			call.pos = pos;
			call.rect = rect;
			call.scale = ds::vec2(1.0f);
			call.color = color;
		}
	}

	// ---------------------------------------------------------------
	// text
	// ---------------------------------------------------------------
	void Text(const ds::vec2& pos, const char* text, bool centered, const ds::vec2& scale, const ds::Color& color) {
		int l = strlen(text);
		ds::vec2 p = pos;
		ds::vec2 size = text_size(text, scale);
		if (centered) {
			p.x = (ds::getScreenWidth() - size.x) * 0.5f;
		}

		if (scale.x != 1.0f || scale.y != 1.0f) {
			ds::vec2 scaled = text_size(text, scale);
			ds::vec2 reg = text_size(text);
			float dx = (scaled.x - reg.x) * 0.5f;
			float dy = (scaled.y - reg.y) * 0.5f;
			p.x -= dx * 0.5f;
			p.y -= dy * 0.5f;
		}

		float lw = 0.0f;
		for (int i = 0; i < l; ++i) {
			ds::vec4 r = get_rect(text[i]);
			p.x += lw * 0.5f + r.z * 0.5f * scale.x;
			if (_guiCtx.num_calls < MAX_DRAW_CALLS) {
				DrawCall& call = _guiCtx.calls[_guiCtx.num_calls++];
				call.pos = p;
				call.rect = r;
				call.scale = scale;
				call.color = color;
			}
			lw = r.z * scale.x;
		}
	}

	// ---------------------------------------------------------------
	// formatted text
	// ---------------------------------------------------------------
	void FormattedText(const ds::vec2& pos, bool centered, const ds::vec2& scale, const ds::Color& color, const char* fmt, ...) {
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsprintf(buffer, fmt, args);
		ds::vec2 size = text_size(buffer);
		Text(pos, buffer, centered, scale, color);
		va_end(args);
	}

	// ---------------------------------------------------------------
	// input
	// ---------------------------------------------------------------
	void Input(const ds::vec2& pos, char* text, size_t maxLength) {
		size_t len = strlen(text);
		for (int i = 0; i < ds::getNumInputKeys(); ++i) {
			const ds::InputKey& key = ds::getInputKey(i);
			if (key.type == ds::IKT_SYSTEM) {
				if (key.value == ds::SpecialKeys::DSKEY_Backspace) {
					--len;
					text[len] = '\0';
				}
			}
			else {
				if ((key.value >= 32 && key.value < 128)) {
					if (len < maxLength) {
						text[len] = key.value;
						++len;
					}
				}
			}
			text[len] = '\0';
		}
		Text(pos, text, true);
	}

	// ---------------------------------------------------------------
	// end
	// ---------------------------------------------------------------
	void end() {
		for (int i = 0; i < _guiCtx.num_calls; ++i) {
			const DrawCall& call = _guiCtx.calls[i];
			_guiCtx.buffer->add(call.pos, call.rect, call.scale, 0.0f, call.color);
		}
	}
}
#endif