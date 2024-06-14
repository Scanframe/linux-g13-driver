#pragma once

class G13Action
{
	private:
		int pressed{};

	protected:
		virtual void key_down();
		virtual void key_up();

	public:
		G13Action();
		virtual ~G13Action();
		virtual int set(int state);
		[[nodiscard]] int isPressed() const;
};

