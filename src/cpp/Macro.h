#pragma once

class Macro
{
	private:
		int _id{};
		char* _name{};
		char* _sequence{};

	public:
		Macro() = default;
		[[nodiscard]] int getId() const;
		[[nodiscard]] const char* getName() const;
		[[nodiscard]] char* getSequence() const;
		void setId(int id);
		void setName(char* name);
		void setSequence(char* sequence);
};
