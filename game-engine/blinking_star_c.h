#pragma once

class blinking_star_c
{
public:
	blinking_star_c(int x, int y, framebuffer_c* buffer, FIBITMAP* star_fib)
		: m_center_x(x)
		, m_center_y(y)
		, m_original_image_chunk(nullptr)
		, m_blink_started(false)
		, m_framebuffer(buffer)
		, m_star_fib(star_fib)
		, m_blink_start_time(0)
	{
		m_finished = true;

		m_star_pixel_data = (decltype(m_star_pixel_data))FreeImage_GetBits(m_star_fib);

		m_size = FreeImage_GetWidth(star_fib);

		m_left_x = m_center_x - (int)(m_size * 0.5f);
		m_top_y = m_center_y - (int)(m_size * 0.5f);

#if 0
		if (m_original_image_chunk == nullptr)
		{
			auto pitch = m_framebuffer->get_buffer_width();

			auto framebuf_data = m_framebuffer->get_data_ptr();

			m_original_image_chunk = new int32_t[m_size * m_size];

			for (int y = m_top_y; y < m_top_y + m_size; ++y)
			{
				for (int x = m_left_x; x < m_left_x + m_size; ++x)
				{
					auto buffer_pixel_index = y * pitch + x;

					auto dest_pixel_index = (y - m_top_y) * m_size + (x - m_left_x);

					m_original_image_chunk[dest_pixel_index] = framebuf_data[buffer_pixel_index];

					auto& star_pixel = m_star_pixel_data[dest_pixel_index];
					auto& original_pixel = m_original_image_chunk[dest_pixel_index];

					/*float star_a = (float)POP_A(star_pixel);
					auto star_r = POP_R(star_pixel);
					auto star_g = POP_G(star_pixel);
					auto star_b = POP_B(star_pixel);

					auto frame_a = POP_A(original_pixel);
					auto frame_r = POP_R(original_pixel);
					auto frame_g = POP_G(original_pixel);
					auto frame_b = POP_B(original_pixel);

					float a01 = (1.0f - star_a) * frame_a + star_a;
					float r01 = ((1.0f - star_a) * frame_a * frame_r + star_a * star_r) / a01;
					float g01 = ((1.0f - star_a) * frame_a * frame_g + star_a * star_g) / a01;
					float b01 = ((1.0f - star_a) * frame_a * frame_b + star_a * star_b) / a01;*/

					/*if (a01 > 255.0f) a01 = 255.0f;
					if (r01 > 255.0f) r01 = 255.0f;
					if (g01 > 255.0f) g01 = 255.0f;
					if (b01 > 255.0f) b01 = 255.0f;

					framebuf_pixel = RGBA((uint8_t)r01, (uint8_t)g01, (uint8_t)b01, (uint8_t)a01);*/
				}
			}
		}
#endif
	}

	~blinking_star_c()
	{
		if (m_original_image_chunk)
		{
			delete m_original_image_chunk;
			m_original_image_chunk = nullptr;
		}
	}

	void start_blink()
	{
		m_finished = false;

		m_blink_start_time = SDL_GetTicks();

		m_blink_duration = 2000;
	}

	void update()
	{
		if (m_finished)
		{
			return;
		}

#if 0
		float blink_factor = abs(sin(SDL_GetTicks() * 0.005f));
#else
		auto cur_ticks = SDL_GetTicks();

		auto cur_duration = cur_ticks - m_blink_start_time;
		if (cur_duration > m_blink_duration)
		{
			cur_duration = m_blink_duration;

			m_finished = true;
		}

		float blink_factor = sin(SR_DEG_TO_RAD(cur_duration * 180 / (float)m_blink_duration));

		/*OutputDebugStringA(std_string_printf("blink_factor: %f, cur_duration: %d, div: %f\n", 64,
			blink_factor, cur_duration,
			cur_duration * 180 / (float)m_blink_duration
		).c_str());*/
#endif

		auto pitch = m_framebuffer->get_buffer_width();

		auto framebuf_data = m_framebuffer->get_data_ptr();

		for (int y = m_top_y; y < m_top_y + m_size; ++y)
		{
			for (int x = m_left_x; x < m_left_x + m_size; ++x)
			{
				auto buffer_pixel_index = y * pitch + x;

				auto dest_pixel_index = (y - m_top_y) * m_size + (x - m_left_x);

				auto& star_pixel = m_star_pixel_data[dest_pixel_index];
				auto& framebuf_pixel = framebuf_data[buffer_pixel_index];

				// todo: optimize!!!!! very slow algorithm

				float star_a = (float)POP_A(star_pixel);
				auto star_r = POP_R(star_pixel);
				auto star_g = POP_G(star_pixel);
				auto star_b = POP_B(star_pixel);

				float frame_a = 255.0f;// (float)POP_A(framebuf_pixel);
				auto frame_r = POP_R(framebuf_pixel);
				auto frame_g = POP_G(framebuf_pixel);
				auto frame_b = POP_B(framebuf_pixel);

				star_a = star_a / 255.0f * blink_factor;
				frame_a /= 255.0f;

				float a01 = (1.0f - star_a) * frame_a + star_a;
				float r01 = ((1.0f - star_a) * frame_a * frame_r + star_a * star_r) / a01;
				float g01 = ((1.0f - star_a) * frame_a * frame_g + star_a * star_g) / a01;
				float b01 = ((1.0f - star_a) * frame_a * frame_b + star_a * star_b) / a01;

				if (a01 > 255.0f) a01 = 255.0f;
				if (r01 > 255.0f) r01 = 255.0f;
				if (g01 > 255.0f) g01 = 255.0f;
				if (b01 > 255.0f) b01 = 255.0f;

				framebuf_pixel = RGBA
				(
					(uint8_t)r01,
					(uint8_t)g01,
					(uint8_t)b01,
					(uint8_t)0
				);
			}
		}
	}

	bool blink_finished() const
	{
		return m_finished;
	}

private:
	int m_center_x, m_center_y, m_left_x, m_top_y, m_size;

	int32_t* m_original_image_chunk;

	bool m_blink_started;

	uint32_t m_blink_start_time, m_blink_duration;

	framebuffer_c* m_framebuffer;

	FIBITMAP* m_star_fib;
	int32_t* m_star_pixel_data;

	bool m_finished;
};
