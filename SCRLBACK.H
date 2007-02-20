// --------------------------------------------------------------------------
// Citadel: ScrlBack.H
//
// The interface to the scroll-back buffer.


void DestroyScrollBackBuffer(void);
Bool CreateScrollBackBuffer();
void SaveToScrollBackBuffer(int Lines);
void DisplayScrollBackBuffer(void);
