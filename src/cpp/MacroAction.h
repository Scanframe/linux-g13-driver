#pragma once

#include <linux/uinput.h>
#include <unistd.h>
#include <vector>

#include "G13Action.h"
#include "Output.h"

using namespace std;

class MacroAction : public G13Action
{
	public:
		class Event
		{
			public:
				Event() = default;
				virtual void execute() {};
		};

		class KeyDownEvent : Event
		{
			private:
				int keycode;

			public:
				explicit KeyDownEvent(int code) { this->keycode = code; }

				void execute() override
				{
					//cout<<"  kde."<<keycode<<"\n";
					send_event(EV_KEY, keycode, 1);
					send_event(0, 0, 0);
				}
		};

		class KeyUpEvent : Event
		{
			private:
				int keycode;

			public:
				explicit KeyUpEvent(int code) { this->keycode = code; }

				void execute() override
				{
					//cout<<"  kue."<<keycode<<"\n";
					send_event(EV_KEY, keycode, 0);
					send_event(0, 0, 0);
				}
		};

		class DelayEvent : Event
		{
			private:
				int delayInMillisecs;

			public:
				explicit DelayEvent(int delayInMillisecs)
				{
					this->delayInMillisecs = delayInMillisecs;
				}

				void execute() override
				{
					//cout<<"  de."<<delayInMillisecs<<"\n";
					usleep(1000 * delayInMillisecs);
				}
		};

		class MultiEventThread
		{
			public:
				int keepRepeating;
				vector<MacroAction::Event*> local_events;

				MultiEventThread() { keepRepeating = 0; }

				virtual void execute()
				{
					do
					{
						//std::cout << "MultiEventThread::execute() local_events.size() = " << local_events.size() << "\n";
						for (auto & local_event : local_events) {
							local_event->execute();
							usleep(100);
						}

					}
					while (keepRepeating);
				}
		};

	private:
		vector<MacroAction::Event*> events;
		int _repeats{};
		MultiEventThread* _thread{};
		pthread_attr_t _attr{};

	protected:
		MacroAction::Event* tokenToEvent(char* token);
		void key_down() override;
		void key_up() override;

	public:
		explicit MacroAction(char* tokens);
		~MacroAction() override;

		int getRepeats() const;
		void setRepeats(int repeats);
		vector<MacroAction::Event*> getEvents();
};

