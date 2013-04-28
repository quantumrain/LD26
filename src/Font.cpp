#include "Pch.h"
#include "Common.h"

void draw_char(vec2 pos, vec2 scale, int sprite, colour col)
{
	int sx = sprite & 7;
	int sy = sprite / 8;

	vec2 uv0(sx / 8.0f, sy / 8.0f);
	vec2 uv1(uv0 + vec2(1.0f / 8.0f));

	draw_font_rect(pos, pos + 8.0f * scale, uv0, uv1, col);
}

float measure_char(int c)
{
	switch(c)
	{
		case 'I': return 4.0f;
		case 'W': return 9.0f;
		case 'M': return 9.0f;
		case ':': return 4.0f;
		case '.': return 4.0f;
		case '/': return 6.0f;
		case ' ': return 4.0f;
		case '-': return 5.0f;
		case '!': return 4.0f;
	}

	return 8.0f;
}

float measure_string(const char* txt)
{
	float w = 0.0f;

	for(; *txt; txt++)
		w += measure_char(*txt);

	return w;
}

void draw_string(vec2 pos, vec2 scale, int flags, colour col, const char* txt, ...)
{
	char buf[512];

	va_list ap;

	va_start(ap, txt);
	_vsnprintf_s(buf, sizeof(buf), _TRUNCATE, txt, ap);
	va_end(ap);

	if (flags == TEXT_CENTRE)
		pos.x -= measure_string(txt) * 0.5f * scale.x;
	else if (flags == TEXT_RIGHT)
		pos.x -= measure_string(txt) * scale.x;

	const char* letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.:/-!@";

	for(const char* p = buf; *p != '\0'; p++)
	{
		int c = toupper(*p);
		int sprite = -1;

		if (const char* ofs = strchr(letters, c))
			sprite = (int)(intptr_t)(ofs - letters);

		if (sprite >= 0)
			draw_char(pos, scale, sprite, col); 

		pos.x += measure_char(c) * scale.x;
	}
}