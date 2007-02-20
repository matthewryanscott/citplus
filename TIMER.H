class Timer
	{
	unsigned long StartValue;

public:
	Timer(void);
	void Reset(void);
	unsigned long Read(void);
	};

void InitializeTimer(void);
void DeinitializeTimer(void);

#define TIMER_HERTZ	1193180L
