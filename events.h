struct events
	{
	events *next;
	CITEVENTS type;
	char line[1];
	};

struct intEvents
	{
	intEvents *next;
	CITEVENTS type;
	void (*func)(CITEVENTS);
	};

void doEvent(CITEVENTS eventNum, void (*InitScript)(ScriptInfoS *si) = NULL);
