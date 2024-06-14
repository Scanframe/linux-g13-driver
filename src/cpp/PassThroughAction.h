#pragma once

#include "G13Action.h"

class PassThroughAction : public G13Action
{
	private:
		int _keycode;

	protected:
		void key_down() override;
		void key_up() override;

	public:
		explicit PassThroughAction(int code);
		~PassThroughAction() override;

		[[nodiscard]] int getKeyCode() const;
		void setKeyCode(int code);
};
